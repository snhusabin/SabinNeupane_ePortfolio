package example.com.ContactServices;

/**
 * Thrown when attempting to add a contact with a duplicate ID.
 */
public class DuplicateContactIdException extends RuntimeException {
    public DuplicateContactIdException(String message) {
        super(message);
    }
}