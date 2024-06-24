#include "Camera.h"
#include <cmath>

CameraController::CameraController(GLFWwindow* window) :
    window(window),
    target(HairSimulation::Vector3()),
    distance(0.5f),
    pitch(0),
    yaw(0),
    zoomSpeed(0.05f),
    rotationSpeed(0.005f),
    isDragging(false),
    previousPosition(HairSimulation::Vector3())
{
    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, ScrollCallback);
}

HairSimulation::Matrix4 CameraController::GetViewMatrix() const
{
    HairSimulation::Vector4 offset(0, 0, distance, 1.0f);
    auto verticalRotation = HairSimulation::Matrix4::RotateX(pitch);
    auto horizontalRotation = HairSimulation::Matrix4::RotateY(yaw);
    offset = verticalRotation * HairSimulation::Vector4(offset.x, offset.y, offset.z, 1);
    offset = horizontalRotation * HairSimulation::Vector4(offset.x, offset.y, offset.z, 1);
    auto position = target + offset.XYZ();

    auto viewMatrix = HairSimulation::Matrix4::LookAt(position, target, HairSimulation::Vector3(0, 1, 0));
    return viewMatrix;
}

void CameraController::Update()
{
    distance = std::fmax(distance, 0.01f);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) != GLFW_RELEASE) {
        double currentMousePositionX;
        double currentMousePositionY;
        glfwGetCursorPos(window, &currentMousePositionX, &currentMousePositionY);
        HairSimulation::Vector3 currentMousePosition(currentMousePositionX, currentMousePositionY, 0);

        if (!isDragging) {
            isDragging = true;
            previousPosition = currentMousePosition;
        }
        
        OnDrag(currentMousePosition - previousPosition);
        previousPosition = currentMousePosition;
    }
    else {
        isDragging = false;
    }
}

void CameraController::SetYAxis(float pitch)
{
    if (pitch >= HairSimulation::PI / 2) {
        pitch = HairSimulation::PI / 2 - 0.00001f;
    }
    if (pitch <= -HairSimulation::PI / 2) {
        pitch = -HairSimulation::PI / 2 + 0.00001f;
    }
    this->pitch = pitch;
}

void CameraController::SetZAxis(float yaw)
{
    if (yaw > HairSimulation::PI * 2) {
        yaw = 0;
    }
    this->yaw = yaw;
}

void CameraController::OnScroll(double xoffset, double yoffset)
{
    distance = distance - yoffset * zoomSpeed;
}

void CameraController::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto controller = static_cast<CameraController*>(glfwGetWindowUserPointer(window));
    controller->OnScroll(xoffset, yoffset);
}

void CameraController::OnDrag(const HairSimulation::Vector3& offset)
{
    SetYAxis(pitch + offset.y * rotationSpeed);
    SetZAxis(yaw + offset.x * rotationSpeed);
}

CameraController::~CameraController()
{
    glfwSetWindowUserPointer(window, nullptr);
    glfwSetScrollCallback(window, nullptr);
}
