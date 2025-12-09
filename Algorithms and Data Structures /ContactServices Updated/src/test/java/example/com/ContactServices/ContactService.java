package example.com.ContactServices;

import java.util.*;
import java.util.stream.Collectors;

/**
 * Service class that manages contacts using a HashMap.
 * HashMap enables O(1) average-time access for add/update/delete operations.
 */
public class ContactService {

    // Core data structure: Maps contactId -> Contact object
    private final Map<String, Contact> contactsById = new HashMap<>();

    /**
     * Add a new contact. Rejects duplicates.
     */
    public void addContact(Contact contact) {
        if (contact == null) {
            throw new IllegalArgumentException("Contact cannot be null.");
        }
        if (contactsById.containsKey(contact.getContactId())) {
            throw new DuplicateContactIdException("Contact ID already exists.");
        }
        contactsById.put(contact.getContactId(), contact);
    }

    /**
     * Batch add using a temporary validation algorithm.
     * Ensures all contacts are valid before modifying the map.
     */
    public void addContactsBatch(Collection<Contact> contacts) {
        for (Contact c : contacts) {
            if (contactsById.containsKey(c.getContactId())) {
                throw new DuplicateContactIdException("Duplicate in batch: " + c.getContactId());
            }
        }
        // Safe to insert
        for (Contact c : contacts) {
            contactsById.put(c.getContactId(), c);
        }
    }

    /**
     * Delete a contact by ID.
     */
    public void deleteContact(String contactId) {
        if (!contactsById.containsKey(contactId)) {
            throw new ContactNotFoundException("Contact not found.");
        }
        contactsById.remove(contactId);
    }

    /**
     * Update an existing contact.
     */
    public void updateContact(String id, String firstName, String lastName, String phone, String address) {
        Contact c = contactsById.get(id);
        if (c == null) {
            throw new ContactNotFoundException("Contact not found.");
        }
        if (firstName != null) c.setFirstName(firstName);
        if (lastName  != null) c.setLastName(lastName);
        if (phone     != null) c.setPhone(phone);
        if (address   != null) c.setAddress(address);
    }

    /**
     * Get a contact by ID (O(1) lookup).
     */
    public Contact getContact(String contactId) {
        return contactsById.get(contactId);
    }

    /**
     * Search algorithm: find all contacts with matching last name.
     */
    public List<Contact> findByLastName(String lastName) {
        List<Contact> result = new ArrayList<>();
        for (Contact c : contactsById.values()) {
            if (c.getLastName().equalsIgnoreCase(lastName)) {
                result.add(c);
            }
        }
        return result;
    }

    /**
     * Sorting algorithm: return contacts sorted by last name then first name.
     */
    public List<Contact> getContactsSortedByName() {
        return contactsById.values().stream()
                .sorted(Comparator.comparing(Contact::getLastName)
                        .thenComparing(Contact::getFirstName))
                .collect(Collectors.toList());
    }

    public int size() {
        return contactsById.size();
    }
}