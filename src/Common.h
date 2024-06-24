#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <hairsimulation/HairTypes.h>
#include <string>

namespace HairSimulation
{
    class HairModel
    {
    public:
        uint32_t segCount;
        uint32_t strandCount;
        uint32_t trianglesCount;
        uint32_t restBuffID;
        uint32_t tangentsBuffID;
        uint32_t hairIndicesBuffID;
        uint32_t refVecsBufferID;
        uint32_t globalRotBuffID;
        uint32_t debugBuffID;
    };

    class HairInstance
    {
    public:
        const HairModel* model;
        uint32_t frame;
        uint32_t posBuffID;
        uint32_t prevPosBuffID;
        HairConfig config;
    };

    std::string LoadFile(const char* path);
}

#endif