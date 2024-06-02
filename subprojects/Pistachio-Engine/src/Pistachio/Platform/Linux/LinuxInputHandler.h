#pragma once
#include "Pistachio/Core/Input.h"

namespace Pistachio 
{
    
    class LinuxInputHandler : public InputHandler
    {
        virtual bool IsKeyPressed(KeyCode code) override;
        virtual bool IsKeyJustPressed(KeyCode code) override;
        virtual bool IsMouseButtonPressed(MouseButton code) override;
        virtual bool IsMouseButtonJustPressed(MouseButton code) override;
        virtual int GetMouseX(bool WindowCoordinates = false) override;
        virtual int GetMouseY(bool WindowCoordinates = false) override;
        virtual float GetLeftAnalogX(int ID) override;
        virtual float GetLeftAnalogY(int ID) override;
        virtual float GetRightAnalogX(int ID) override;
        virtual float GetRightAnalogY(int ID) override;
        virtual bool IsGamepadButtonPressed(int ID, int code) override;
        virtual bool IsGamepadButtonJustPressed(int ID, int code) override;
        virtual void VibrateController(int ID, int left, int right) override;
        virtual float GetLeftTriggerState(int ID) override;
        virtual float GetRightTriggerState(int ID) override;
    };
}