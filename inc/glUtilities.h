#pragma once

#include <array>
#include <vector>


// Forward Declarations:
struct GLFWwindow;

typedef void (*TerminateFunction)();

struct InstanceInformation {
    /* Struct to hold data relevant to the state of the instance. */ 
    
    // Frame delta:
    double time = 0.0;
    double lastTime = 0.0;
    double deltaTime = 0.0;

    // Window Data:
    int WindowWidth = 0;
    int WindowHeight = 0;
    double aspectRatio = 0;
    
    // Cursor Data:
    bool captureCursor = false;
    double xPosDelta = 0.0;
    double yPosDelta = 0.0;
    double xPos = 0.0;
    double yPos = 0.0;
    double lastxPos = 0.0;
    double lastyPos = 0.0;
    
    // Internal lists:
    std::array<int, GLFW_KEY_LAST> gKeysPrev{};
    std::array<int, GLFW_KEY_LAST> gKeysCurr{};
    std::vector<TerminateFunction> TerminationFunctions;
};

static InstanceInformation internalInstanceInfo;

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);
GLFWwindow* Initialize(const int width, const int height, const char* tittle);

void glUtilTerminate();
void glUtilAddTerminationFunction(TerminateFunction function);

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouse_callback(GLFWwindow* window, double xPos, double yPos);

void glUtilInitializeFrame(GLFWwindow* window);
void glUtilPollEvents();

void SetCaptureCursor(const bool captureCursor);
void GetCursorPositionDelta(double* xPos, double* yPos);
void GetCursorPosition(double* xPos, double* yPos);

double Time();
double DeltaTime();
double AspectRatio();
int WindowHeight();
int WindowWidth();

bool IsKeyPressed(int key);
bool IsKeyDown(int key);
bool IsKeyUp(int key);
