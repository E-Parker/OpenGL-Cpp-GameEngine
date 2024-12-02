#include <cstdint>
#include "asset.h"
#include "vectorMath.h"


void SetAlias(void* gameObject, const char* string) {
    /* copy the string into the gameObject alias buffer. */
    char* gameObjectAlias = (char*)gameObject;

    for(uint8_t i = 0; i < 36; i++) {
        if(string[i] == '\0') {
            return;
        }
        gameObjectAlias[i] = string[i];
    }
}


Matrix GetGlobalTransform(void* gameObject) {
    /* This function returns the global transform of any asset. */ 
    Matrix result = *GET_ASSET_TRANSFORM(gameObject);
    void* parentObject;
    
    for(uint16_t i = 0; i < 512; i++) {
        parentObject = GET_ASSET_PARENT(gameObject);
        
        if(parentObject == nullptr) {
            return result;
        }

        result = result * *GET_ASSET_TRANSFORM(parentObject);
    }
    return result;
}


