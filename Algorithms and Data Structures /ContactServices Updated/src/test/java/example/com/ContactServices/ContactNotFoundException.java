package example.com.ContactServices;

/**
 * Thrown when a requested contact does not exist in the data store.
 */
public class ContactNotFoundException extends RuntimeException {
    public ContactNotFoundException(String message) {
        super(message);
    }
}