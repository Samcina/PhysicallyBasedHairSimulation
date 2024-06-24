#include <hairsimulation/HairSimulation.h>
#include <stdexcept>
#include <vector>
#include "gl/GLUtils.h"
#include "Renderer.h"

namespace HairSimulation
{
    HairSimulationSystem::HairSimulationSystem() :
        hairRenderer(nullptr)
    {
        if (!InitGL()) {
            throw std::runtime_error("Cannot initialize OpenGL resources.");
        }

        hairRenderer = new HairRenderer();
    }


    void UpdateConstraintsBuffers(const std::vector<Vector4>& vertices, int segmentsPerStrand, std::vector<Vector4>& tangents)
    {
        tangents.resize(vertices.size());

        for (int guideIndex = 0; guideIndex < vertices.size() / segmentsPerStrand; guideIndex++) {

            for (int i = 0; i < segmentsPerStrand - 1; i++) {

                auto p0 = vertices[guideIndex * segmentsPerStrand + i];
                auto p1 = vertices[guideIndex * segmentsPerStrand + i + 1];
                tangents[guideIndex * segmentsPerStrand + i].w = (p1.XYZ() - p0.XYZ()).Length();

            }
        }
    }

    void UpdateRotationBuffers(const std::vector<Vector4>& vertices, int segmentsPerStrand, std::vector<Quaternion>& globalRotations, std::vector<Vector4>& refVectors)
    {
        globalRotations.resize(vertices.size());
        refVectors.resize(vertices.size());
        std::vector<Quaternion> localRotations(vertices.size());


        for (int strandIndex = 0; strandIndex < vertices.size() / segmentsPerStrand; strandIndex++) {
            int rootIndex = strandIndex * segmentsPerStrand;

            auto position = vertices[rootIndex].XYZ();
            auto positionNext = vertices[rootIndex + 1].XYZ();
            auto tangent = positionNext - position;
            auto tangentX = tangent.Normalized();
            auto tangentZ = Vector3::Cross(tangentX, Vector3(1.0f, 0, 0));

            if (tangentZ.Length() < 0.0001f) {
                tangentZ = Vector3::Cross(tangentX, Vector3(0, 1.0f, 0));
            }

            tangentZ.Normalize();
            auto tangentY = Vector3::Cross(tangentZ, tangentX);

            Matrix3 rotationMatrix;
            rotationMatrix.m[0][0] = tangentX[0];
            rotationMatrix.m[0][1] = tangentY[0];
            rotationMatrix.m[0][2] = tangentZ[0];
            rotationMatrix.m[1][0] = tangentX[1];
            rotationMatrix.m[1][1] = tangentY[1];
            rotationMatrix.m[1][2] = tangentZ[1];
            rotationMatrix.m[2][0] = tangentX[2];
            rotationMatrix.m[2][1] = tangentY[2];
            rotationMatrix.m[2][2] = tangentZ[2];

            globalRotations[rootIndex] = localRotations[rootIndex] = Quaternion::FromMatrix(rotationMatrix);

            for (int i = 1; i < segmentsPerStrand; i++) {
                auto positionPrev = vertices[rootIndex + i - 1].XYZ();
                position = vertices[rootIndex + i].XYZ();
                tangent = position - positionPrev;
                auto tangentLocal = globalRotations[rootIndex + i - 1].Inversed() * tangent;

                tangentX = tangentLocal.Normalized();
                Vector3 axisX(1.0f, 0, 0);
                auto rotationAxis = Vector3::Cross(axisX, tangentX);
                float angle = acos(Vector3::Dot(axisX, tangentX));

                if (abs(angle) > 0.001 && rotationAxis.Length2() > 0.001) {
                    rotationAxis.Normalize();
                    localRotations[rootIndex + i] = Quaternion(rotationAxis, angle);
                }
                else {
                    localRotations[rootIndex + i] = Quaternion();
                }

                globalRotations[rootIndex + i] = globalRotations[rootIndex + i - 1] * localRotations[rootIndex + i];
                refVectors[rootIndex + i] = Vector4(tangentLocal.x, tangentLocal.y, tangentLocal.z, 0.0f);
            }
        }
    }

    void HairSimulationSystem::RenderHair(const HairInstance* instance, const Matrix4& viewMatrix, const Matrix4& projectionMatrix) const
    {
        hairRenderer->Render(instance, viewMatrix, projectionMatrix);
    }

