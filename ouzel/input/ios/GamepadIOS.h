// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "input/Gamepad.h"

#if defined(__OBJC__)
#include <GameController/GameController.h>
typedef GCController* GCControllerPtr;
#else
#include <objc/objc.h>
typedef id GCControllerPtr;
#endif

namespace ouzel
{
    namespace input
    {
        class InputIOS;

        class GamepadIOS: public Gamepad
        {
            friend InputIOS;
        public:
            virtual bool isAttached() const override;

            virtual int32_t getPlayerIndex() const override;
            virtual bool setPlayerIndex(int32_t playerIndex) override;

            GCControllerPtr getController() const { return controller; }

        protected:
            GamepadIOS(GCControllerPtr aController);

            GCControllerPtr controller;
        };
    } // namespace input
} // namespace ouzel
