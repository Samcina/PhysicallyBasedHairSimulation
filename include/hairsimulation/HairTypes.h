#ifndef HAIRTYPES_H
#define HAIRTYPES_H

#include <stdint.h>
#include <hairsimulation/Math.h>

namespace HairSimulation
{
    struct HairModelDescriptor
    {
        Vector4* positions;
        uint32_t segmentsCount;
        uint32_t strandsCount;
    };

    struct HairConfig
    {
        bool renderHair;
        bool renderStrands;
        bool renderRoot;
        Matrix4 modelMatrix;
        Vector3 windVecs;
        float globalConstraint;
        float localConstraint;
        float friction;
        float tesselationFactor;
        float rootWidth;
        float tipWidth;
        float thinningStart;
        float density;
        float ambientStrength;
        float diffuseStrength;
        float specularStrength;
        float specularPow;
        Vector4 color;


        HairConfig() :
            renderHair(true),
            renderStrands(false),
            renderRoot(false),
            windVecs(0, 0, 0),
            globalConstraint(0.002f),
            localConstraint(0.01f),
            friction(0.05f),
            tesselationFactor(4.0f),
            rootWidth(0.002f),
            tipWidth(0.0005f),
            thinningStart(0.5f),
            density(64.0f),
            ambientStrength(0.5f),
            diffuseStrength(0.5f),
            specularStrength(0.5f),
            specularPow(50.0f),
            color(0.95f, 0.9f, 0.625f, 1.0f)

        {
            modelMatrix.SetIdentity();
        }
    };
}

#endif