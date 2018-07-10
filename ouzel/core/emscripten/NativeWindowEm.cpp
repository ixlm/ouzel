// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#include <emscripten.h>
#include <emscripten/html5.h>
#include "NativeWindowEm.hpp"
#include "core/Engine.hpp"

static EM_BOOL emResizeCallback(int eventType, const EmscriptenUiEvent* uiEvent, void* userData)
{
    if (eventType == EMSCRIPTEN_EVENT_RESIZE)
    {
        reinterpret_cast<ouzel::NativeWindowEm*>(userData)->handleResize();
        return true;
    }

    return false;
}

static EM_BOOL emFullscreenCallback(int eventType, const void*, void* userData)
{
    reinterpret_cast<ouzel::NativeWindowEm*>(userData)->handleResize();
    return true;
}

namespace ouzel
{
    NativeWindowEm::NativeWindowEm(const Size2& newSize,
                                   bool newFullscreen,
                                   const std::string& newTitle):
        ouzel::NativeWindow(newSize,
                            true,
                            newFullscreen,
                            true,
                            newTitle,
                            true)
    {
        emscripten_set_resize_callback(nullptr, this, 1, emResizeCallback);

        if (size.width <= 0.0F || size.height <= 0.0F)
        {
            int width, height, fullscreen;
            emscripten_get_canvas_size(&width, &height, &fullscreen);

            if (size.width <= 0.0F) size.width = static_cast<float>(width);
            if (size.height <= 0.0F) size.height = static_cast<float>(height);
        }
        else
            emscripten_set_canvas_size(static_cast<int>(size.width),
                                       static_cast<int>(size.height));

        if (fullscreen)
        {
            EmscriptenFullscreenStrategy s;
            s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;
            s.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
            s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
            s.canvasResizedCallback = emFullscreenCallback;
            s.canvasResizedCallbackUserData = this;

            emscripten_request_fullscreen_strategy(nullptr, EM_TRUE, &s);
        }

        resolution = size;
    }

    void NativeWindowEm::setSize(const Size2& newSize)
    {
        NativeWindow::setSize(newSize);

        emscripten_set_canvas_size(static_cast<int>(newSize.width),
                                   static_cast<int>(newSize.height));
    }

    void NativeWindowEm::setFullscreen(bool newFullscreen)
    {
        NativeWindow::setFullscreen(newFullscreen);

        if (fullscreen)
        {
            EmscriptenFullscreenStrategy s;
            s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;
            s.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
            s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
            s.canvasResizedCallback = emFullscreenCallback;
            s.canvasResizedCallbackUserData = this;

            emscripten_request_fullscreen_strategy(nullptr, EM_TRUE, &s);
        }
        else
            emscripten_exit_fullscreen();
    }

    void NativeWindowEm::handleResize()
    {
        int width, height, fullscreen;
        emscripten_get_canvas_size(&width, &height, &fullscreen);

        Size2 newSize(static_cast<float>(width), static_cast<float>(height));

        size = newSize;
        resolution = size;

        if (listener)
        {
            listener->onSizeChange(size);
            listener->onResolutionChange(resolution);
        }
    }
}