    void HairSimulationSystem::SimulateHair(HairInstance* instance, float timeStep) const
    {
        hairRenderer->Simulate(instance, timeStep);
    }


    HairModel* HairSimulationSystem::LoadModel(const char* path) const
    {
        auto file = fopen(path, "rb");
        if (file == nullptr) {
            throw std::runtime_error(std::string("Cannot open file ") + path);
        }

        fseek(file, 0, SEEK_END);
        int size = ftell(file);
        fseek(file, 0, SEEK_SET);

        int strandCount = 0;
        int segmentsCount = 0;
        int trianglesCount = 0;

        fread(&strandCount, sizeof(strandCount), 1, file);
        fread(&segmentsCount, sizeof(segmentsCount), 1, file);
        fread(&trianglesCount, sizeof(trianglesCount), 1, file);

        int verticesPerStrand = segmentsCount + 1;
        std::vector<Vector4> vertices(strandCount * verticesPerStrand);
        for (int i = 0; i < vertices.size(); i++) {
            if (feof(file)) {
                throw std::runtime_error(std::string("Invalid hair asset file ") + path);
            }

            fread(&vertices[i], sizeof(float), 3, file);
            vertices[i].w = i % verticesPerStrand == 0 ? 0 : 1;
        }

        std::vector<int> triangles(trianglesCount * 4, 0);
        for (int i = 0; i < trianglesCount; i++) {
            if (feof(file)) {
                throw std::runtime_error(std::string("Invalid hair asset file ") + path);
            }

            fread(&triangles[i * 4], sizeof(int), 3, file);
        }

        fclose(file);

        std::vector<Vector4> tangents;
        UpdateConstraintsBuffers(vertices, verticesPerStrand, tangents);

		std::vector<Vector4> refVecs;
		std::vector<Quaternion> globalRotations;
		UpdateRotationBuffers(vertices, verticesPerStrand, globalRotations, refVecs);

        auto model = new HairModel();
        model->strandCount = strandCount;
        model->segCount = segmentsCount;
        model->trianglesCount = trianglesCount;

        glGenBuffers(1, &model->tangentsBuffID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, model->tangentsBuffID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, tangents.size() * sizeof(Vector4), tangents.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &model->refVecsBufferID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, model->refVecsBufferID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, refVecs.size() * sizeof(Vector4), refVecs.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &model->debugBuffID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, model->debugBuffID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, vertices.size() * sizeof(Vector4), nullptr, GL_STATIC_DRAW);

        glGenBuffers(1, &model->restBuffID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, model->restBuffID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, vertices.size() * sizeof(Vector4), vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &model->globalRotBuffID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, model->globalRotBuffID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, globalRotations.size() * sizeof(Quaternion), globalRotations.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &model->hairIndicesBuffID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, model->hairIndicesBuffID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, triangles.size() * sizeof(int), triangles.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        return model;
    }

    void HairSimulationSystem::DestroyModel(HairModel* model) const
    {
        glDeleteBuffers(1, &model->restBuffID);
        delete model;
    }

    void CopyBuffer(uint32_t src, uint32_t dst, int size)
    {
        glBindBuffer(GL_COPY_READ_BUFFER, src);
        glBindBuffer(GL_COPY_WRITE_BUFFER, dst);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);
    }

    HairInstance* HairSimulationSystem::CreateInstance(const HairModel* model) const
    {
        auto instance = new HairInstance();
        instance->model = model;

        size_t positionsSize = sizeof(Vector4) * model->strandCount * (model->segCount + 1);

        glGenBuffers(1, &instance->posBuffID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, instance->posBuffID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, positionsSize, nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &instance->prevPosBuffID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, instance->prevPosBuffID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, positionsSize, nullptr, GL_DYNAMIC_DRAW);

        CopyBuffer(model->restBuffID, instance->posBuffID, positionsSize);
        CopyBuffer(model->restBuffID, instance->prevPosBuffID, positionsSize);

        return instance;
    }

    void HairSimulationSystem::UpdateInstanceSettings(HairInstance* instance, const HairConfig& config) const
    {
        instance->config = config;
    }

    void HairSimulationSystem::DestroyInstance(HairInstance* instance) const
    {
        glDeleteBuffers(1, &instance->posBuffID);
        delete instance;
    }

    HairSimulationSystem::~HairSimulationSystem()
    {
        delete hairRenderer;
    }
}