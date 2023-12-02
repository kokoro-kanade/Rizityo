#ifdef _WIN64

#include "PlatformInput.h"
#include "Input/Input.h"

namespace Rizityo::Platform
{
    namespace
    {
        constexpr uint32 VK_Mapping[256]{
            /* 0x00 */ UINT32_INVALID_NUM,
            /* 0x01 */ Input::InputCode::MouseLeft,
            /* 0x02 */ Input::InputCode::MouseRight,
            /* 0x03 */ UINT32_INVALID_NUM,
            /* 0x04 */ Input::InputCode::MouseMiddle,
            /* 0x05 */ UINT32_INVALID_NUM,
            /* 0x06 */ UINT32_INVALID_NUM,
            /* 0x07 */ UINT32_INVALID_NUM,
            /* 0x08 */ Input::InputCode::KeyBackspace,
            /* 0x09 */ Input::InputCode::KeyTab,
            /* 0x0A */ UINT32_INVALID_NUM,
            /* 0x0B */ UINT32_INVALID_NUM,
            /* 0x0C */ UINT32_INVALID_NUM,
            /* 0x0D */ Input::InputCode::KeyReturn,
            /* 0x0E */ UINT32_INVALID_NUM,
            /* 0x0F */ UINT32_INVALID_NUM,

            /* 0x10 */ Input::InputCode::KeyShift,
            /* 0x11 */ Input::InputCode::KeyControl,
            /* 0x12 */ Input::InputCode::KeyAlt,
            /* 0x13 */ Input::InputCode::KeyPause,
            /* 0x14 */ Input::InputCode::KeyCapslock,
            /* 0x15 */ UINT32_INVALID_NUM,
            /* 0x16 */ UINT32_INVALID_NUM,
            /* 0x17 */ UINT32_INVALID_NUM,
            /* 0x18 */ UINT32_INVALID_NUM,
            /* 0x19 */ UINT32_INVALID_NUM,
            /* 0x1A */ UINT32_INVALID_NUM,
            /* 0x1B */ Input::InputCode::KeyEscape,
            /* 0x1C */ UINT32_INVALID_NUM,
            /* 0x1D */ UINT32_INVALID_NUM,
            /* 0x1E */ UINT32_INVALID_NUM,
            /* 0x1F */ UINT32_INVALID_NUM,

            /* 0x20 */ Input::InputCode::KeySpace,
            /* 0x21 */ Input::InputCode::KeyPageUp,
            /* 0x22 */ Input::InputCode::KeyPageDown,
            /* 0x23 */ Input::InputCode::KeyEnd,
            /* 0x24 */ Input::InputCode::KeyHome,
            /* 0x25 */ Input::InputCode::KeyLeft,
            /* 0x26 */ Input::InputCode::KeyUp,
            /* 0x27 */ Input::InputCode::KeyRight,
            /* 0x28 */ Input::InputCode::KeyDown,
            /* 0x29 */ UINT32_INVALID_NUM,
            /* 0x2A */ UINT32_INVALID_NUM,
            /* 0x2B */ UINT32_INVALID_NUM,
            /* 0x2C */ Input::InputCode::KeyPrintScreen,
            /* 0x2D */ Input::InputCode::KeyInsert,
            /* 0x2E */ Input::InputCode::KeyDelete,
            /* 0x2F */ UINT32_INVALID_NUM,

            /* 0x30 */ Input::InputCode::Key0,
            /* 0x31 */ Input::InputCode::Key1,
            /* 0x32 */ Input::InputCode::Key2,
            /* 0x33 */ Input::InputCode::Key3,
            /* 0x34 */ Input::InputCode::Key4,
            /* 0x35 */ Input::InputCode::Key5,
            /* 0x36 */ Input::InputCode::Key6,
            /* 0x37 */ Input::InputCode::Key7,
            /* 0x38 */ Input::InputCode::Key8,
            /* 0x39 */ Input::InputCode::Key9,
            /* 0x3A */ UINT32_INVALID_NUM,
            /* 0x3B */ UINT32_INVALID_NUM,
            /* 0x3C */ UINT32_INVALID_NUM,
            /* 0x3D */ UINT32_INVALID_NUM,
            /* 0x3E */ UINT32_INVALID_NUM,
            /* 0x3F */ UINT32_INVALID_NUM,

            /* 0x40 */ UINT32_INVALID_NUM,
            /* 0x41 */ Input::InputCode::KeyA,
            /* 0x42 */ Input::InputCode::KeyB,
            /* 0x43 */ Input::InputCode::KeyC,
            /* 0x44 */ Input::InputCode::KeyD,
            /* 0x45 */ Input::InputCode::KeyE,
            /* 0x46 */ Input::InputCode::KeyF,
            /* 0x47 */ Input::InputCode::KeyG,
            /* 0x48 */ Input::InputCode::KeyH,
            /* 0x49 */ Input::InputCode::KeyI,
            /* 0x4A */ Input::InputCode::KeyJ,
            /* 0x4B */ Input::InputCode::KeyK,
            /* 0x4C */ Input::InputCode::KeyL,
            /* 0x4D */ Input::InputCode::KeyM,
            /* 0x4E */ Input::InputCode::KeyN,
            /* 0x4F */ Input::InputCode::KeyO,

            /* 0x50 */ Input::InputCode::KeyP,
            /* 0x51 */ Input::InputCode::KeyQ,
            /* 0x52 */ Input::InputCode::KeyR,
            /* 0x53 */ Input::InputCode::KeyS,
            /* 0x54 */ Input::InputCode::KeyT,
            /* 0x55 */ Input::InputCode::KeyU,
            /* 0x56 */ Input::InputCode::KeyV,
            /* 0x57 */ Input::InputCode::KeyW,
            /* 0x58 */ Input::InputCode::KeyX,
            /* 0x59 */ Input::InputCode::KeyY,
            /* 0x5A */ Input::InputCode::KeyZ,
            /* 0x5B */ UINT32_INVALID_NUM,
            /* 0x5C */ UINT32_INVALID_NUM,
            /* 0x5D */ UINT32_INVALID_NUM,
            /* 0x5E */ UINT32_INVALID_NUM,
            /* 0x5F */ UINT32_INVALID_NUM,

            /* 0x60 */ Input::InputCode::KeyNumpad0,
            /* 0x61 */ Input::InputCode::KeyNumpad1,
            /* 0x62 */ Input::InputCode::KeyNumpad2,
            /* 0x63 */ Input::InputCode::KeyNumpad3,
            /* 0x64 */ Input::InputCode::KeyNumpad4,
            /* 0x65 */ Input::InputCode::KeyNumpad5,
            /* 0x66 */ Input::InputCode::KeyNumpad6,
            /* 0x67 */ Input::InputCode::KeyNumpad7,
            /* 0x68 */ Input::InputCode::KeyNumpad8,
            /* 0x69 */ Input::InputCode::KeyNumpad9,
            /* 0x6A */ Input::InputCode::KeyMultiply,
            /* 0x6B */ Input::InputCode::KeyAdd,
            /* 0x6C */ UINT32_INVALID_NUM,
            /* 0x6D */ Input::InputCode::KeySubtract,
            /* 0x6E */ Input::InputCode::KeyDecimal,
            /* 0x6F */ Input::InputCode::KeyDivide,

            /* 0x70 */ Input::InputCode::KeyF1,
            /* 0x71 */ Input::InputCode::KeyF2,
            /* 0x72 */ Input::InputCode::KeyF3,
            /* 0x73 */ Input::InputCode::KeyF4,
            /* 0x74 */ Input::InputCode::KeyF5,
            /* 0x75 */ Input::InputCode::KeyF6,
            /* 0x76 */ Input::InputCode::KeyF7,
            /* 0x77 */ Input::InputCode::KeyF8,
            /* 0x78 */ Input::InputCode::KeyF9,
            /* 0x79 */ Input::InputCode::KeyF10,
            /* 0x7A */ Input::InputCode::KeyF11,
            /* 0x7B */ Input::InputCode::KeyF12,
            /* 0x7C */ UINT32_INVALID_NUM,
            /* 0x7D */ UINT32_INVALID_NUM,
            /* 0x7E */ UINT32_INVALID_NUM,
            /* 0x7F */ UINT32_INVALID_NUM,

            /* 0x80 */ UINT32_INVALID_NUM,
            /* 0x81 */ UINT32_INVALID_NUM,
            /* 0x82 */ UINT32_INVALID_NUM,
            /* 0x83 */ UINT32_INVALID_NUM,
            /* 0x84 */ UINT32_INVALID_NUM,
            /* 0x85 */ UINT32_INVALID_NUM,
            /* 0x86 */ UINT32_INVALID_NUM,
            /* 0x87 */ UINT32_INVALID_NUM,
            /* 0x88 */ UINT32_INVALID_NUM,
            /* 0x89 */ UINT32_INVALID_NUM,
            /* 0x8A */ UINT32_INVALID_NUM,
            /* 0x8B */ UINT32_INVALID_NUM,
            /* 0x8C */ UINT32_INVALID_NUM,
            /* 0x8D */ UINT32_INVALID_NUM,
            /* 0x8E */ UINT32_INVALID_NUM,
            /* 0x8F */ UINT32_INVALID_NUM,

            /* 0x90 */ Input::InputCode::KeyNumlock,
            /* 0x91 */ Input::InputCode::KeyScrollock,
            /* 0x92 */ UINT32_INVALID_NUM,
            /* 0x93 */ UINT32_INVALID_NUM,
            /* 0x94 */ UINT32_INVALID_NUM,
            /* 0x95 */ UINT32_INVALID_NUM,
            /* 0x96 */ UINT32_INVALID_NUM,
            /* 0x97 */ UINT32_INVALID_NUM,
            /* 0x98 */ UINT32_INVALID_NUM,
            /* 0x99 */ UINT32_INVALID_NUM,
            /* 0x9A */ UINT32_INVALID_NUM,
            /* 0x9B */ UINT32_INVALID_NUM,
            /* 0x9C */ UINT32_INVALID_NUM,
            /* 0x9D */ UINT32_INVALID_NUM,
            /* 0x9E */ UINT32_INVALID_NUM,
            /* 0x9F */ UINT32_INVALID_NUM,

            /* 0xA0 */ UINT32_INVALID_NUM,
            /* 0xA1 */ UINT32_INVALID_NUM,
            /* 0xA2 */ UINT32_INVALID_NUM,
            /* 0xA3 */ UINT32_INVALID_NUM,
            /* 0xA4 */ UINT32_INVALID_NUM,
            /* 0xA5 */ UINT32_INVALID_NUM,
            /* 0xA6 */ UINT32_INVALID_NUM,
            /* 0xA7 */ UINT32_INVALID_NUM,
            /* 0xA8 */ UINT32_INVALID_NUM,
            /* 0xA9 */ UINT32_INVALID_NUM,
            /* 0xAA */ UINT32_INVALID_NUM,
            /* 0xAB */ UINT32_INVALID_NUM,
            /* 0xAC */ UINT32_INVALID_NUM,
            /* 0xAD */ UINT32_INVALID_NUM,
            /* 0xAE */ UINT32_INVALID_NUM,
            /* 0xAF */ UINT32_INVALID_NUM,

            /* 0xB0 */ UINT32_INVALID_NUM,
            /* 0xB1 */ UINT32_INVALID_NUM,
            /* 0xB2 */ UINT32_INVALID_NUM,
            /* 0xB3 */ UINT32_INVALID_NUM,
            /* 0xB4 */ UINT32_INVALID_NUM,
            /* 0xB5 */ UINT32_INVALID_NUM,
            /* 0xB6 */ UINT32_INVALID_NUM,
            /* 0xB7 */ UINT32_INVALID_NUM,
            /* 0xB8 */ UINT32_INVALID_NUM,
            /* 0xB9 */ UINT32_INVALID_NUM,
            /* 0xBA */ UINT32_INVALID_NUM,
            /* 0xBB */ UINT32_INVALID_NUM,
            /* 0xBC */ UINT32_INVALID_NUM,
            /* 0xBD */ UINT32_INVALID_NUM,
            /* 0xBE */ UINT32_INVALID_NUM,
            /* 0xBF */ UINT32_INVALID_NUM,

            /* 0xC0 */ UINT32_INVALID_NUM,
            /* 0xC1 */ UINT32_INVALID_NUM,
            /* 0xC2 */ UINT32_INVALID_NUM,
            /* 0xC3 */ UINT32_INVALID_NUM,
            /* 0xC4 */ UINT32_INVALID_NUM,
            /* 0xC5 */ UINT32_INVALID_NUM,
            /* 0xC6 */ UINT32_INVALID_NUM,
            /* 0xC7 */ UINT32_INVALID_NUM,
            /* 0xC8 */ UINT32_INVALID_NUM,
            /* 0xC9 */ UINT32_INVALID_NUM,
            /* 0xCA */ UINT32_INVALID_NUM,
            /* 0xCB */ UINT32_INVALID_NUM,
            /* 0xCC */ UINT32_INVALID_NUM,
            /* 0xCD */ UINT32_INVALID_NUM,
            /* 0xCE */ UINT32_INVALID_NUM,
            /* 0xCF */ UINT32_INVALID_NUM,

            /* 0xD0 */ UINT32_INVALID_NUM,
            /* 0xD1 */ UINT32_INVALID_NUM,
            /* 0xD2 */ UINT32_INVALID_NUM,
            /* 0xD3 */ UINT32_INVALID_NUM,
            /* 0xD4 */ UINT32_INVALID_NUM,
            /* 0xD5 */ UINT32_INVALID_NUM,
            /* 0xD6 */ UINT32_INVALID_NUM,
            /* 0xD7 */ UINT32_INVALID_NUM,
            /* 0xD8 */ UINT32_INVALID_NUM,
            /* 0xD9 */ UINT32_INVALID_NUM,
            /* 0xDA */ UINT32_INVALID_NUM,
            /* 0xDB */ UINT32_INVALID_NUM,
            /* 0xDC */ UINT32_INVALID_NUM,
            /* 0xDD */ UINT32_INVALID_NUM,
            /* 0xDE */ UINT32_INVALID_NUM,
            /* 0xDF */ UINT32_INVALID_NUM,

            /* 0xE0 */ UINT32_INVALID_NUM,
            /* 0xE1 */ UINT32_INVALID_NUM,
            /* 0xE2 */ UINT32_INVALID_NUM,
            /* 0xE3 */ UINT32_INVALID_NUM,
            /* 0xE4 */ UINT32_INVALID_NUM,
            /* 0xE5 */ UINT32_INVALID_NUM,
            /* 0xE6 */ UINT32_INVALID_NUM,
            /* 0xE7 */ UINT32_INVALID_NUM,
            /* 0xE8 */ UINT32_INVALID_NUM,
            /* 0xE9 */ UINT32_INVALID_NUM,
            /* 0xEA */ UINT32_INVALID_NUM,
            /* 0xEB */ UINT32_INVALID_NUM,
            /* 0xEC */ UINT32_INVALID_NUM,
            /* 0xED */ UINT32_INVALID_NUM,
            /* 0xEE */ UINT32_INVALID_NUM,
            /* 0xEF */ UINT32_INVALID_NUM,

            /* 0xF0 */ UINT32_INVALID_NUM,
            /* 0xF1 */ UINT32_INVALID_NUM,
            /* 0xF2 */ UINT32_INVALID_NUM,
            /* 0xF3 */ UINT32_INVALID_NUM,
            /* 0xF4 */ UINT32_INVALID_NUM,
            /* 0xF5 */ UINT32_INVALID_NUM,
            /* 0xF6 */ UINT32_INVALID_NUM,
            /* 0xF7 */ UINT32_INVALID_NUM,
            /* 0xF8 */ UINT32_INVALID_NUM,
            /* 0xF9 */ UINT32_INVALID_NUM,
            /* 0xFA */ UINT32_INVALID_NUM,
            /* 0xFB */ UINT32_INVALID_NUM,
            /* 0xFC */ UINT32_INVALID_NUM,
            /* 0xFD */ UINT32_INVALID_NUM,
            /* 0xFE */ UINT32_INVALID_NUM,
            /* 0xFF */ UINT32_INVALID_NUM,
        };

