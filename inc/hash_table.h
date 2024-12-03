#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "cStringUtilities.h"

#define Pow2Ceiling(T, num) ((T)internal_Pow2Ceiling(sizeof(T), num))
uint64_t internal_Pow2Ceiling(uint64_t size, uint64_t num);

typedef struct HashTableItem {
    /* An object in a hash table. */
    char* Key;
    void* Value;
} HashTableItem;

typedef struct HashTable {
    uint64_t Size;
    uint64_t SlotsUsed;
    HashTableItem* Array;
} HashTable;

#define HashTable_create(T, size) internal_HashTable_create(sizeof(T), size)
HashTable* internal_HashTable_create(uint64_t itemSize, uint64_t size);
void HashTable_destroy(HashTable** table);
char* HashTable_insert(HashTable* table, const char* alias, void* value);
void HashTable_remove(HashTable* table, const char* alias);
void HashTable_resize(HashTable* table, const uint64_t size);

#define HashTable_find(table, alias, outValue) (internal_HashTable_find(table, alias, (void**)outValue))
bool internal_HashTable_find(const HashTable* table, const char* alias, void** outValue);


#ifdef __cplusplus
}
#endif
