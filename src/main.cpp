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
#include "gl_shader_uniform.h"

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;


int main(void) {
    
    // Initialize the window to the starting size and set the header.
    GLFWwindow* window = Initialize(SCREEN_WIDTH, SCREEN_HEIGHT, "Delta Render");
    InitShaders();
    
    // Add termination functions to be executed at the end of the program.
    glUtilAddTerminationFunction(DereferenceFonts);
    glUtilAddTerminationFunction(DereferenceTextures);
    glUtilAddTerminationFunction(DereferenceShaders);
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
    Font* defautFont = CreateFont("./assets/defaultAssets/IBMPlexMono-Regular.ttf", "IBM", DefaultTextMaterial, 22.0f);
    
    // There is a known issue with fonts right now. Something is getting deleted when it isn't supposed to. Will run fine on a first pass.  
    TextRender* testText = new TextRender();
    SetFont(testText, "IBM", defautFont);


    //StaticMesh* mesh = CreateStaticMeshPrimativePlane(1, 1);
    StaticMesh* mesh = CreateStaticMeshFromWavefront("./assets/meshes/head.obj");
    mesh->SetMaterial(Mat0, 0);
    Matrix* transform = GET_ASSET_TRANSFORM(mesh);

    Camera* mainCamera = new Camera(NoClipCameraUpdate);
    
    Shader* testShader = Shader_create(DefaultTextMaterial->Program, "TestShader");
    //Uniform* mvpUniform;
    //Uniform_set_data(mvpUniform, transform);
    
    int x = 0;
    int y = 0;
    GLfloat time = 0.0f;

    void* testData = calloc(16, 64);

    GLuint activeLights = 0;

   // UniformBuffer_set_Global("LightData", "u_activeLights", &activeLights);
    
    //Matrix* data = internal_Uniform_get_data(Matrix, mvpUniform);
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        
        // FRAME STARTS HERE
        glUtilInitializeFrame(window);
        time = (GLfloat)Time();

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
        *transform = Translate(static_cast<float>(x), static_cast<float>(y), 0.0f);

        mainCamera->Update(mainCamera, DeltaTime(), AspectRatio());
        
        float16 cameraView = ToFloat16(mainCamera->ViewMatrix);

        std::cout << time << std::endl;

        UniformBuffer* buffer = UniformBuffer_get_self("FrameData");
        void* data = UniformBuffer_get_shared(buffer);
        //UniformBuffer* buffer1 = UniformBuffer_get_self("LightData");

        //UniformStruct* u_lights;
        //UniformBuffer_get_Struct(buffer1, "u_lights", &u_lights);

        UniformBuffer_set_Global("FrameData", "u_time", &time);
        UniformBuffer_set_Global("FrameData", "u_camera", &cameraView.v);

        //UniformBuffer_set_Struct_at_Global("LightData", "u_lights", "position", 0, testData);
        UniformBuffer_update_all();

        //Shader_use(testShader);

        mesh->Draw();
       
        SetText(testText,"This is a test.", x, y, static_cast<float>(WindowWidth()), static_cast<float>(WindowHeight()), 2.0f);
        DrawTextMesh(testText, mainCamera, AspectRatio());

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll and process events */
        glUtilPollEvents();
        //Shader_debug(Mat0->Program);
    }

    delete mainCamera;
    delete mesh;
    delete DefaultTextMaterial;
    delete NormalMaterial;
    delete TChoodColorMaterial;
    delete Mat0;
    delete testText;

    free(testData);

    Shader_destroy(&testShader);
    glUtilTerminate();
    return 0;
}

