package example.com.ContactServices;

public class Contact {

    private final String contactId;
    private String firstName;
    private String lastName;
    private String phone;
    private String address;

    public Contact(String contactId, String firstName, String lastName, String phone, String address) {
        this.contactId = ContactValidator.validateContactId(contactId);
        this.firstName = ContactValidator.validateFirstName(firstName);
        this.lastName = ContactValidator.validateLastName(lastName);
        this.phone = ContactValidator.validatePhone(phone);
        this.address = ContactValidator.validateAddress(address);
    }

    public String getContactId() {
        return contactId;
    }
    public String getFirstName() { return firstName; }
    public String getLastName() { return lastName; }
    public String getPhone() { return phone; }
    public String getAddress() { return address; }

    public void setFirstName(String firstName) {
        this.firstName = ContactValidator.validateFirstName(firstName);
    }
    public void setLastName(String lastName) {
        this.lastName = ContactValidator.validateLastName(lastName);
    }
    public void setPhone(String phone) {
        this.phone = ContactValidator.validatePhone(phone);
    }
    public void setAddress(String address) {
        this.address = ContactValidator.validateAddress(address);
    }
}