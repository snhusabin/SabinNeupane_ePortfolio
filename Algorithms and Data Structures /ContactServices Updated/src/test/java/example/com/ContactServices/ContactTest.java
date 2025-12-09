package example.com.ContactServices;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Unit tests for Contact object validation and behavior.
 */
public class ContactTest {

    @Test
    public void testValidContact() {
        Contact c = new Contact("1", "John", "Doe", "1234567890", "123 Main St");
        assertNotNull(c);
    }

    @Test
    public void testInvalidPhoneThrows() {
        assertThrows(IllegalArgumentException.class, () ->
                new Contact("1", "John", "Doe", "1234", "123 Main St"));
    }

    @Test
    public void testSettersValidation() {
        Contact c = new Contact("1", "John", "Doe", "1234567890", "123 Main");
        assertThrows(IllegalArgumentException.class, () -> c.setFirstName(null));
    }
}