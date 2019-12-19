#include <stdlib.h>
#include <string.h>

#include "table.h"

static const int INITIAL_SIZE = 2;

gatecli_table gatecli_table_new() {
    gatecli_table tab = {0, 0, NULL};
    return tab;
}

static int hash_key(const char *key) {
    // java.lang.String hashCode()

    int h = 0;
    for (int i = 0;; ++i) {
        char c = key[i];
        if (c == '\0') {
            break;
        }

        h = 31 * h + c;
    }
    return h;
}

static int key_matches(char *key_one, char *key_two) {
    return strcmp(key_one, key_two) == 0;
}

static gatecli_table_entry *gatecli_table_entry_new(char *key, void *value) {
    gatecli_table_entry *entry = malloc(sizeof(*entry));
    if (entry == NULL) {
        exit(1);
    }

    entry->key = key;
    entry->value = value;
    entry->next = NULL;

    return entry;
}

static void *insert(gatecli_table *tab, char *key, void *value) {
    int hash = hash_key(key);
    int idx = hash % tab->buckets_len;
    gatecli_table_entry *head = tab->buckets[idx];

    // Null bucket head: Start a new bucket here
    if (head == NULL) {
        gatecli_table_entry *entry = gatecli_table_entry_new(key, value);
        tab->buckets[idx] = entry;
        tab->size++;

        return NULL;
    }

    // Non-null bucket head: link to first matching key
    // else link to the last non-null node
    gatecli_table_entry *prev_node;
    gatecli_table_entry *node = head;
    do {
        if (key_matches(node->key, key)) {
            void *old_value = node->value;
            node->value = value;

            return old_value;
        }

        prev_node = node;
        node = node->next;
    } while (node != NULL);

    gatecli_table_entry *entry = gatecli_table_entry_new(key, value);
    prev_node->next = entry;
    tab->size++;

    return NULL;
}

static void ensure_size(gatecli_table *tab, int required) {
    int old_len = tab->buckets_len;
    if (old_len < required) {
        int new_len;
        if (old_len == 0) {
            new_len = INITIAL_SIZE;
        } else {
            new_len = old_len * 2;
        }

        gatecli_table_entry **old_buckets = tab->buckets;
        gatecli_table_entry **new_buckets = calloc(new_len, sizeof(**new_buckets));
        if (new_buckets == NULL) {
            exit(1);
        }

        tab->size = 0;
        tab->buckets_len = new_len;
        tab->buckets = new_buckets;

        // Go through each bucket and relink all of the entries
        // in each bucket to their new position
        for (int i = 0; i < old_len; ++i) {
            for (gatecli_table_entry *node = old_buckets[i];
                 node != NULL;
                 node = node->next) {
                insert(tab, node->key, node->value);
            }
        }
    }
}

void *gatecli_table_put(gatecli_table *tab, char *key, void *value) {
    int new_size = tab->size + 1;
    ensure_size(tab, new_size);

    void *prev_value = insert(tab, key, value);
    return prev_value;
}

void *gatecli_table_get(gatecli_table *tab, char *key) {
    if (tab->buckets_len == 0) {
        return NULL;
    }

    int hash = hash_key(key);
    int idx = hash % tab->buckets_len;

    for (gatecli_table_entry *node = tab->buckets[idx];
         node != NULL;
         node = node->next) {
        if (key_matches(node->key, key)) {
            return node->value;
        }
    }

    return NULL;
}

void *gatecli_table_rem(gatecli_table *tab, char *key) {
    if (tab->buckets_len == 0) {
        return NULL;
    }

    int hash = hash_key(key);
    int idx = hash % tab->buckets_len;

    for (gatecli_table_entry *prev_node = NULL, *node = tab->buckets[idx];
         node != NULL;
         prev_node = node, node = node->next) {
        if (key_matches(node->key, key)) {
            if (prev_node == NULL) {
                // Bucket head removed, shift next node into head
                tab->buckets[idx] = node->next;
            } else {
                // Middle of bucket is removed, orphan current node
                prev_node->next = node->next;
            }
            
            void *prev_value = node->value;
            tab->size--;

            free(node);
            return prev_value;
        }
    }

    return NULL;
}

void gatecli_table_free(gatecli_table *tab) {
    gatecli_table_entry **entries = tab->buckets;
    for (int i = 0; i < tab->buckets_len; ++i) {
        for (gatecli_table_entry *entry = entries[i];
             entry != NULL;
                ) {
            gatecli_table_entry *cur_entry = entry;
            entry = entry->next;
            free(cur_entry);
        }
    }
    free(entries);

    tab->size = 0;
    tab->buckets_len = 0;
    tab->buckets = NULL;
}