        struct ModifierFlags
        {
            enum Flags : uint8 
            {
                LeftShift = 0x10,
                LeftControl = 0x20,
                LeftAlt = 0x40,

                RightShift = 0x01,
                RightControl = 0x02,
                RightAlt = 0x04,
            };
        };

        uint8 ModifierKeysState = 0;

        void SetModifierInput(uint8 virtualKey, Input::InputCode::Code code, ModifierFlags::Flags flags)
        {
            if (GetKeyState(virtualKey) < 0)
            {
                SetInputValue(Input::InputSource::Keyboard, code, { 1.f, 0.f, 0.f });
                ModifierKeysState |= flags;
            }
            else if (ModifierKeysState & flags)
            {
                SetInputValue(Input::InputSource::Keyboard, code, { 0.f, 0.f, 0.f });
                ModifierKeysState &= ~flags;
            }
        }

        void SetModifierInputs(Input::InputCode::Code code)
        {
            if (code == Input::InputCode::KeyShift)
            {
                SetModifierInput(VK_LSHIFT, Input::InputCode::KeyLeftShift, ModifierFlags::LeftShift);
                SetModifierInput(VK_RSHIFT, Input::InputCode::KeyRightShift, ModifierFlags::RightShift);
            }
            else if (code == Input::InputCode::KeyControl)
            {
                SetModifierInput(VK_LCONTROL, Input::InputCode::KeyLeftControl, ModifierFlags::LeftControl);
                SetModifierInput(VK_RCONTROL, Input::InputCode::KeyRightControl, ModifierFlags::RightControl);
            }
            else if (code == Input::InputCode::KeyAlt)
            {
                SetModifierInput(VK_LMENU, Input::InputCode::KeyLeftAlt, ModifierFlags::LeftAlt);
                SetModifierInput(VK_RMENU, Input::InputCode::KeyRightAlt, ModifierFlags::RightAlt);
            }
        }

