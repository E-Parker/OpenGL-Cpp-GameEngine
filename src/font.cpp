#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_truetype.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include "vectorMath.h"
#include "hashTable.h"
#include "material.h"
#include "camera.h"
#include "font.h"
#include "mesh.h"

// Generate truetype body here since it's only needed here.
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"


static HashTable<Font> FontTable(512);

#define DEFAULT_START_CHARACTER 31

Font* FontManager::InternalLoadFont(const char* path, Material* material, const float pointSize = 12.0f) {
    
    uint8_t* fontDataBuf;
    stbtt_fontinfo fontInfo;

    try {

        // Try to open the file.
        std::ifstream inputStream(path, ios::in | ios::binary | ios::ate);

        // Get the size of the file.
        inputStream.seekg(0, std::ios::end);
        std::streampos&& fontFileSize = inputStream.tellg();
        inputStream.seekg(0, std::ios::beg);

        fontDataBuf = new uint8_t[fontFileSize];

        inputStream.read((char*)fontDataBuf, fontFileSize);

        inputStream.close();

        if (!stbtt_InitFont(&fontInfo, fontDataBuf, 0)) {
            std::cout << "stbtt_InitFont() Failed!" << std::endl;
        }
    }
    catch(std::ifstream::failure& e) {
        std::cout << "Error creating Font: could not read file, " << e.what() << std::endl;
        return nullptr;
    }
    
    uint32_t atlasSize = Pow2Ceiling((uint32_t)(pointSize * sqrt((double)(fontInfo.numGlyphs))));

    Font* font = new Font(material, fontInfo.numGlyphs, atlasSize);
    font->FontSize = pointSize + 3.0f;  // I don't know why but +3.0f makes it work. :/

    // Tell stb_trueType the parameters for packing the incoming font.
    stbtt_pack_context packContext;
    stbtt_PackBegin(&packContext, font->fontAtlasTextureData, font->AtlasSize, font->AtlasSize, font->AtlasSize, 1, nullptr);
    stbtt_PackFontRange(&packContext, fontDataBuf, 0, (float)font->FontSize, DEFAULT_START_CHARACTER, font->CharactersLoaded, font->packedChars);
    stbtt_PackEnd(&packContext);

    // Loop over every pixel and pack it 
    for (uint32_t i = 0; i < font->CharactersLoaded; i++) {
        float posX, posY;
        stbtt_GetPackedQuad(font->packedChars, font->AtlasSize, font->AtlasSize, i, &posX, &posY, &font->alignedQuads[i], 0);
    }

    return font;
}

Font* CreateFont(const char* path, const char* alias, Material* material, const float pointSize) {
    
    Font* font = nullptr;

    // try to find the font in the table.
    FontTable.Find(alias, font);

    // if the font doesn't already exist, make a new one, and return that instead.
    if (font != nullptr) {
        return font;
    }

    font = FontManager::InternalLoadFont(path, material, pointSize);
    TextureManager::InternalCreateTexture(font->textureAtlas, false, alias, GL_RED, GL_RED, font->fontAtlasTextureData, false);
    material->SetTexture(font->textureAtlas, 0);

    if (font == nullptr) {
        std::cout << "Error creating Font: \"" << alias << "\" From: \"" << path << "\". The Font will be discarded." << std::endl;
        return nullptr;
    }

    // Reference the key in font struct for faster insertions.
    font->alias = FontTable.Insert(alias, font);

    return font;

}

void DeleteFont(const char* alias) {
    /* This function manages deleting fonts from the table. The fonts will be deleted from graphics memory if it isn't referenced anywhere.
    This will only work for fonts created with the system, Create fonts manually at your own risk!! */

    Font* font = nullptr;

    if (alias == nullptr) {
        return;
    }

    FontTable.Find(alias, font);

    if (font == nullptr) {
        std::cout << "Error deleting Font: \"" << alias << "\". No font with that name found. " << std::endl;
        return;
    }

    if (--font->references == 0) {
        FontTable.Delete(alias);
    }
}


