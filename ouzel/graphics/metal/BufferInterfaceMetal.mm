// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "core/CompileConfig.h"

#if OUZEL_SUPPORTS_METAL

#include <algorithm>
#include "BufferInterfaceMetal.h"
#include "RendererMetal.h"
#include "core/Engine.h"
#include "utils/Log.h"

namespace ouzel
{
    namespace graphics
    {
        BufferInterfaceMetal::BufferInterfaceMetal()
        {
        }

        BufferInterfaceMetal::~BufferInterfaceMetal()
        {
            if (buffer)
            {
                [buffer release];
            }
        }

        bool BufferInterfaceMetal::init(Buffer::Usage newUsage, uint32_t newFlags)
        {
            if (!BufferInterface::init(newUsage, newFlags))
            {
                return false;
            }

            return true;
        }

        bool BufferInterfaceMetal::init(Buffer::Usage newUsage, const std::vector<uint8_t>& newData, uint32_t newFlags)
        {
            if (!BufferInterface::init(newUsage, newData, newFlags))
            {
                return false;
            }

            if (!data.empty())
            {
                RendererMetal* rendererMetal = static_cast<RendererMetal*>(sharedEngine->getRenderer());

                bufferSize = static_cast<uint32_t>(data.size());

                if (buffer) [buffer release];

                buffer = [rendererMetal->getDevice() newBufferWithLength:bufferSize
                                                                 options:MTLResourceCPUCacheModeWriteCombined];

                if (!buffer)
                {
                    Log(Log::Level::ERR) << "Failed to create Metal buffer";
                    return false;
                }

                std::copy(data.begin(), data.end(), static_cast<uint8_t*>([buffer contents]));
            }

            return true;
        }

        bool BufferInterfaceMetal::setData(const std::vector<uint8_t>& newData)
        {
            if (!BufferInterface::setData(newData))
            {
                return false;
            }

            if (!data.empty())
            {
                if (!buffer || data.size() > bufferSize)
                {
                    RendererMetal* rendererMetal = static_cast<RendererMetal*>(sharedEngine->getRenderer());

                    bufferSize = static_cast<uint32_t>(data.size());

                    if (buffer) [buffer release];

                    buffer = [rendererMetal->getDevice() newBufferWithLength:bufferSize
                                                                     options:MTLResourceCPUCacheModeWriteCombined];

                    if (!buffer)
                    {
                        Log(Log::Level::ERR) << "Failed to create Metal buffer";
                        return false;
                    }

                    bufferSize = static_cast<uint32_t>(data.size());
                }

                std::copy(data.begin(), data.end(), static_cast<uint8_t*>([buffer contents]));
            }

            return true;
        }
    } // namespace graphics
} // namespace ouzel

#endif