        constexpr Math::DX_Vector2 GetMousePosition(LPARAM lparam)
        {
            return { (float32)((int16)(lparam & 0x0000ffff)), (float32)((int16)(lparam >> 16)) };
        }

    } // –³–¼‹óŠÔ

    HRESULT ProcessInputMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            assert(wparam <= 0xff);
            const Input::InputCode::Code code{ VK_Mapping[wparam & 0xff] };
            if (code != UINT32_INVALID_NUM)
            {
                SetInputValue(Input::InputSource::Keyboard, code, { 1.f, 0.f, 0.f });
                SetModifierInputs(code);
            }
        }
        break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            assert(wparam <= 0xff);
            const Input::InputCode::Code code{ VK_Mapping[wparam & 0xff] };
            if (code != UINT32_INVALID_NUM)
            {
                SetInputValue(Input::InputSource::Keyboard, code, { 0.f, 0.f, 0.f });
                SetModifierInputs(code);
            }
        }
        break;
        case WM_MOUSEMOVE:
        {
            const Math::DX_Vector2 pos{ GetMousePosition(lparam) };
            SetInputValue(Input::InputSource::Mouse, Input::InputCode::MousePositionX, { pos.x, 0.f, 0.f });
            SetInputValue(Input::InputSource::Mouse, Input::InputCode::MousePositionY, { pos.y, 0.f, 0.f });
            SetInputValue(Input::InputSource::Mouse, Input::InputCode::MousePosition, { pos.x, pos.y, 0.f });
        }
        break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        {
            SetCapture(hwnd);
            const Input::InputCode::Code code{ msg == WM_LBUTTONDOWN ? Input::InputCode::MouseLeft : msg == WM_RBUTTONDOWN ? Input::InputCode::MouseRight : Input::InputCode::MouseMiddle };
            const Math::DX_Vector2 pos{ GetMousePosition(lparam) };
            SetInputValue(Input::InputSource::Mouse, code, { pos.x, pos.y, 1.f });
        }
        break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            ReleaseCapture();
            const Input::InputCode::Code code{ msg == WM_LBUTTONUP ? Input::InputCode::MouseLeft : msg == WM_RBUTTONUP ? Input::InputCode::MouseRight : Input::InputCode::MouseMiddle };
            const Math::DX_Vector2 pos{ GetMousePosition(lparam) };
            SetInputValue(Input::InputSource::Mouse, code, { pos.x, pos.y, 0.f });
        }
        break;
        case WM_MOUSEHWHEEL:
        {
            SetInputValue(Input::InputSource::Mouse, Input::InputCode::MouseWheel, { (float32)(GET_WHEEL_DELTA_WPARAM(wparam)), 0.f, 0.f });
        }
        break;
        }

        return S_OK;
    }
}

#endif // !_WIN64