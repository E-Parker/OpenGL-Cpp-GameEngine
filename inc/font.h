#pragma once

#include <cassert>
#include "stb_truetype.h"

// Forward Declarations:
struct Material;
struct Vector3;
struct Vector2;
struct Mesh;

typedef struct Font {

	// Font Table information:
	char* alias = nullptr;
	uint16_t references = 0;

	// Font atlas information:
	Material* material = nullptr;

	uint8_t* fontAtlasTextureData = nullptr;
	stbtt_packedchar* packedChars = nullptr;
	stbtt_aligned_quad* alignedQuads = nullptr;

	Texture* textureAtlas = nullptr;
	uint16_t CharactersLoaded = 0;
	uint16_t AtlasSize = 0;
	float FontSize = 0.0f;

	Font(Material* material, uint16_t charactersToLoad, uint16_t atlasSize);
	~Font();

} Font;

typedef struct TextRender {
	/* struct to hold the data for rendering text */ 
    
	ASSET_BODY(ObjectType::Text)
	GLfloat color[3]{0.0f, 0.0f, 0.0f};
	Font* font = nullptr;
    Mesh* textMesh = nullptr;
    
    TextRender();
    ~TextRender();

} TextRender;


namespace FontManager {
	static Font* InternalLoadFont(const char* path, Material* material, const float pointSize);
}

Font* CreateFont(const char* path, const char* alias, Material* material, const float pointSize);
static void DeleteFont(const char* alias);

void DereferenceFonts();
void DrawTextMesh(const TextRender* textRender, const Camera* camera, const double aspectRatio);

void SetFont(TextRender* textRender, const char* fontName, Font* defaultFont = nullptr);
void SetText(TextRender* textRender, const char* string, int x, int y, const float windowWidth, const float windowHeight, const float size);
