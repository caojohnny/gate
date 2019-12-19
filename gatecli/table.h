/**
 * This file contains the prototypes for operating on a
 * hashtable, which is an efficient data structure mapping
 * keys to values. String keys mapping to pointer values
 * are supported, which allows quick access to values given
 * a key.
 */

#ifndef GATE_PARENT_TABLE_H
#define GATE_PARENT_TABLE_H

typedef struct gatecli_table_entry gatecli_table_entry;

/**
 * Represents a key-value pair contained in a hashtable's
 * entries data.
 */
struct gatecli_table_entry {
    /**
     * The string key.
     */
    char *key;
    /**
     * The value mapped to the key.
     */
    void *value;
    /**
     * The next entry in the bucket
     */
    gatecli_table_entry *next;
};

/**
 * Represents a hashtable mapping string keys to void *
 * values.
 */
typedef struct {
    /**
     * The number of actual mappings contained in the
     * hashtable.
     */
    int size;
    /**
     * The number of buckets in this hashtable.
     */
    int buckets_len;
    /**
     * The data contained in this hashtable, in the form of
     * an array of buckets.
     */
    gatecli_table_entry **buckets;
} gatecli_table;

/**
 * Creates a new, empty hashtable.
 *
 * @return a new, empty hashtable
 */
gatecli_table gatecli_table_new();

/**
 * Puts a key-value pair into the given hashtable.
 *
 * @param tab the table into which to put the mapping
 * @param key the key
 * @param value the value
 * @return the old mapping, or NULL if there was none
 */
void *gatecli_table_put(gatecli_table *tab, char *key, void *value);

/**
 * Gets the value mapped to the given key in the given
 * hashtable.
 *
 * @param tab the hashtable from which to obtain the value
 * @param key the key for which to obtain the value
 * @return the value, or NULL if the key isn't mapped
 */
void *gatecli_table_get(gatecli_table *tab, char *key);

/**
 * Removes a mapping from the given hashtable.
 *
 * @param tab the table from which to remove the mapping
 * @param key the key for which to remove the mapping
 * @return the value removed, or NULL if nothing was
 * removed
 */
void *gatecli_table_rem(gatecli_table *tab, char *key);

/**
 * Clears and removes all data associated with the given
 * hashtable.
 *
 * @param tab the hashtable to clear
 */
void gatecli_table_free(gatecli_table *tab);

#endif // GATECLI_TABLE_H
