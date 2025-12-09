///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============  updated 
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>
#include <iostream>

// declare the global variables
namespace
{
    const char* g_ModelName        = "model";
    const char* g_ColorValueName   = "objectColor";
    const char* g_TextureValueName = "objectTexture";
    const char* g_UseTextureName   = "bUseTexture";
    const char* g_UseLightingName  = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
    : m_pShaderManager(pShaderManager),
      m_basicMeshes(new ShapeMeshes())
{
    // start with empty containers; textures & materials will be filled later
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
    // destroy the created OpenGL textures
    DestroyGLTextures();

    delete m_basicMeshes;
    m_basicMeshes = nullptr;

    m_pShaderManager = nullptr;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, const std::string& tag)
{
    int width = 0;
    int height = 0;
    int colorChannels = 0;
    GLuint textureID = 0;

    // indicate to always flip images vertically when loaded
    stbi_set_flip_vertically_on_load(true);

    // try to parse the image data from the specified image file
    unsigned char* image = stbi_load(
        filename,
        &width,
        &height,
        &colorChannels,
        0);

    if (!image)
    {
        std::cout << "Could not load image: " << filename << std::endl;
        return false;
    }

    std::cout << "Successfully loaded image: " << filename
              << ", width: " << width
              << ", height: " << height
              << ", channels: " << colorChannels << std::endl;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // if the loaded image is in RGB or RGBA format
    if (colorChannels == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, image);
    }
    else if (colorChannels == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, image);
    }
    else
    {
        std::cout << "Not implemented to handle image with "
                  << colorChannels << " channels" << std::endl;
        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0);
        return false;
    }

    // generate the texture mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // free the image data from local memory
    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

    // Enhancement: store texture info in dynamic container + map
    TEXTURE_INFO texInfo;
    texInfo.ID  = textureID;
    texInfo.tag = tag;

    int slotIndex = static_cast<int>(m_textures.size());
    if (slotIndex >= 16)
    {
        std::cout << "WARNING: Maximum texture slots (16) reached. Ignoring texture: "
                  << tag << std::endl;
        glDeleteTextures(1, &textureID);
        return false;
    }

    m_textures.push_back(texInfo);
    m_textureSlotLookup[tag] = slotIndex;

    return true;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots. There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
    for (size_t i = 0; i < m_textures.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
        glBindTexture(GL_TEXTURE_2D, m_textures[i].ID);
    }
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
    for (const auto& tex : m_textures)
    {
        GLuint id = tex.ID;
        if (id != 0)
        {
            glDeleteTextures(1, &id);
        }
    }
    m_textures.clear();
    m_textureSlotLookup.clear();
}

/***********************************************************
 *  FindTextureID()
 *
 *  Get an ID for the previously loaded texture bitmap
 *  associated with the passed-in tag.
 ***********************************************************/
int SceneManager::FindTextureID(const std::string& tag)
{
    auto it = m_textureSlotLookup.find(tag);
    if (it == m_textureSlotLookup.end())
    {
        return -1;
    }
    int slot = it->second;
    if (slot < 0 || slot >= static_cast<int>(m_textures.size()))
    {
        return -1;
    }
    return m_textures[slot].ID;
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  Get a slot index for the previously loaded texture bitmap
 *  associated with the passed-in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(const std::string& tag)
{
    auto it = m_textureSlotLookup.find(tag);
    if (it == m_textureSlotLookup.end())
    {
        return -1;
    }
    return it->second;
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
    glm::vec3 scaleXYZ,
    float XrotationDegrees,
    float YrotationDegrees,
    float ZrotationDegrees,
    glm::vec3 positionXYZ)
{
    glm::mat4 modelView(1.0f);
    glm::mat4 scale      = glm::scale(scaleXYZ);
    glm::mat4 rotationX  = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationY  = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationZ  = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 translation = glm::translate(positionXYZ);

    modelView = translation * rotationX * rotationY * rotationZ * scale;

    if (m_pShaderManager != nullptr)
    {
        m_pShaderManager->setMat4Value(g_ModelName, modelView);
    }
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
    float redColorValue,
    float greenColorValue,
    float blueColorValue,
    float alphaValue)
{
    glm::vec4 currentColor(redColorValue, greenColorValue, blueColorValue, alphaValue);

    if (m_pShaderManager != nullptr)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, false);
        m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
    }
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(const std::string& textureTag)
{
    if (m_pShaderManager != nullptr)
    {
        int textureSlot = FindTextureSlot(textureTag);
        if (textureSlot >= 0)
        {
            m_pShaderManager->setIntValue(g_UseTextureName, true);
            m_pShaderManager->setSampler2DValue(g_TextureValueName, textureSlot);
        }
        else
        {
            // fallback to color-only if texture is missing
            m_pShaderManager->setIntValue(g_UseTextureName, false);
        }
    }
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
    if (m_pShaderManager != nullptr)
    {
        m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
    }
}