void DereferenceFonts() {
    /* Call this function at the end of your program to ensure all tracked textures are properly cleaned up. */

    // Iterate through all the positions in the hash table.
    for (uint64_t i = 0; i < FontTable.Size; i++) {

        // Check if there is a value stored here:
        if (FontTable.Array[i].Key == nullptr) {
            continue;
        }

        Font* font = FontTable.Array[i].Value;

        // Forcefully clear the memory for all textures marked as managed.
        if (FontTable.Array[i].isManaged) {
            std::cout << "Font Manager: Freeing texture, \"" << font->alias << "\"." << std::endl;
            FontTable.Delete(font->alias);
        }
    }
}

TextRender::TextRender() {
    font = nullptr;
    textMesh = new Mesh();
}

TextRender::~TextRender() {
    
    if(textMesh == nullptr) {
        return;
    }

    InternalFreeMesh(textMesh);
    delete textMesh;
    textMesh = nullptr;
}

void DrawTextMesh(TextRender* textRender, Camera* camera, double aspectRatio) {
    /* Draw text to the screen. */

    // if the textRender is invalid, leave early without drawing anything.
    bool validTextRender = (
        textRender != nullptr &&
        textRender->font != nullptr &&
        textRender->font->material != nullptr );

    if(!validTextRender) {
        std::cout << "failed to draw text" << std::endl;
        return;
    }

    // Calculate the projection. in this case its just an Orthographic projection to show up in screen-space.
    Matrix mvp = MatrixIdentity() * Ortho(-aspectRatio, aspectRatio, -1.0, 1.0, 1.0, -1.0);


    textRender->font->material->BindMaterial();
    GLint uniform = glGetUniformLocation(textRender->font->material->Program, "u_mvp");
    GLint color = glGetUniformLocation(textRender->font->material->Program, "u_color");
    
    // Bind the VAO and draw the elements.
    glBindVertexArray(textRender->textMesh->VertexAttributeObject);
    glUniformMatrix4fv(uniform, 1, GL_FALSE, ToFloat16(mvp).v);
    glUniform3f(color, textRender->color[0], textRender->color[1], textRender->color[2]);
    glDrawElements(GL_TRIANGLES, textRender->textMesh->indexBytes, GL_UNSIGNED_SHORT, 0);

    // unbind the VAO.
    glBindVertexArray(GL_NONE);
}

