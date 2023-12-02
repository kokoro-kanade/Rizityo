#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Input
{
    struct Axis
    {
        enum Type : uint32 
        {
            X = 0,
            Y = 1,
            Z = 2,
        };
    };

    struct ModifierKey
    {
        enum Key : uint32 
        {
            None = 0x00,
            LeftShift = 0x01,
            RightShift = 0x02,
            Shift = LeftShift | RightShift,
            LeftCtrl = 0x04,
            RightCtrl = 0x08,
            Ctrl = LeftCtrl | RightCtrl,
            LeftAlt = 0x10,
            RightAlt = 0x20,
            Alt = LeftAlt | RightAlt,
        };
    };

    struct InputValue
    {
        Math::DX_Vector3 Previous;
        Math::DX_Vector3 Current;
    };

    struct InputCode
    {
        enum Code : uint32 
        {
            MousePosition,
            MousePositionX,
            MousePositionY,
            MouseLeft,
            MouseRight,
            MouseMiddle,
            MouseWheel,

            KeyBackspace,
            KeyTab,
            KeyReturn,
            KeyShift,
            KeyLeftShift,
            KeyRightShift,
            KeyControl,
            KeyLeftControl,
            KeyRightControl,
            KeyAlt,
            KeyLeftAlt,
            KeyRightAlt,
            KeyPause,
            KeyCapslock,
            KeyEscape,
            KeySpace,
            KeyPageUp,
            KeyPageDown,
            KeyHome,
            KeyEnd,
            KeyLeft,
            KeyUp,
            KeyRight,
            KeyDown,
            KeyPrintScreen,
            KeyInsert,
            KeyDelete,

            Key0,
            Key1,
            Key2,
            Key3,
            Key4,
            Key5,
            Key6,
            Key7,
            Key8,
            Key9,

            KeyA,
            KeyB,
            KeyC,
            KeyD,
            KeyE,
            KeyF,
            KeyG,
            KeyH,
            KeyI,
            KeyJ,
            KeyK,
            KeyL,
            KeyM,
            KeyN,
            KeyO,
            KeyP,
            KeyQ,
            KeyR,
            KeyS,
            KeyT,
            KeyU,
            KeyV,
            KeyW,
            KeyX,
            KeyY,
            KeyZ,

            KeyNumpad0,
            KeyNumpad1,
            KeyNumpad2,
            KeyNumpad3,
            KeyNumpad4,
            KeyNumpad5,
            KeyNumpad6,
            KeyNumpad7,
            KeyNumpad8,
            KeyNumpad9,

            KeyMultiply,
            KeyAdd,
            KeySubtract,
            KeyDecimal,
            KeyDivide,

            KeyF1,
            KeyF2,
            KeyF3,
            KeyF4,
            KeyF5,
            KeyF6,
            KeyF7,
            KeyF8,
            KeyF9,
            KeyF10,
            KeyF11,
            KeyF12,

            KeyNumlock,
            KeyScrollock,
        };
    };

    struct InputSource
    {
        enum Type : uint32
        {
            Keyboard,
            Mouse,
            Controler,
            Raw,
            Count
        };

        uint64 BindingKey = 0;
        Type SourceType{};
        uint32 Code = 0;
        float32 Multiplier = 0;
        bool IsDiscrete = true;
        Axis::Type SourceAxis{};
        Axis::Type Axis{};
        ModifierKey::Key Modifier{};
    };

    void GetInputValue(InputSource::Type type, InputCode::Code code, OUT InputValue& value);
    void GetInputValue(uint64 binding, OUT InputValue& value);

    namespace Internal 
    {
        class InputSystemBase
        {
        public:

            virtual void OnEvent(InputSource::Type, InputCode::Code, const InputValue&) = 0;
            virtual void OnEvent(uint64 binding, const InputValue& value) = 0;

        protected:

            InputSystemBase();
            ~InputSystemBase();
        };

    } // Internal

    template<typename T>
    class InputSystem final : public Internal::InputSystemBase
    {
    public:

        using InputCallbackT = void(T::*)(InputSource::Type, InputCode::Code, const InputValue&);
        using BindingCallbackT = void(T::*)(uint64, const InputValue&);

        void AddHandler(InputSource::Type type, T* instance, InputCallbackT callback)
        {
            assert(instance && callback && type < InputSource::Count);
            auto& collection = InputCallbacks[type];

            // ハンドラーが既に追加されていれば追加しない
            for (const auto& func : collection)
            {
                if (func.Instance == instance && func.Callback == callback)
                    return;
            }

            collection.emplace_back(InputCallback{ instance, callback });
        }

        void AddHandler(uint64 bindingKey, T* instance, BindingCallbackT callback)
        {
            assert(instance && callback);

            // ハンドラーが既に追加されていれば追加しない
            for (const auto& func : BindingCallbacks)
            {
                if (func.BindingKey == bindingKey && func.Instance == instance && func.Callback == callback)
                    return;
            }

            BindingCallbacks.emplace_back(BindingCallback{ bindingKey, instance, callback });
        }

        void OnEvent(InputSource::Type type, InputCode::Code code, const InputValue& value) override
        {
            assert(type < InputSource::Count);
            for (const auto& item : InputCallbacks[type])
            {
                (item.Instance->*item.Callback)(type, code, value);
            }
        }

        void OnEvent(uint64 bindingKey, const InputValue& value) override
        {
            for (const auto& item : BindingCallbacks)
            {
                if (item.BindingKey == bindingKey)
                {
                    (item.Instance->*item.Callback)(bindingKey, value);
                }
            }
        }

    private:

        struct InputCallback
        {
            T* Instance;
            InputCallbackT Callback;
        };

        struct BindingCallback
        {
            uint64 BindingKey;
            T* Instance;
            BindingCallbackT Callback;
        };

        Utility::Vector<InputCallback> InputCallbacks[InputSource::Type::Count];
        Utility::Vector<BindingCallback> BindingCallbacks;
    };
}