/***********************************************************
 *  FindMaterial()
 *
 *  Look up material by tag using map; fall back to linear
 *  search if needed (during initial population).
 ***********************************************************/
bool SceneManager::FindMaterial(const std::string& tag, OBJECT_MATERIAL& material)
{
    // Fast path: use map lookup
    auto it = m_materialLookup.find(tag);
    if (it != m_materialLookup.end())
    {
        material = it->second;
        return true;
    }

    // Fallback: linear search (in case map not populated yet)
    for (const auto& mat : m_objectMaterials)
    {
        if (mat.tag == tag)
        {
            material = mat;
            return true;
        }
    }

    return false;
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(const std::string& materialTag)
{
    if (m_objectMaterials.empty() || m_pShaderManager == nullptr)
    {
        return;
    }

    OBJECT_MATERIAL material;
    if (FindMaterial(materialTag, material))
    {
        m_pShaderManager->setVec3Value("material.ambientColor",  material.ambientColor);
        m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
        m_pShaderManager->setVec3Value("material.diffuseColor",  material.diffuseColor);
        m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
        m_pShaderManager->setFloatValue("material.shininess",    material.shininess);
    }
}

/***********************************************************
 * DefineObjectMaterials()
 *
 * Configure the various material settings for all objects
 * in the 3D scene.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
    m_objectMaterials.clear();
    m_materialLookup.clear();

    OBJECT_MATERIAL goldMaterial;
    goldMaterial.ambientColor    = glm::vec3(0.2f, 0.2f, 0.1f);
    goldMaterial.ambientStrength = 0.4f;
    goldMaterial.diffuseColor    = glm::vec3(0.3f, 0.3f, 0.2f);
    goldMaterial.specularColor   = glm::vec3(0.6f, 0.5f, 0.4f);
    goldMaterial.shininess       = 22.0f;
    goldMaterial.tag             = "gold";
    m_objectMaterials.push_back(goldMaterial);

    OBJECT_MATERIAL cementMaterial;
    cementMaterial.ambientColor    = glm::vec3(0.2f, 0.2f, 0.2f);
    cementMaterial.ambientStrength = 0.2f;
    cementMaterial.diffuseColor    = glm::vec3(0.5f, 0.5f, 0.5f);
    cementMaterial.specularColor   = glm::vec3(0.4f, 0.4f, 0.4f);
    cementMaterial.shininess       = 0.5f;
    cementMaterial.tag             = "cement";
    m_objectMaterials.push_back(cementMaterial);

    OBJECT_MATERIAL woodMaterial;
    woodMaterial.ambientColor    = glm::vec3(0.4f, 0.3f, 0.1f);
    woodMaterial.ambientStrength = 0.2f;
    woodMaterial.diffuseColor    = glm::vec3(0.3f, 0.2f, 0.1f);
    woodMaterial.specularColor   = glm::vec3(0.1f, 0.1f, 0.1f);
    woodMaterial.shininess       = 0.3f;
    woodMaterial.tag             = "wood";
    m_objectMaterials.push_back(woodMaterial);

    OBJECT_MATERIAL tileMaterial;
    tileMaterial.ambientColor    = glm::vec3(0.2f, 0.3f, 0.4f);
    tileMaterial.ambientStrength = 0.3f;
    tileMaterial.diffuseColor    = glm::vec3(0.3f, 0.2f, 0.1f);
    tileMaterial.specularColor   = glm::vec3(0.4f, 0.5f, 0.6f);
    tileMaterial.shininess       = 25.0f;
    tileMaterial.tag             = "tile";
    m_objectMaterials.push_back(tileMaterial);

    OBJECT_MATERIAL glassMaterial;
    glassMaterial.ambientColor    = glm::vec3(0.4f, 0.4f, 0.4f);
    glassMaterial.ambientStrength = 0.3f;
    glassMaterial.diffuseColor    = glm::vec3(0.3f, 0.3f, 0.3f);
    glassMaterial.specularColor   = glm::vec3(0.6f, 0.6f, 0.6f);
    glassMaterial.shininess       = 85.0f;
    glassMaterial.tag             = "glass";
    m_objectMaterials.push_back(glassMaterial);

    OBJECT_MATERIAL clayMaterial;
    clayMaterial.ambientColor    = glm::vec3(0.2f, 0.2f, 0.3f);
    clayMaterial.ambientStrength = 0.3f;
    clayMaterial.diffuseColor    = glm::vec3(0.4f, 0.4f, 0.5f);
    clayMaterial.specularColor   = glm::vec3(0.2f, 0.2f, 0.4f);
    clayMaterial.shininess       = 0.5f;
    clayMaterial.tag             = "clay";
    m_objectMaterials.push_back(clayMaterial);

    // Enhancement: add a "ceramic" material actually used in RenderScene
    OBJECT_MATERIAL ceramicMaterial;
    ceramicMaterial.ambientColor    = glm::vec3(0.8f, 0.8f, 0.9f);
    ceramicMaterial.ambientStrength = 0.4f;
    ceramicMaterial.diffuseColor    = glm::vec3(0.7f, 0.7f, 0.8f);
    ceramicMaterial.specularColor   = glm::vec3(0.9f, 0.9f, 1.0f);
    ceramicMaterial.shininess       = 32.0f;
    ceramicMaterial.tag             = "ceramic";
    m_objectMaterials.push_back(ceramicMaterial);

    // Build hash map for O(1) material lookup
    for (const auto& mat : m_objectMaterials)
    {
        m_materialLookup[mat.tag] = mat;
    }
}

/***********************************************************
 *  SetupSceneLights()
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
    if (m_pShaderManager == nullptr)
    {
        return;
    }

    // First light - Warmer light focused on the wood
    m_pShaderManager->setVec3Value("lightSources[0].position",       0.0f, 1.5f, 0.0f);
    m_pShaderManager->setVec3Value("lightSources[0].diffuseColor",   0.4f, 0.3f, 0.2f);
    m_pShaderManager->setVec3Value("lightSources[0].specularColor",  0.0f, 0.0f, 0.0f);
    m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 64.0f);
    m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.1f);

    // Second light - Soft fill light coming from the camera side
    m_pShaderManager->setVec3Value("lightSources[1].position",       0.0f, 1.2f, 2.0f);
    m_pShaderManager->setVec3Value("lightSources[1].diffuseColor",   0.3f, 0.3f, 0.3f);
    m_pShaderManager->setVec3Value("lightSources[1].specularColor",  0.0f, 0.0f, 0.0f);
    m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 90.0f);
    m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.05f);

    // Enable lighting
    m_pShaderManager->setBoolValue(g_UseLightingName, true);
}

/***********************************************************
 *  LoadSceneTextures()
 *
 *  Load textures and bind them to texture slots
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
    bool bReturn = false;

    bReturn = CreateGLTexture(
        "../../Utilities/textures/wood.jpg",
        "wood");

    bReturn = CreateGLTexture(
        "../../Utilities/textures/greencup.png",
        "Mug");

    bReturn = CreateGLTexture(
        "../../Utilities/textures/light.jpg",
        "light");

    bReturn = CreateGLTexture(
        "../../Utilities/textures/stainedglass.jpg",
        "glass");

    bReturn = CreateGLTexture(
        "../../Utilities/textures/gold-seamless-texture.jpg",
        "gold");

    // after the texture image data is loaded into memory,
    // bind loaded textures to slots (up to 16)
    BindGLTextures();
}

/***********************************************************
 *  PrepareScene()
 *
 *  Prepare the 3D scene by loading shapes, textures, and
 *  materials into memory.
 ***********************************************************/
void SceneManager::PrepareScene()
{
    // load the textures for the 3D scene
    LoadSceneTextures();

    // define materials and lights
    DefineObjectMaterials();
    SetupSceneLights();

    // only one instance of a particular mesh needs to be loaded
    // in memory no matter how many times it is drawn
    m_basicMeshes->LoadBoxMesh();
    m_basicMeshes->LoadPlaneMesh();
    m_basicMeshes->LoadCylinderMesh();
    m_basicMeshes->LoadConeMesh();
    m_basicMeshes->LoadPrismMesh();
    m_basicMeshes->LoadPyramid4Mesh();
    m_basicMeshes->LoadSphereMesh();
    m_basicMeshes->LoadTaperedCylinderMesh();
    m_basicMeshes->LoadTorusMesh();
}

/***********************************************************
 *  RenderScene()
 *
 *  Render the 3D scene by transforming and drawing shapes.
 ***********************************************************/
void SceneManager::RenderScene()
{
    glm::vec3 scaleXYZ;
    float XrotationDegrees = 0.0f;
    float YrotationDegrees = 0.0f;
    float ZrotationDegrees = 0.0f;
    glm::vec3 positionXYZ;

    /*** Render the Table ***/
    scaleXYZ    = glm::vec3(12.0f, 0.3f, 12.0f);
    positionXYZ = glm::vec3(0.0f, -3.0f, 0.0f);
    XrotationDegrees = YrotationDegrees = ZrotationDegrees = 0.0f;
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderMaterial("wood");
    SetShaderTexture("wood");
    m_basicMeshes->DrawCylinderMesh();

    /*** Render the Lamp Base ***/
    scaleXYZ    = glm::vec3(0.8f, 1.5f, 0.8f);
    positionXYZ = glm::vec3(0.0f, -1.95f, -1.0f);
    XrotationDegrees = YrotationDegrees = ZrotationDegrees = 0.0f;
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderMaterial("gold");
    SetShaderTexture("gold");
    m_basicMeshes->DrawCylinderMesh();

    /*** Render the Lamp Shade ***/
    scaleXYZ    = glm::vec3(1.2f, 1.2f, 1.2f);
    positionXYZ = glm::vec3(0.0f, -0.25f, -1.0f);
    XrotationDegrees = YrotationDegrees = ZrotationDegrees = 0.0f;
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderMaterial("glass");
    SetShaderTexture("light"); // fixed: tag matches LoadSceneTextures
    m_basicMeshes->DrawConeMesh();

    /*** Render the Coffee Mug ***/
    scaleXYZ    = glm::vec3(0.6f, 0.7f, 0.6f);
    positionXYZ = glm::vec3(1.5f, -2.85f, -1.2f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = 30.0f;
    ZrotationDegrees = 0.0f;
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderMaterial("ceramic"); // now defined in materials
    SetShaderTexture("Mug");
    m_basicMeshes->DrawCylinderMesh();

    /*** Render the Book ***/
    scaleXYZ    = glm::vec3(1.5f, 0.2f, 1.0f);
    positionXYZ = glm::vec3(-1.2f, -2.7f, -1.5f);
    XrotationDegrees = YrotationDegrees = ZrotationDegrees = 0.0f;
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(0.5f, 0.2f, 0.1f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    /*** Render the Laptop Base ***/
    scaleXYZ    = glm::vec3(2.5f, 0.2f, 1.8f);
    positionXYZ = glm::vec3(-0.5f, -2.7f, 0.5f);
    XrotationDegrees = YrotationDegrees = ZrotationDegrees = 0.0f;
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    /*** Render the Laptop Screen ***/
    scaleXYZ    = glm::vec3(2.5f, 1.5f, 0.2f);
    positionXYZ = glm::vec3(-0.5f, -1.3f, 1.0f);
    XrotationDegrees = -60.0f;
    YrotationDegrees = ZrotationDegrees = 0.0f;
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(0.3f, 0.3f, 0.3f, 1.0f);
    m_basicMeshes->DrawPlaneMesh();
}