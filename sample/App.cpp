#include "App.h"
#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

App::App(int screenWidth, int screenHeight) :
    camera(nullptr),
    hairInstance(nullptr),
	hairSystem(nullptr),
	hairModel(nullptr),
	windMagnitude(0.0f)
{
    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("Cannot initialize GLFW.");
    }

    window = glfwCreateWindow(screenWidth, screenHeight, "Hair Simulation", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    
    hairSystem = new HairSimulation::HairSimulationSystem();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    hairConfig.renderStrands = false;
    hairConfig.renderRoot = false;
    hairConfig.modelMatrix.SetIdentity();
    hairConfig.tipWidth = 0.0005f;
    hairConfig.rootWidth = 0.002f;
    hairConfig.color = { 0.95f, 0.9f, 0.625f, 1.0f };
    hairConfig.density = 64.0f;
    hairConfig.tesselationFactor = 4.0f;
    hairConfig.friction = 0.05f;
    hairConfig.globalConstraint = 0.002f;
    hairConfig.localConstraint = 0.01f;

    hairModel = hairSystem->LoadModel("data/hair.hgl");
    hairInstance = hairSystem->CreateInstance(hairModel);

    camera = new CameraController(window);
}

void App::Run()
{
    glfwShowWindow(window);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        //update camera
        camera->Update();

        //display settings
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImVec2 configurationWindowPosition;
        configurationWindowPosition.x = 800;
        configurationWindowPosition.y = 10;

        ImVec2 configurationWindowSize;
        configurationWindowSize.x = 450;
        configurationWindowSize.y = 130;

        ImGui::SetNextWindowPos(configurationWindowPosition);
        ImGui::SetNextWindowSizeConstraints(configurationWindowSize, configurationWindowSize);
        ImGui::Begin("Configuration", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
        ImGui::SliderFloat("Hair Strand Density", &hairConfig.density, 16.0f, 64.0f);
        ImGui::SliderFloat("Wind Strength", &windMagnitude, 0.0f, 15.0f);
        ImGui::Checkbox("Show Initial Hair Strands", &hairConfig.renderStrands);
        ImGui::Checkbox("Show Root Triangles", &hairConfig.renderRoot);
        ImGui::End();
        ImGui::Render();

        //update wind vectors based on magnitude
        hairConfig.windVecs = HairSimulation::Vector3(1.0f, 0.0f, 0.0f).Normalized() * (windMagnitude * (pow(sin(glfwGetTime() * 20), 2) * 0.01 + 1));

        //update settings
        hairSystem->UpdateInstanceSettings(hairInstance, hairConfig);

        //simulate hair
        hairSystem->SimulateHair(hairInstance);

        //render hair after simulation
        int screenWidth, screenHeight;
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
        glViewport(0, 0, screenWidth, screenHeight);
        auto viewMatrix = camera->GetViewMatrix();
        auto projectionMatrix = HairSimulation::Matrix4::Perspective(HairSimulation::DegToRad * 100.0f, (float)screenWidth / (float)screenHeight, 0.01f, 100.0f);

        glClearColor(0.25f, 0.15f, 0.18f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        hairSystem->RenderHair(hairInstance, viewMatrix, projectionMatrix);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwWindowHint(GLFW_SAMPLES, 8);

        glfwSwapBuffers(window);
    }
}

App::~App()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    hairSystem->DestroyInstance(hairInstance);
    hairSystem->DestroyModel(hairModel);
    delete hairSystem;

    delete camera;

    glfwDestroyWindow(window);
    glfwTerminate();
}