#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <array>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <cstring>

#include "glUtilities.h"

double Time() {
    return internalInstanceInfo.time;
}

double DeltaTime() {
    return internalInstanceInfo.deltaTime;
}

double AspectRatio() {
    return internalInstanceInfo.aspectRatio;
}

int WindowHeight() {
    return internalInstanceInfo.WindowHeight;
}

int WindowWidth() {
    return internalInstanceInfo.WindowWidth;
}

void SetCaptureCursor(const bool captureCursor) {
    // Globally accessible function to tell GKFW what the cursor settings should be.
    // if captureCursor is set, the xPos and yPos from GetCursorPosition will act like a delta.
    internalInstanceInfo.captureCursor = captureCursor;
}

void GetCursorPositionDelta(double* xPos, double* yPos) {
    // Globally accessible function to get the cursor position.
    *xPos = internalInstanceInfo.xPosDelta / (double)internalInstanceInfo.WindowHeight;
    *yPos = internalInstanceInfo.yPosDelta / (double)internalInstanceInfo.WindowHeight;

    // This is not great but I cant think of a better way to make sure the delta get set back to zero once after camera gets updated. 
    internalInstanceInfo.xPosDelta = 0.0;
    internalInstanceInfo.yPosDelta = 0.0;
}

void GetCursorPosition(double* xPos, double* yPos) {
    // Globally accessible function to get the cursor position.
    *xPos = internalInstanceInfo.xPos / (double)internalInstanceInfo.WindowHeight;
    *yPos = internalInstanceInfo.yPos / (double)internalInstanceInfo.WindowHeight;
}

GLFWwindow* Initialize(const int width, const int height, const char* tittle) {
    /* Initialize a GLFW window. */
    
    assert(glfwInit() == GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    
    #ifdef NDEBUG
    #else
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    #endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(width, height, tittle, NULL, NULL);
    glfwMakeContextCurrent(window);
    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);

    #ifdef NDEBUG
    #else
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(glDebugOutput, nullptr);
    #endif
    
    glfwSetWindowSize(window, width, height);

    // Misc OpenGL settings.
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW); 

    return window;
}

void glUtilTerminate() {
    /* This function executes each function in the list of termination functions. */
    if (internalInstanceInfo.TerminationFunctions.empty()) {
        return;
    }

    // For each termination function added to the list, call it.
    for (TerminateFunction function : internalInstanceInfo.TerminationFunctions) {
        function();
    }
}

void glUtilAddTerminationFunction(TerminateFunction function) {
    internalInstanceInfo.TerminationFunctions.push_back(function);
}

void glUtilInitializeFrame(GLFWwindow* window){

    internalInstanceInfo.time = glfwGetTime();
    internalInstanceInfo.deltaTime = internalInstanceInfo.time - internalInstanceInfo.lastTime;

    // Resize the view to match the window.
    glfwGetFramebufferSize(window, &(internalInstanceInfo.WindowWidth), &(internalInstanceInfo.WindowHeight));
    internalInstanceInfo.aspectRatio = (double)internalInstanceInfo.WindowWidth / (double)internalInstanceInfo.WindowHeight;
    glViewport(0, 0, internalInstanceInfo.WindowWidth, internalInstanceInfo.WindowHeight);
    
    // Clear the screen buffer.
    glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
}

static void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
    /* Get the mouse position from the window.*/

    // truncate it down to an int to make handling it easier.
    internalInstanceInfo.xPosDelta = internalInstanceInfo.xPos - xPos;
    internalInstanceInfo.yPosDelta = internalInstanceInfo.yPos - yPos;
    internalInstanceInfo.xPos = xPos;
    internalInstanceInfo.yPos = yPos;

    // If the mouse should be captured, set it as such.
    if(internalInstanceInfo.captureCursor) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    // otherwise set the cursor back to normal.
    else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    /* Update the keyboard input lists from GLFW. */

    // Leave early if the key is repeated, to "disable" repeat events so that keys are either up or down
    if (action == GLFW_REPEAT) {
        return;
    }

    // Leave early if the exit key is pressed.
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }
    
    internalInstanceInfo.gKeysCurr[key] = action;

}

void glUtilPollEvents() {
    /* Handle keyboard polling, update the internal timer, and call glfwPollEvents. */
    
    memcpy(internalInstanceInfo.gKeysPrev.data(), internalInstanceInfo.gKeysCurr.data(), GLFW_KEY_LAST * sizeof(int));
    glfwPollEvents();
    
    internalInstanceInfo.lastTime = internalInstanceInfo.time;
}

bool IsKeyDown(int key) {
    return internalInstanceInfo.gKeysCurr[key] == GLFW_PRESS;
}

bool IsKeyUp(int key) {
    return internalInstanceInfo.gKeysCurr[key] == GLFW_RELEASE;
}

bool IsKeyPressed(int key) {
    return internalInstanceInfo.gKeysPrev[key] == GLFW_PRESS && internalInstanceInfo.gKeysCurr[key] == GLFW_RELEASE;
}

// Graphics debug callback
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}




