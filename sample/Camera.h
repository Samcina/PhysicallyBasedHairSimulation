#ifndef CAMERA_H
#define CAMERA_H

#include <hairsimulation/Math.h>
#include <GLFW/glfw3.h>

class CameraController
{
public:
    CameraController(GLFWwindow* window);

    HairSimulation::Matrix4 GetViewMatrix() const;

    void SetYAxis(float pitch);
    void SetZAxis(float yaw);
    void Update();

    ~CameraController();

private:
    GLFWwindow* window;
    float distance, pitch, yaw, rotationSpeed, zoomSpeed;
    HairSimulation::Vector3 target;
    HairSimulation::Vector3 previousPosition;
    bool isDragging;

    void OnDrag(const HairSimulation::Vector3& offset);
    void OnScroll(double xoffset, double yoffset);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};

#endif