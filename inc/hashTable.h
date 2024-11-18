#pragma once

#include <cassert>
#include <cstring>


inline uint64_t fnvHash64(const char* buffer, const char* const bufferEnd) {
    /* implementation of the fnv64 hashing function, created by Glenn Fowler, Landon Curt Noll,
    and Kiem-Phong Vo. I used fixed-width integers here for maximum portability. */

    const uint64_t Prime = 0x00000100000001B3;
    uint64_t Hash = 0xCBF29CE484222325;

    char* bufferIter = const_cast<char*>(buffer);

    // Iterate from the buffer start address to the buffer end address.
    for (; bufferIter < bufferEnd; bufferIter++) {
        //XOR the current hash with the current character, then multiply by an arbitrary prime number.
        Hash = (Hash ^ (*bufferIter)) * Prime;
    }
    return Hash;
}

inline char* FindBufferEnd(const char* buffer) {

    char* bufferEnd = const_cast<char*>(buffer);

    // Assume the key is a c_string, iterate through to find the null terminator.
    for (uint16_t i = 0; i < 0xFFFF; i++) {
        if (*bufferEnd == '\0') {
            break;
        }
        bufferEnd++;
    }

    return bufferEnd;
}

template<typename T> T Pow2Ceiling(T num) {
    /* This function returns the next nearest power of 2 from the input number. */

    assert(num > 1);

    uint64_t iterations = sizeof(T) * 8;
    num--;

    for (int i = 1; i < iterations; i = i << 1) {
        num |= num >> i;
    }

    return ++num;
}


