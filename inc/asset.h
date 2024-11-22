#pragma once

#include <cstdint>
#include "vectorMath.h"

// For these macros, the cast to uint8_t then back to void is kind of nasty but it *should* optimize out in the compiler.
// Since it's not getting moved from a general purpose register to a floating point register, it's probably fine. probably.

// Standard Buffer Size is the maximum size any alias can be.
#define STANDARD_BUFFER_SIZE = 36
#define ASSET_IS_MIN_TYPE(Asset)  ((*(((uint8_t*)Asset) + 45)) == 0xff)
#define ASSET_IS_TYPE(Asset, Type) ((*(((uint8_t*)Asset) + 45)) == Type)
#define GET_ASSET_TRANSFORM(Asset) ((Matrix*)(((uint8_t*)Asset) + 48))
#define GET_ASSET_PARENT(Asset) ((void*)(((uint8_t*)Asset) + 112))

typedef struct min_asset {
    // Minimum Asset Definition, 48 bytes long. Only stores the alias, references, type and flags.
    #define MIN_ASSET_BODY()\
    char Alias[36]{'\0'};               /* 0    |   36      <--+-- These will be the same for any managed object.             */  \
    uint64_t References = 0;            /* 36   |   8       <-/                                                               */  \
    union Data {uint8_t Type = 0xff;    /* 44   |   x           _- Only use the upper 24 bits, the first 8 represent type.    */  \
    uint32_t Flags;};                   /* 44   |   4       <--+-- General purpose bit flags. useful for keeping object state.*/  \

    MIN_ASSET_BODY()

} min_asset;


typedef struct asset {
    // Holds basic information that all objects in a scene will have. Instead of using inheritance & polymorphism, which has some 
    // overhead, pass objects as void* then static cast to the actual type. This has some advantages in that any generic operation, 
    // like moving, rotating, scaling, etc can be done without a class-specific override. This also allows for more frequent cache hits
    // because the data for an asset is always going to be exactly 128 bytes
    //
    // The union is used because by default, the c preprocessor will try to pack things into 4 bytes. 
    //
    //                                  Offset  | Size in bytes    
    #define ASSET_BODY(type)\
    char Alias[36]{'\0'};               /* 0    |   36      <--+-- These will be the same for any managed object.             */  \
    uint64_t References = 0;            /* 36   |   8       <-/                                                               */  \
    union Data {uint8_t Type = type;    /* 44   |   x           _- Only use the upper 24 bits, the first 8 represent type.    */  \
    uint32_t Flags;};                   /* 44   |   4       <--+-- General purpose bit flags. useful for keeping object state.*/  \
    Matrix Transform = MatrixIdentity();/* 48   |   64      <----- 4 * 4 matrix, represents the local position.               */  \
    void* Parent = nullptr;             /* 112  |   8       <----- pointer to the parent node.                                */  \
    void** Children = nullptr;          /* 120  |   8       <----- pointer to array of child nodes.                           */  \
    
    ASSET_BODY(0x01);

} asset;


namespace ObjectType {
    const uint8_t Minimum       = 0xff; 
    const uint8_t None          = 0x01;
    const uint8_t StaticMesh    = 0x02;
    const uint8_t SkinnedMesh   = 0x03;
    const uint8_t Text          = 0x04;
    const uint8_t Camera        = 0x05;
}


void SetAlias(void* gameObject, const char* string); 
Matrix GetGlobalTransform(void* gameObject); 


