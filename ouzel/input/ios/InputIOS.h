// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "input/Input.h"

#if defined(__OBJC__)
#include <GameController/GameController.h>
typedef GCController* GCControllerPtr;
@class ConnectDelegate;
typedef ConnectDelegate* ConnectDelegatePtr;
#else
#include <objc/objc.h>
#include <objc/NSObjCRuntime.h>
typedef id GCControllerPtr;
typedef id ConnectDelegatePtr;
#endif

namespace ouzel
{
    namespace input
    {
        class InputIOS: public Input
        {
            friend Engine;
        public:
            virtual void startGamepadDiscovery() override;
            virtual void stopGamepadDiscovery() override;

            virtual bool showVirtualKeyboard() override;
            virtual bool hideVirtualKeyboard() override;

            void handleGamepadDiscoveryCompleted();
            void handleGamepadConnected(GCControllerPtr controller);
            void handleGamepadDisconnected(GCControllerPtr controller);

        protected:
            InputIOS();

            ConnectDelegatePtr connectDelegate = nullptr;

            bool discovering = false;
            bool cursorVisible = true;
            bool cursorLocked = false;
        };
    } // namespace input
} // namespace ouzel
