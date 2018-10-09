// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#include "Touchpad.hpp"
#include "core/Engine.hpp"
#include "events/EventDispatcher.hpp"

namespace ouzel
{
    namespace input
    {
        Touchpad::Touchpad(InputManager& initInputManager, uint32_t initDeviceId):
            Controller(initInputManager, Controller::Type::TOUCHPAD, initDeviceId)
        {
        }

        bool Touchpad::handleTouchBegin(uint64_t touchId, const Vector2& position, float force)
        {
            Event event;
            event.type = Event::Type::TOUCH_BEGIN;
            event.touchEvent.touchpad = this;
            event.touchEvent.touchId = touchId;
            event.touchEvent.position = position;
            event.touchEvent.force = force;

            touchPositions[touchId] = position;

            return engine->getEventDispatcher().dispatchEvent(event);
        }

        bool Touchpad::handleTouchEnd(uint64_t touchId, const Vector2& position, float force)
        {
            Event event;
            event.type = Event::Type::TOUCH_END;
            event.touchEvent.touchpad = this;
            event.touchEvent.touchId = touchId;
            event.touchEvent.position = position;
            event.touchEvent.force = force;

            auto i = touchPositions.find(touchId);

            if (i != touchPositions.end())
                touchPositions.erase(i);

            return engine->getEventDispatcher().dispatchEvent(event);
        }

        bool Touchpad::handleTouchMove(uint64_t touchId, const Vector2& position, float force)
        {
            Event event;
            event.type = Event::Type::TOUCH_MOVE;
            event.touchEvent.touchpad = this;
            event.touchEvent.touchId = touchId;
            event.touchEvent.difference = position - touchPositions[touchId];
            event.touchEvent.position = position;
            event.touchEvent.force = force;

            touchPositions[touchId] = position;

            return engine->getEventDispatcher().dispatchEvent(event);
        }

        bool Touchpad::handleTouchCancel(uint64_t touchId, const Vector2& position, float force)
        {
            Event event;
            event.type = Event::Type::TOUCH_CANCEL;
            event.touchEvent.touchpad = this;
            event.touchEvent.touchId = touchId;
            event.touchEvent.position = position;
            event.touchEvent.force = force;

            auto i = touchPositions.find(touchId);

            if (i != touchPositions.end())
                touchPositions.erase(i);

            return engine->getEventDispatcher().dispatchEvent(event);
        }
    } // namespace input
} // namespace ouzel
