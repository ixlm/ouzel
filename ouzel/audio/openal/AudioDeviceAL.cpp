// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "core/Setup.h"

#if OUZEL_COMPILE_OPENAL

#if OUZEL_PLATFORM_IOS || OUZEL_PLATFORM_TVOS
#include <objc/message.h>
extern "C" id const AVAudioSessionCategoryAmbient;
#endif

#include "AudioDeviceAL.hpp"
#include "core/Engine.hpp"
#include "utils/Errors.hpp"
#include "utils/Log.hpp"

namespace ouzel
{
    namespace audio
    {
        AudioDeviceAL::AudioDeviceAL():
            AudioDevice(Audio::Driver::OPENAL)
        {
#if OUZEL_PLATFORM_IOS || OUZEL_PLATFORM_TVOS
            id audioSession = reinterpret_cast<id (*)(Class, SEL)>(&objc_msgSend)(objc_getClass("AVAudioSession"), sel_getUid("sharedInstance"));
            reinterpret_cast<BOOL (*)(id, SEL, id, id)>(&objc_msgSend)(audioSession, sel_getUid("setCategory:error:"), AVAudioSessionCategoryAmbient, nil);
#endif

#if OUZEL_MULTITHREADED
            running = false;
#endif

            std::fill(std::begin(bufferIds), std::end(bufferIds), 0);
        }

        AudioDeviceAL::~AudioDeviceAL()
        {
#if OUZEL_MULTITHREADED
            running = false;
            if (audioThread.isJoinable()) audioThread.join();
#endif

            if (sourceId)
            {
                alSourceStop(sourceId);
                alSourcei(sourceId, AL_BUFFER, 0);
                alDeleteSources(1, &sourceId);

                if (checkOpenALError())
                    Log(Log::Level::ERR) << "Failed to delete OpenAL source";
            }

            for (ALuint bufferId : bufferIds)
            {
                if (bufferId)
                {
                    alDeleteBuffers(1, &bufferId);

                    if (checkOpenALError())
                        Log(Log::Level::ERR) << "Failed to delete OpenAL buffer";
                }
            }

            if (context)
            {
                alcMakeContextCurrent(nullptr);
                alcDestroyContext(context);
            }

            if (device)
                alcCloseDevice(device);
        }

        void AudioDeviceAL::init(bool debugAudio)
        {
            AudioDevice::init(debugAudio);

            const ALCchar* deviceName = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);

            if (deviceName)
                Log(Log::Level::INFO) << "Using " << deviceName << " for audio";

            device = alcOpenDevice(deviceName);

            if (!device || checkALCError())
                throw SystemError("Failed to create OpenAL device");

            int capabilities[] =
            {
                ALC_FREQUENCY, 44100,
                ALC_STEREO_SOURCES, 4,
                0, 0
            };

            context = alcCreateContext(device, capabilities);

            if (checkALCError())
                throw SystemError("Failed to create OpenAL context");

            alcMakeContextCurrent(context);

            if (checkALCError())
                throw SystemError("Failed to make OpenAL context current");

#if !OUZEL_PLATFORM_EMSCRIPTEN
            format40 = alGetEnumValue("AL_FORMAT_QUAD16");
            format51 = alGetEnumValue("AL_FORMAT_51CHN16");
            format61 = alGetEnumValue("AL_FORMAT_61CHN16");
            format71 = alGetEnumValue("AL_FORMAT_71CHN16");
#endif

            if (checkOpenALError())
                Log(Log::Level::WARN) << "Failed to get OpenAL enum values";

            alGenSources(1, &sourceId);

            if (checkOpenALError())
                throw SystemError("Failed to create OpenAL source");

            alGenBuffers(2, bufferIds);

            if (checkOpenALError())
                throw SystemError("Failed to create OpenAL buffers");

            switch (channels)
            {
                case 1: format = AL_FORMAT_MONO16; break;
                case 2: format = AL_FORMAT_STEREO16; break;
                case 4: format = format40; break;
                case 6: format = format51; break;
                case 7: format = format61; break;
                case 8: format = format71; break;
                default:
                    throw SystemError("Invalid channel count");
            }

            sampleFormat = Audio::SampleFormat::SINT16;

            getData(bufferSize / (channels * sizeof(int16_t)), data);

            alBufferData(bufferIds[0], format,
                         data.data(),
                         static_cast<ALsizei>(data.size()),
                         static_cast<ALsizei>(sampleRate));

            getData(bufferSize / (channels * sizeof(int16_t)), data);

            alBufferData(bufferIds[1], format,
                         data.data(),
                         static_cast<ALsizei>(data.size()),
                         static_cast<ALsizei>(sampleRate));

            nextBuffer = 0;

            alSourceQueueBuffers(sourceId, 2, bufferIds);

            if (checkOpenALError())
                throw SystemError("Failed to queue OpenAL buffers");

            alSourcePlay(sourceId);

            if (checkOpenALError())
                throw SystemError("Failed to play OpenAL source");

#if OUZEL_MULTITHREADED
            running = true;
            audioThread = Thread(std::bind(&AudioDeviceAL::run, this), "Audio");
#endif
        }

        void AudioDeviceAL::process()
        {
            AudioDevice::process();

            alcMakeContextCurrent(context);

            if (checkALCError())
                throw SystemError("Failed to make OpenAL context current");

            ALint buffersProcessed;
            alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &buffersProcessed);

            if (checkOpenALError())
                throw SystemError("Failed to get processed buffer count");

            // requeue all processed buffers
            for (; buffersProcessed > 0; --buffersProcessed)
            {
                alSourceUnqueueBuffers(sourceId, 1, &bufferIds[nextBuffer]);

                if (checkOpenALError())
                    throw SystemError("Failed to unqueue OpenAL buffer");

                if (!getData(bufferSize / (channels * sizeof(int16_t)), data))
                    throw SystemError("Failed to get data");

                alBufferData(bufferIds[nextBuffer], format,
                             data.data(),
                             static_cast<ALsizei>(data.size()),
                             static_cast<ALsizei>(sampleRate));

                alSourceQueueBuffers(sourceId, 1, &bufferIds[nextBuffer]);

                if (checkOpenALError())
                    throw SystemError("Failed to queue OpenAL buffer");

                ALint state;
                alGetSourcei(sourceId, AL_SOURCE_STATE, &state);
                if (state != AL_PLAYING)
                {
                    alSourcePlay(sourceId);

                    if (checkOpenALError())
                        throw SystemError("Failed to play OpenAL source");
                }

                // swap the buffer
                nextBuffer = (nextBuffer + 1) % 2;
            }
        }

        void AudioDeviceAL::run()
        {
#if OUZEL_MULTITHREADED
            while (running) process();
#endif
        }
    } // namespace audio
} // namespace ouzel

#endif
