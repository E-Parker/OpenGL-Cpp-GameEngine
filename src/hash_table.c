#include <stdlib.h>

#include "assert.h"
#include "cStringUtilities.h"
#include "hash_table.h"

uint64_t internal_Pow2Ceiling(uint64_t size, uint64_t num) {
    /* This function returns the next nearest power of 2 from the input number. */

    assert(num > 1);

    uint64_t iterations = size * 8;
    num--;

    for (int i = 1; i < iterations; i = i << 1) {
        num |= num >> i;
    }

    return ++num;
}


HashTable* internal_HashTable_create(uint64_t itemSize, uint64_t size) {

    uint64_t Size = (size < 16) ? 16 : Pow2Ceiling(uint64_t, size);

    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    assert(table != NULL);

    table->Array = calloc(Size, sizeof(HashTableItem));
    assert(table->Array != NULL);

    table->Size = Size;
    table->SlotsUsed = 0;

}

void HashTable_destroy(HashTable** table) {

    for (uint64_t i = 0; i < (*table)->Size; i++) {
         
        HashTableItem item = (*table)->Array[i];

        // If there is no value to delete, continue.
        if (item.Value == NULL) {
            continue;
        }

        // MSVC you are so unbelievably stupid. 
        // this is literally BY FUCKING DEFININITION a real value since using calloc. Shut the fuck up!!!!
        free(item.Value);
    }

    free((*table)->Array);
    free(*table);
    *table = NULL;

}

char* HashTable_insert(HashTable* table, const char* key, void* value) {

    // if out of space, double the size.
    if(++table->SlotsUsed == table->Size) {
        HashTable_resize(table, table->Size << 1);
    }

    char* keyEnd = FindBufferEnd(key);
    uint64_t hash = fnvHash64(key, keyEnd) % table->Size;
    uint64_t originalHash = hash;

    while (table->Array[hash].Key != NULL) {

        if (strcmp(key, table->Array[hash].Key) == 0) {
            break;
        }

        hash++;
        hash %= table->Size;

        if (originalHash == hash) {
            // Something has gone wrong and there is no space.
            assert(false);
        }
    }
    
    // Copy the key across
    table->Array[hash].Key = (char*)malloc(keyEnd - key + 1);
    assert(table->Array[hash].Key != NULL);

    memcpy(table->Array[hash].Key, key, keyEnd - key + 1);
    table->Array[hash].Value = value;
    return table->Array[hash].Key;
}

void HashTable_remove(HashTable* table, const char* key) {

    char* keyEnd = FindBufferEnd(key);
    uint64_t hash = fnvHash64(key, keyEnd) % table->Size;
    uint64_t originalHash = hash;

    while (table->Array[hash].Key != NULL) {

        if (strcmp(key, table->Array[hash].Key) == 0) {
            free(table->Array[hash].Key);
            free(table->Array[hash].Value);
            table->Array[hash].Key = NULL;
            table->Array[hash].Value = NULL;
            return;
        }

        hash++;
        hash %= table->Size;

        if (originalHash == hash) {
            return;
        }
    }
}

void HashTable_resize(HashTable* table, const uint64_t size) {

    HashTableItem* Temp = (HashTableItem*)calloc(size, sizeof(HashTableItem));
    assert(Temp != NULL);

    for (uint64_t i = 0; i < table->Size; i++) {

        // if this slot was unused, skip it.
        if (table->Array[i].Key == NULL) {
            continue;
        }

        // Generate the hash for the item.
        uint64_t hash = fnvHash64(table->Array[i].Key, FindBufferEnd(table->Array[i].Key)) % size;
        uint64_t originalHash = hash;

        // Check for collisions, linearly probe for a free slot.
        while (Temp[hash].Key != NULL) {
            hash++;
            hash %= size;

            if (originalHash == hash) {
                // something went really wrong and an open slot could not be found.
                assert(false);
            }
        }
        // Store the item from the old array in the temp one.
        Temp[hash] = table->Array[i];
    }

    free(table->Array);
    table->Array = Temp;
    Temp = NULL;
    table->Size = size;
}


bool internal_HashTable_find(const HashTable* table, const char* key, void** outValue) {

    uint64_t hash = fnvHash64(key, FindBufferEnd(key)) % table->Size;
    uint64_t originalHash = hash;

    while (table->Array[hash].Key != NULL) {

        if (strcmp(key, table->Array[hash].Key) == 0) {
            break;
        }

        hash++;
        hash %= table->Size;

        if (originalHash == hash) {
            *outValue = NULL;
            return false;
        }
    }
    assert(outValue != NULL);
    *outValue = table->Array[hash].Value;
    return true;
}
