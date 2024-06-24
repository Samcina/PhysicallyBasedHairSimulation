#include "Renderer.h"
#include "gl/GLUtils.h"
#include <vector>
#include <algorithm>
#include <hairsimulation/Math.h>
#include "shaders/ShaderTypes.h"

namespace HairSimulation
{
    const std::string GLSLVersion = "#version 430 core\n";

    HairRenderer::HairRenderer() :
        strandVisualizationID(0),
        rootVisualizationID(0),
        hairSimulationID(0),
        hairRenderID(0),
        emptyVertexArrID(0)
    {
        glGenVertexArrays(1, &emptyVertexArrID);

        glGenBuffers(1, &lightBuffID);
        glBindBuffer(GL_UNIFORM_BUFFER, lightBuffID);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(LightRenderData), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glGenBuffers(1, &hairBuffID);
        glBindBuffer(GL_UNIFORM_BUFFER, hairBuffID);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(HairRenderData), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glGenBuffers(1, &sceneBuffID);
        glBindBuffer(GL_UNIFORM_BUFFER, sceneBuffID);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneRenderData), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);


        shaderIncludeSrc = LoadFile("HairSimulationshaders/ShaderTypes.h");

        auto strandVisualizationVertShaderSource = LoadFile("HairSimulationshaders/StrandVisualization.vert");
        auto strandVisualizationFragShaderSource = LoadFile("HairSimulationshaders/SimpleColor.frag");
        uint32_t strandVisualizationVertShaderID = CompileShader(GLSLVersion, strandVisualizationVertShaderSource, GL_VERTEX_SHADER);
        uint32_t strandVisualizationFragShaderID = CompileShader(GLSLVersion, strandVisualizationFragShaderSource, GL_FRAGMENT_SHADER);
        strandVisualizationID = LinkProgram(strandVisualizationVertShaderID, strandVisualizationFragShaderID);

        auto rootVisualizationVertShaderSource = LoadFile("HairSimulationshaders/RootVisualization.vert");
        auto rootVisualizationFragShaderSource = LoadFile("HairSimulationshaders/SimpleColor.frag");
        uint32_t rootVisualizationVertShaderID = CompileShader(GLSLVersion, rootVisualizationVertShaderSource, GL_VERTEX_SHADER);
        uint32_t rootVisualizationFragShaderID = CompileShader(GLSLVersion, rootVisualizationFragShaderSource, GL_FRAGMENT_SHADER);
        rootVisualizationID = LinkProgram(rootVisualizationVertShaderID, rootVisualizationFragShaderID);

        auto simulationShaderSource = LoadFile("HairSimulationshaders/HairSimulation.comp");
        uint32_t simulationShaderID = CompileShader(GLSLVersion, simulationShaderSource, GL_COMPUTE_SHADER, &shaderIncludeSrc);
        hairSimulationID = LinkProgram(simulationShaderID);

        auto hairSimulationVertShaderSource = LoadFile("HairSimulationshaders/HairSimulation.vert");
        auto hairSimulationTessControlShaderSource = LoadFile("HairSimulationshaders/HairSimulation.tesc");
        auto hairSimulationTessEvaluationShaderSource = LoadFile("HairSimulationshaders/HairSimulation.tese");
        auto hairGeomShaderSource = LoadFile("HairSimulationshaders/HairSimulation.geom");
        auto hairSimulationFragShaderSource = LoadFile("HairSimulationshaders/HairSimulation.frag");

        uint32_t hairSimulationVertShaderID = CompileShader(GLSLVersion, hairSimulationVertShaderSource, GL_VERTEX_SHADER, &shaderIncludeSrc);
        uint32_t hairSimulationTessControlShaderID = CompileShader(GLSLVersion, hairSimulationTessControlShaderSource, GL_TESS_CONTROL_SHADER, &shaderIncludeSrc);
        uint32_t hairSimulationTessEvaluationShaderID = CompileShader(GLSLVersion, hairSimulationTessEvaluationShaderSource, GL_TESS_EVALUATION_SHADER, &shaderIncludeSrc);
        uint32_t hairSimulationGeomShaderID = CompileShader(GLSLVersion, hairGeomShaderSource, GL_GEOMETRY_SHADER, &shaderIncludeSrc);
        uint32_t hairSimulationFragShaderID = CompileShader(GLSLVersion, hairSimulationFragShaderSource, GL_FRAGMENT_SHADER, &shaderIncludeSrc);

        hairRenderID = LinkProgram(hairSimulationVertShaderID, hairSimulationTessControlShaderID, hairSimulationTessEvaluationShaderID, hairSimulationGeomShaderID, hairSimulationFragShaderID);

        glDeleteShader(hairSimulationVertShaderID);
        glDeleteShader(hairSimulationTessControlShaderID);
        glDeleteShader(hairSimulationTessEvaluationShaderID);
        glDeleteShader(hairSimulationGeomShaderID);
        glDeleteShader(hairSimulationFragShaderID);
    }

    void HairRenderer::Render(const HairInstance* instance, const Matrix4& viewMatrix, const Matrix4& projectionMatrix) const
    {
        auto asset = instance->model;
        auto settings = instance->config;
        auto viewProjectionMatrix = projectionMatrix * viewMatrix;
        int verticesPerStrand = asset->segCount + 1;

        glEnable(GL_DEPTH_TEST);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, REST_POSITIONS_BUFFER_BINDING, asset->restBuffID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, POSITIONS_BUFFER_BINDING, instance->posBuffID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PREVIOUS_POSITIONS_BUFFER_BINDING, instance->prevPosBuffID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, HAIR_INDICES_BUFFER_BINDING, asset->hairIndicesBuffID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TANGENTS_DISTANCES_BINDING, asset->tangentsBuffID);

        if (settings.renderStrands) {
            glUseProgram(strandVisualizationID);

            glUniformMatrix4fv(glGetUniformLocation(strandVisualizationID, "viewProjectionMatrix"), 1, false, (float*)viewProjectionMatrix.m);
            glUniform1i(glGetUniformLocation(strandVisualizationID, "doubleSegments"), asset->segCount * 2);
            glUniform1i(glGetUniformLocation(strandVisualizationID, "verticesPerStrand"), verticesPerStrand);
            glUniform4f(glGetUniformLocation(strandVisualizationID, "color"), 0, 1, 0, 1);

            glBindVertexArray(emptyVertexArrID);
            glDrawArrays(GL_LINES, 0, asset->strandCount * asset->segCount * 2);
            glUseProgram(0);
        }

        if (instance->config.renderRoot) {
            glUseProgram(rootVisualizationID);

            glUniformMatrix4fv(glGetUniformLocation(rootVisualizationID, "viewProjectionMatrix"), 1, false, (float*)viewProjectionMatrix.m);
            glUniform1i(glGetUniformLocation(rootVisualizationID, "verticesPerStrand"), verticesPerStrand);
            glUniform4f(glGetUniformLocation(rootVisualizationID, "color"), 0, 1, 0.8, 1);

            glBindVertexArray(emptyVertexArrID);
            glDrawArrays(GL_LINES, 0, asset->trianglesCount * 6);
            glUseProgram(0);
        }

        if (instance->config.renderHair) {
            auto inversedViewMatrix = viewMatrix.EuclidianInversed();

            HairRenderData hairRenderData = {};
            hairRenderData.tesselationFactor = settings.tesselationFactor;
            hairRenderData.segmentsCount = instance->model->segCount;
            hairRenderData.rootWidth = settings.rootWidth;
            hairRenderData.tipWidth = settings.tipWidth;
            hairRenderData.density = settings.density;
            hairRenderData.color = settings.color;
            hairRenderData.ambient = settings.ambientStrength;
            hairRenderData.diffuse = settings.diffuseStrength;
            hairRenderData.specular = settings.specularStrength;
            hairRenderData.specularPower = settings.specularPow;
            hairRenderData.thinningStart = settings.thinningStart;

            SceneRenderData sceneRenderData = {};
            sceneRenderData.viewProjectionMatrix = viewProjectionMatrix;
            sceneRenderData.eyePosition = inversedViewMatrix.m[3].XYZ();

            LightRenderData lightData = {};
            lightData.lightsCount = 1;
            lightData.lights[0].position = { 5, 5, 5 };
            lightData.lights[0].color = { 1, 1, 1, 1 };

            glBindBuffer(GL_UNIFORM_BUFFER, hairBuffID);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(HairRenderData), &hairRenderData, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glBindBuffer(GL_UNIFORM_BUFFER, sceneBuffID);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneRenderData), &sceneRenderData, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glBindBuffer(GL_UNIFORM_BUFFER, lightBuffID);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(LightRenderData), &lightData, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, POSITIONS_BUFFER_BINDING, instance->posBuffID);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, HAIR_INDICES_BUFFER_BINDING, asset->hairIndicesBuffID);

            glUseProgram(hairRenderID);

            glBindBufferRange(GL_UNIFORM_BUFFER, HAIR_DATA_BINDING, hairBuffID, 0, sizeof(HairRenderData));
            glBindBufferRange(GL_UNIFORM_BUFFER, SCENE_DATA_BINDING, sceneBuffID, 0, sizeof(SceneRenderData));
            glBindBufferRange(GL_UNIFORM_BUFFER, LIGHT_DATA_BINDING, lightBuffID, 0, sizeof(LightRenderData));

            glPatchParameteri(GL_PATCH_VERTICES, 1);
            glDrawArrays(GL_PATCHES, 0, asset->trianglesCount * asset->segCount);
            glUseProgram(0);
        }
    }

    void HairRenderer::Simulate(HairInstance* instance, float timeStep) const
    {
        auto model = instance->model;

        glUseProgram(hairSimulationID);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, REF_VECTORS_BINDING, model->refVecsBufferID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, POSITIONS_BUFFER_BINDING, instance->posBuffID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PREVIOUS_POSITIONS_BUFFER_BINDING, instance->prevPosBuffID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, REST_POSITIONS_BUFFER_BINDING, model->restBuffID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TANGENTS_DISTANCES_BINDING, model->tangentsBuffID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GLOBAL_ROTATIONS_BINDING, model->globalRotBuffID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, DEBUG_BUFFER_BINDING, model->debugBuffID);

        

        glUniform3f(glGetUniformLocation(hairSimulationID, "gravityForce"), 0.0f, -9.8f, 0.0f);
        glUniform1f(glGetUniformLocation(hairSimulationID, "friction"), instance->config.friction);
        glUniform1i(glGetUniformLocation(hairSimulationID, "lenConstraintIter"), 5);
        glUniform1i(glGetUniformLocation(hairSimulationID, "localConstraintIter"), 10);
        glUniform1f(glGetUniformLocation(hairSimulationID, "localConstraint"), (std::min)(instance->config.localConstraint, 0.95f) * 0.5f);
        glUniform1f(glGetUniformLocation(hairSimulationID, "globalConstraint"), instance->config.globalConstraint);

        int verticesPerStrand = model->segCount + 1;
        glUniform1i(glGetUniformLocation(hairSimulationID, "verticesPerStrand"), verticesPerStrand);

        glUniform1f(glGetUniformLocation(hairSimulationID, "timeStep"), timeStep);

        
        auto windVecs = CalculateWindVecs(instance->config.windVecs, instance->frame);

		glUniformMatrix4fv(glGetUniformLocation(hairSimulationID, "windVecs"), 1, false, (float*)windVecs.m);

        glUniformMatrix4fv(glGetUniformLocation(hairSimulationID, "modelMatrix"), 1, false, (float*)instance->config.modelMatrix.m);

        glDispatchCompute(instance->model->strandCount, 1, 1);
        glUseProgram(0);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		instance->frame++;
    }

	Vector4 GetWindVecCorner(const Quaternion& rotationFromXToWind, const Vector3& axis, float angle, float magnitude)
	{
		Vector3 xAxis(1.0f, 0.0f, 0.0f);
		Quaternion rotation(axis, angle);
		auto side = rotationFromXToWind * rotation * xAxis * magnitude;
		return Vector4(side.x, side.y, side.z, 0.0f);
	}

	Matrix4 HairRenderer::CalculateWindVecs(const Vector3& wind, int frame) const
	{
		float magnitude = wind.Length();
		auto dir = wind / magnitude;

		Vector3 axisX(1.0f, 0.0f, 0.0f);
		auto rotAxis = Vector3::Cross(axisX, dir);
		float angle = asin(rotAxis.Length());

		Quaternion rotationFromXToWind;
		if (angle > 0.001)
		{
			rotationFromXToWind = Quaternion(rotAxis.Normalized(), angle);
		}

		float coneAngle = 20.0f * DegToRad;

		Matrix4 windVecs;
		windVecs.m[0] = GetWindVecCorner(rotationFromXToWind, Vector3(0, 1, 0), coneAngle, magnitude);
		windVecs.m[1] = GetWindVecCorner(rotationFromXToWind, Vector3(0, -1, 0), coneAngle, magnitude);
		windVecs.m[2] = GetWindVecCorner(rotationFromXToWind, Vector3(0, 0, 1), coneAngle, magnitude);
		windVecs.m[3] = GetWindVecCorner(rotationFromXToWind, Vector3(0, 0, -1), coneAngle, magnitude);

		return windVecs;
	}

    HairRenderer::~HairRenderer()
    {
        glFinish();

        glDeleteProgram(strandVisualizationID);
        glDeleteProgram(hairRenderID);
        glDeleteProgram(hairSimulationID);
        glDeleteVertexArrays(1, &emptyVertexArrID);
    }
}