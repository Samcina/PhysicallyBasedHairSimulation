#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>
#include <hairsimulation/Math.h>
#include "Common.h"

namespace HairSimulation
{
    class HairRenderer
    {
    public:
        HairRenderer();
        HairRenderer(const HairRenderer&) = delete;
        void Render(const HairInstance* instance, const Matrix4& viewMatrix, const Matrix4& projectionMatrix) const;
        void Simulate(HairInstance* instance, float timeStep) const;
        ~HairRenderer();

    private:
        uint32_t hairBuffID;
        uint32_t sceneBuffID;
        uint32_t lightBuffID;
        uint32_t emptyVertexArrID;

        uint32_t hairSimulationID;
        uint32_t hairRenderID;
        uint32_t rootVisualizationID;
        uint32_t strandVisualizationID;

		Matrix4 CalculateWindVecs(const Vector3& wind, int frame) const;

        std::string shaderIncludeSrc;
    };
}

#endif