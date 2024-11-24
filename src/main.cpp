#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>

#include "glUtilities.h"
#include "vectorMath.h"
#include "texture.h"
#include "material.h"
#include "camera.h"
#include "mesh.h"
#include "font.h"
#include "shaderUniform.h"

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;


int main(void) {
    
    // Initialize the window to the starting size and set the header.
    GLFWwindow* window = Initialize(SCREEN_WIDTH, SCREEN_HEIGHT, "Delta Render");
    
    // Add termination functions to be executed at the end of the program.
    glUtilAddTerminationFunction(DereferenceFonts);
    glUtilAddTerminationFunction(DereferenceTextures);
    glUtilAddTerminationFunction(glfwTerminate);
    
    // Load Textures:
    CreateTexture("./assets/defaultAssets/missingTexture.png", "MissingTexture", GL_RGBA, GL_RGBA, false, false, false, GL_LINEAR);
    
    // Load Materials:
    Material* DefaultTextMaterial = new Material("./assets/shaders/defaultText.vert", "./assets/shaders/defaultText.frag", 1, GL_BACK, GL_ALWAYS);
    Material* NormalMaterial = new Material("./assets/shaders/default.vert", "./assets/shaders/normal_color.frag", 0, GL_BACK, GL_LESS);
    Material* TChoodColorMaterial = new Material("./assets/shaders/default.vert", "./assets/shaders/tcoord_color.frag", 0, GL_BACK, GL_LESS);
    Material* Mat0 = new Material("./assets/shaders/default.vert", "./assets/shaders/ditheredAlpha.frag", 1, GL_BACK, GL_LESS);
    
    // Set Material Textures:
    SetTextureFromAlias(Mat0, "MissingTexture", 0);
    
    // Load Font:
    Font* departureMono = CreateFont("./assets/defaultAssets/DepartureMono-Regular.ttf", "DepartureMono", DefaultTextMaterial, 14.0f);
    
    
    // There is a known issue with fonts right now. Something is getting deleted when it isn't supposed to. Will run fine on a first pass.  
    TextRender* testText = new TextRender();
    SetFont(testText, "DepartureMono", departureMono);


    StaticMesh* mesh = CreateStaticMeshPrimativePlane(1, 1);
    mesh->SetMaterial(Mat0, 0);
    Matrix* transform = GET_ASSET_TRANSFORM(mesh);
    
    *transform = *transform * Translate(0.0f, 0.0f, -1.0f);

    Camera* mainCamera = new Camera(NoClipCameraUpdate);

    //Shader* testShader = CreateShader(Mat0->Program, "TestShader");
    //FreeShader(&testShader);

    int x = 0;
    int y = 0;
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        
        // FRAME STARTS HERE
        glUtilInitializeFrame(window);

        if (IsKeyPressed(GLFW_KEY_UP)) {
            y++;
        }
        
        if (IsKeyPressed(GLFW_KEY_DOWN)) {
            y--;
        }
        
        if (IsKeyPressed(GLFW_KEY_RIGHT)) {
            x++;
        }
        
        if (IsKeyPressed(GLFW_KEY_LEFT)) {
            x--;
        }

        mainCamera->Update(mainCamera, DeltaTime(), AspectRatio());
     
        mesh->Draw(mainCamera, (GLfloat)Time());
       
        SetText(testText,"This is a test.", x, y, static_cast<float>(WindowWidth()), static_cast<float>(WindowHeight()), 1.0f);
        DrawTextMesh(testText, mainCamera, AspectRatio(), (GLfloat)Time());
        

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll and process events */
        glUtilPollEvents();
        
    }

    delete mainCamera;

    delete mesh;

    delete DefaultTextMaterial;
    delete NormalMaterial;
    delete TChoodColorMaterial;
    delete Mat0;

    delete testText;

    glUtilTerminate();
    return 0;
}

