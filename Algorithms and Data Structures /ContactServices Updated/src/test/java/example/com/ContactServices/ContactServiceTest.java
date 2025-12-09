package example.com.ContactServices;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.Arrays;
import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Tests the ContactService and its algorithms (search, sort, batch insert).
 */
public class ContactServiceTest {

    private ContactService service;

    @BeforeEach
    public void setup() {
        service = new ContactService();
    }

    @Test
    public void testAddAndRetrieve() {
        Contact c = new Contact("1", "John", "Doe", "1234567890", "St");
        service.addContact(c);
        assertNotNull(service.getContact("1"));
    }

    @Test
    public void testDuplicateIdThrows() {
        service.addContact(new Contact("1", "John", "Doe", "1234567890", "St"));
        assertThrows(DuplicateContactIdException.class, () ->
                service.addContact(new Contact("1", "Jane", "Doe", "0987654321", "Ave")));
    }

    @Test
    public void testFindByLastName() {
        service.addContact(new Contact("1", "John", "Doe", "1234567890", "St"));
        service.addContact(new Contact("2", "Mike", "Doe", "0987654321", "Ave"));

        List<Contact> result = service.findByLastName("Doe");
        assertEquals(2, result.size());
    }

    @Test
    public void testSortContacts() {
        service.addContact(new Contact("1", "Bob", "Smith", "1234567890", "A"));
        service.addContact(new Contact("2", "Alice", "Brown", "0987654321", "B"));
        service.addContact(new Contact("3", "Chris", "Brown", "1111111111", "C"));

        List<Contact> sorted = service.getContactsSortedByName();
        assertEquals("Brown", sorted.get(0).getLastName());
        assertEquals("Alice", sorted.get(0).getFirstName());
    }
}