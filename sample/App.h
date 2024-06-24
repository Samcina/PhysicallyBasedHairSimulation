#ifndef APP_H
#define APP_H

#include <GLFW/glfw3.h>
#include <hairsimulation/HairSimulation.h>
#include "Camera.h"

class App
{
public:
    App(int screenWidth, int screenHeight);
    void Run();
    ~App();

private:
    HairSimulation::HairInstance* hairInstance;
    HairSimulation::HairModel* hairModel;
    HairSimulation::HairSimulationSystem* hairSystem;
    HairSimulation::HairConfig hairConfig;
    CameraController* camera;
	float windMagnitude;
    GLFWwindow* window;
};

#endif