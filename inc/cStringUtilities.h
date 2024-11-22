#pragma once

#include <cstdint>
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