package example.com.ContactServices;

/**
 * Central validation class.
 * Contains reusable validation algorithms for Contact fields.
 */
public final class ContactValidator {

    private ContactValidator() {} // Prevent instantiation

    public static String validateContactId(String id) {
        if (id == null || id.length() > 10) {
            throw new IllegalArgumentException("Invalid contact ID");
        }
        return id;
    }

    public static String validateFirstName(String name) {
        if (name == null || name.length() > 10) {
            throw new IllegalArgumentException("Invalid first name");
        }
        return name;
    }

    public static String validateLastName(String name) {
        if (name == null || name.length() > 10) {
            throw new IllegalArgumentException("Invalid last name");
        }
        return name;
    }

    public static String validatePhone(String phone) {
        // Uses regex algorithm to check for digits only
        if (phone == null || phone.length() != 10 || !phone.matches("\\d+")) {
            throw new IllegalArgumentException("Invalid phone number");
        }
        return phone;
    }

    public static String validateAddress(String address) {
        if (address == null || address.length() > 30) {
            throw new IllegalArgumentException("Invalid address");
        }
        return address;
    }
}