void SetText(TextRender* textRender, const char* string, int x, int y, const float windowWidth, const float windowHeight, const float size) {

    // leave early if the font or text render is null.
    if (textRender == nullptr || textRender->font == nullptr) {
        return;
    }

    // convert character position to normalized device coordinates.
    float pixelScale = 2.0f / windowHeight;

    Vector2 position;
    position.x = (float)x * textRender->font->packedChars[1].xadvance * pixelScale * size;
    position.y = (float)y * textRender->font->FontSize * pixelScale * size;

    position.x -= (windowWidth / windowHeight);
    position.y += 1.0f - (textRender->font->FontSize * pixelScale * size);

    Vector2 localPosition = position;

    char* bufferEnd = FindBufferEnd(string);
    char* character = const_cast<char*>(string);

    // Determine the size of the mesh.
    uint16_t VertexBufferSize = static_cast<uint16_t>((bufferEnd - string) * 4);
    uint16_t ElementBufferSize = static_cast<uint16_t>((bufferEnd - string) * 6);

    // Make enough vectors to store each face for each quad generated.
    Vector3* vertices = new Vector3[VertexBufferSize];
    Vector3* normals = new Vector3[VertexBufferSize];
    Vector2* tChoords = new Vector2[VertexBufferSize];
    uint16_t* elements = new uint16_t[ElementBufferSize];

    uint16_t vertexIndex = 0;
    uint16_t elementIndex = 0;

    for (; character < bufferEnd; character++) {
       
        // Check if the current character is in the font atlas.
        if ((*character < DEFAULT_START_CHARACTER || *character > DEFAULT_START_CHARACTER + textRender->font->CharactersLoaded) && *character != '\n') {
            continue;
        }

        // If there's a new line, update the position.
        if (*character == '\n') {
            localPosition.y -= textRender->font->FontSize * pixelScale * size;
            localPosition.x = position.x;
            continue;
        }

        // Get the graphic for the font:
        stbtt_packedchar* packedChar = &textRender->font->packedChars[*character - DEFAULT_START_CHARACTER];
        stbtt_aligned_quad* alignedQuad = &textRender->font->alignedQuads[*character - DEFAULT_START_CHARACTER];

        Vector2 glyphSize{
            (packedChar->x1 - packedChar->x0) * pixelScale * size,
            (packedChar->y1 - packedChar->y0) * pixelScale * size
        };

        // Calculate bounding box:
        Vector3 glyphBB_BottomLeft{
            localPosition.x + (packedChar->xoff * pixelScale * size),
            localPosition.y - (packedChar->yoff + packedChar->y1 - packedChar->y0) * pixelScale * size,
            0.0f
        };

        Vector3 glyphBB_TopRight {
            glyphBB_BottomLeft.x + glyphSize.x,
            glyphBB_BottomLeft.y + glyphSize.y,
            0.0f
        };

        Vector3 glyphBB_TopLeft{
            glyphBB_BottomLeft.x,
            glyphBB_BottomLeft.y + glyphSize.y,
            0.0f
        };

        Vector3 glyphBB_BottomRight{
            glyphBB_BottomLeft.x + glyphSize.x, 
            glyphBB_BottomLeft.y,
            0.0f
        };
         
        // Quad from bounding box.
        Vector3 glyphVertices[4]{
            glyphBB_TopRight, 
            glyphBB_TopLeft, 
            glyphBB_BottomLeft, 
            glyphBB_BottomRight,
        };

        Vector3 glyphNormals[4]{
            {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
        };

        Vector2 glyphTCoords[4]{
            { alignedQuad->s1, alignedQuad->t0 },
            { alignedQuad->s0, alignedQuad->t0 },
            { alignedQuad->s0, alignedQuad->t1 },
            { alignedQuad->s1, alignedQuad->t1 },
        };

        uint16_t glyphElements[6]{
            vertexIndex,
            static_cast<uint16_t>(vertexIndex + 1),
            static_cast<uint16_t>(vertexIndex + 2),
            vertexIndex,
            static_cast<uint16_t>(vertexIndex + 2),
            static_cast<uint16_t>(vertexIndex + 3),
        };

        // Write the current quad into the vertices and tChoords.
        memcpy(&vertices[vertexIndex], glyphVertices, 4 * sizeof(Vector3));
        memcpy(&normals[vertexIndex], glyphNormals, 4 * sizeof(Vector3));
        memcpy(&tChoords[vertexIndex], glyphTCoords, 4 * sizeof(Vector2));
        memcpy(&elements[elementIndex], glyphElements, 6 * sizeof(uint16_t));
        
        vertexIndex += 4;
        elementIndex += 6;

        // Move position by the amount needed to space out the characters.
        localPosition.x += packedChar->xadvance * pixelScale * size;
    }
    
    // Upload the mesh.
    InternalUploadMesh(textRender->textMesh, elements, vertices, normals, tChoords, ElementBufferSize, VertexBufferSize);
    
    // clean up arrays.
    delete[] vertices;
    delete[] normals;
    delete[] tChoords;
    delete[] elements;
}


void SetFont(TextRender* textRender, const char* fontName, Font* defaultFont) {
    /* Set the font for a text render from the alias of the font. */

    if (textRender->font != nullptr) {
        DeleteFont(textRender->font->alias);
        textRender->font = nullptr;
    }

    Font* font = nullptr;

    FontTable.Find(fontName, font);

    if (font != nullptr) {        
        textRender->font = font;
        font->references++;
        return;
    }

    std::cout << "Error setting TextRender font: Font by name \"" << fontName << "\" could not be found, ";

    if (defaultFont != nullptr) {
        std::cout << "using default Font, \"" << defaultFont->alias << "\"." << std::endl;
        textRender->font = defaultFont;
        defaultFont->references++;
        return;
    }

    std::cout << "no default font provided, text will not render." << std::endl;

}