template<typename T> class HashTable {
    /* Hash table with key indexing and dynamic resizing. */

    struct HashTableItem {
        /* An object in a hash table. */
        char* Key;
        char* KeyEnd;
        T* Value;
        bool isManaged;
        uint64_t KeyLength;

        inline HashTableItem() : Key(nullptr), KeyEnd(nullptr), Value(nullptr), KeyLength(0), isManaged(true) { }

        inline ~HashTableItem() {
            /* This function does not manage the value pointer.
            
            !!!! This is intentional !!!! 
            
            Please see HashTable Insert.

            */
            
            if (Key != nullptr) {
                delete[] Key;
            }
            
            Key = nullptr;
            KeyEnd = nullptr;
            Value = nullptr;
        }
    };

public:

    inline HashTable(uint64_t size) {
        Size = (size < 32) ? 32 : Pow2Ceiling<uint64_t>(size);
        Array = new HashTable<T>::HashTableItem[Size];
        SlotsUsed = 0;
    }

    inline ~HashTable() {

        for (uint64_t i = 0; i < Size; i++) {
            
            // If there is no value to delete, continue.
            if (Array[i].Value == nullptr) {
                continue;
            }

            // If the item is managed, (Dangling Pointer) delete it.
            if (Array[i].isManaged) {
                delete Array[i].Value;
            }
        }

        delete[] Array;
        Array = nullptr;
    }

    inline bool Find(const char* key, T*& outValue) {

        char* keyEnd = FindBufferEnd(key);

        // Generate the hash for the string.
        uint64_t hash = fnvHash64(key, keyEnd) % Size;
        uint64_t originalHash = hash;

        while (Array[hash].Key != nullptr) {

            if (HashTable::CompareKeys(&Array[hash], key, keyEnd)) {
                outValue = Array[hash].Value;
                return true;
            }

            hash++;
            hash %= Size;

            if (originalHash == hash) {
                return false;
            }
        }
        return false;
    }

    inline bool Delete(const char* key) {

        char* keyEnd = FindBufferEnd(key);

        // Generate the hash for the string.
        uint64_t hash = fnvHash64(key, keyEnd) % Size;
        uint64_t originalHash = hash;

        while (Array[hash].Key != nullptr) {

            if (HashTable::CompareKeys(&Array[hash], key, keyEnd)) {
                // Free the key and mark it as null for reuse. 
                delete[] Array[hash].Key;
                Array[hash].Key = nullptr;
                Array[hash].KeyEnd = nullptr;
                
                // If the item is not a managed resource, delete it.
                if (!Array[hash].isManaged) {
                    delete Array[hash].Value;
                }

                Array[hash].Value = nullptr;
                Array[hash].isManaged = true;
                SlotsUsed--;
                return true;
            }

            hash++;
            hash %= Size;

            if (originalHash == hash) {
                return false;
            }
        }
        return false;
    }

    inline char* Insert(const char* key, T* value, bool isManaged = false) {
        /* Insert an item into the table, returns a pointer to the key. 
       
        The is managed value requires some explanation. This flag controls if the
        value T is a managed resource or not. Since T* values can be from either 
        the stack, the heap or statically declared, some values might need to be freed
        when the HashTable gets destroyed while others do not.
        
        If you don't know what you're doing with this, leave it as false.
        
        */

        // If there are no available slots, expand the table.
        if (++SlotsUsed == Size) {
            Expand();
        }

        char* keyEnd = FindBufferEnd(key);

        // Generate the hash for the string.
        uint64_t hash = fnvHash64(key, keyEnd) % Size;
        uint64_t originalHash = hash;

        // Check for collisions, linearly probe for a free slot.
        while (Array[hash].Key != nullptr) {
            hash++;
            hash %= Size;

            if (originalHash == hash) {
                // something went really wrong and an open slot could not be found.
                assert(false);
            }
        }

        // Found a free slot, create an item and store it there.
       
        // Trying to just override Array[hash] will cause the system to try to delete the new item after it falls out of scope, so doing this here now.
        Array[hash].KeyLength = keyEnd - key;
        
        Array[hash].Key = new char[Array[hash].KeyLength + 1];  // Must be KeyLength + 1 to account for the null terminator.
        
        memcpy(Array[hash].Key, key, Array[hash].KeyLength + 1);
        Array[hash].KeyEnd = Array[hash].Key + Array[hash].KeyLength;
        Array[hash].Value = value;

        return Array[hash].Key;
    }

    inline void Expand() {
        /* This function is insanely expensive. Since the hashes are determined by the initial size,
        everything needs to be recalculated. the size is set to the next nearest power of two to ensure
        this doesn't happen often. */

        uint64_t newSize = Size << 1;   // double the size.

        HashTable<T>::HashTableItem* Temp = new HashTable<T>::HashTableItem[newSize];

        for (uint64_t i = 0; i < Size; i++) {

            // if this slot was unused, skip it.
            if (Array[i].Key == nullptr) {
                continue;
            }

            // Generate the hash for the item.
            uint64_t hash = fnvHash64(Array[i].Key, Array[i].KeyEnd) % newSize;
            uint64_t originalHash = hash;

            // Check for collisions, linearly probe for a free slot.
            while (Temp[hash].Key != nullptr) {
                hash++;
                hash %= newSize;

                if (originalHash == hash) {
                    // something went really wrong and an open slot could not be found.
                    assert(false);
                }
            }
            // Store the item from the old array in the temp one.
            Temp[hash] = Array[i];
        }

        delete[] Array;
        Array = Temp;
        Temp = nullptr;
        Size = newSize;
    }

    inline bool CompareKeys(const HashTableItem* item, const char* key, const char* keyEnd) {
        /* Basically the same as strcmp, the length check might be slightly faster. */ 

        // Leave early if the keys don't have the same length.
        if (item->KeyLength != (keyEnd - key)) {
            return false;
        }

        // since they ARE the same length, now compare the characters until a difference is found. Big O(n)
        char* aIterator = item->Key;
        char* bIterator = const_cast<char*>(key);

        for (; aIterator < item->KeyEnd; aIterator++) {

            // If the value at bIterator is not the same as the one in aIterator, the keys are different.
            if (*bIterator != *aIterator) {
                return false;
            }

            bIterator++;    // aIterator is handled by the for loop, so bIterator needs to be updated here.
        }

        // All comparisons passed, the keys are the same.
        return true;
    }
    
    uint64_t SlotsUsed;
    uint64_t Size;
    HashTableItem* Array;
};
