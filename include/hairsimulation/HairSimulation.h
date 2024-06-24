#ifndef HAIRSIMULATION_H
#define HAIRSIMULATION_H


#include "Math.h"
#include "HairTypes.h"
#include <stdint.h>

namespace HairSimulation
{
    class HairRenderer;
    class HairModel;
    class HairInstance;

    class HairSimulationSystem
    {
    public:
        HairSimulationSystem();
        HairSimulationSystem(const HairSimulationSystem&) = delete;
        HairModel* LoadModel(const char* path) const;
        void DestroyModel(HairModel* model) const;
        HairInstance* CreateInstance(const HairModel* model) const;
        void UpdateInstanceSettings(HairInstance* instance, const HairConfig& settings) const;
        void DestroyInstance(HairInstance* instance) const;
        void SimulateHair(HairInstance* instance, float timeStep = 1.0f / 60.0f) const;
        void RenderHair(const HairInstance* instance, const Matrix4& viewMatrix, const Matrix4& projectionMatrix) const;
        ~HairSimulationSystem();

    private:
        HairRenderer* hairRenderer;
    };
}

#endif