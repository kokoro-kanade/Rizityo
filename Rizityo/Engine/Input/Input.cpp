#include "Input.h"

namespace Rizityo::Input
{
    namespace
    {
        struct InputBinding
        {
            Utility::Vector<InputSource> Sources;
            InputValue Value{};
            bool IsDirty = true; // 値を取得するときにtrueであれば値を更新する(必要になったら更新する)
        };

        std::unordered_map<uint64, InputValue> InputValueMap;       // キーは(typeとcodeをつなげたもの)
        std::unordered_map<uint64, InputBinding> InputBindingMap;   // キーはバインディング用に設定したもの
        std::unordered_map<uint64, uint64> BindingKeyMap;           // キーは(typeとcodeをつなげたもの)
        Utility::Vector<Internal::InputSystemBase*> InputSystems;

        constexpr uint64 GetKey(InputSource::Type type, uint32 code)
        {
            return ((uint64)type << 32) | (uint64)code;
        }

    } // 無名空間

    void Bind(InputSource source)
    {
        assert(source.SourceType < InputSource::Count);
        const uint64 key = GetKey(source.SourceType, source.Code);
        Unbind(source.SourceType, (InputCode::Code)source.Code);

        InputBindingMap[source.BindingKey].Sources.emplace_back(source);
        BindingKeyMap[key] = source.BindingKey;
    }

    void Unbind(InputSource::Type type, InputCode::Code code)
    {
        assert(type < InputSource::Count);
        const uint64 key = GetKey(type, code);
        if (!BindingKeyMap.count(key))
        {
            return;
        }

        const uint64 bindingKey = BindingKeyMap[key];
        assert(InputBindingMap.count(bindingKey));
        InputBinding& binding{ InputBindingMap[bindingKey] };
        Utility::Vector<InputSource>& sources{ binding.Sources };
        uint32 index = UINT32_INVALID_NUM;
        for (uint32 i = 0; i < sources.size(); i++)
        {
            if (sources[i].SourceType == type && sources[i].Code == code)
            {
                assert(sources[i].BindingKey == BindingKeyMap[key]);
                index = i;
                break;
            }
        }

        if (index != UINT32_INVALID_NUM)
        {
            Utility::EraseUnordered(sources, index);
            BindingKeyMap.erase(key);
        }

        if (!sources.size())
        {
            assert(!BindingKeyMap.count(key));
            InputBindingMap.erase(bindingKey);
        }
    }

    void Unbind(uint64 bindingKey)
    {
        if (!InputBindingMap.count(bindingKey))
            return;

        Utility::Vector<InputSource>& sources{ InputBindingMap[bindingKey].Sources };
        for (const auto& source : sources)
        {
            assert(source.BindingKey == bindingKey);
            const uint64 key = GetKey(source.SourceType, source.Code);
            assert(BindingKeyMap.count(key) && BindingKeyMap[key] == bindingKey);
            BindingKeyMap.erase(key);
        }

        InputBindingMap.erase(bindingKey);
    }

    void SetInputValue(InputSource::Type type, InputCode::Code code, Math::Vector3 value)
    {
        assert(type < InputSource::Count);
        const uint64 key = GetKey(type, code);
        InputValue& input{ InputValueMap[key] };
        input.Previous = input.Current;
        input.Current = value;

        if (BindingKeyMap.count(key))
        {
            const uint64 bindingKey = BindingKeyMap[key];
            assert(InputBindingMap.count(bindingKey));
            InputBinding& binding{ InputBindingMap[bindingKey] };
            binding.IsDirty = true;

            InputValue bindingValue;
            GetInputValue(bindingKey, bindingValue);

            // TODO: マルチスレッドで実行した場合はデータ競合の可能性がある
            for (const auto& is : InputSystems)
            {
                is->OnEvent(bindingKey, bindingValue);
            }
        }

        // TODO: マルチスレッドで実行した場合はデータ競合の可能性がある
        for (const auto& is : InputSystems)
        {
            is->OnEvent(type, code, input);
        }
    }

    void GetInputValue(InputSource::Type type, InputCode::Code code, OUT InputValue& value)
    {
        assert(type < InputSource::Count);
        const uint64 key = GetKey(type, code);
        value = InputValueMap[key];
    }

    void GetInputValue(uint64 bindingKey, OUT InputValue& value)
    {
        if (!InputBindingMap.count(bindingKey))
            return;

        InputBinding& InputBinding{ InputBindingMap[bindingKey] };

        if (!InputBinding.IsDirty)
        {
            value = InputBinding.Value;
            return;
        }

        Utility::Vector<InputSource>& sources{ InputBindingMap[bindingKey].Sources };
        InputValue subInputValue{};
        InputValue result{};

        for (const auto& source : sources)
        {
            assert(source.BindingKey == bindingKey);
            GetInputValue(source.SourceType, (InputCode::Code)source.Code, subInputValue);
            assert(source.Axis <= Axis::Z);
            if (source.Axis > Axis::Z)
                return;

            if (source.SourceType == InputSource::Type::Mouse)
            {
                const float32 current = (&subInputValue.Current.x)[source.SourceAxis];
                const float32 previous = (&subInputValue.Previous.x)[source.SourceAxis];
                (&result.Current.x)[source.Axis] += (current - previous) * source.Multiplier;
            }
            else
            {
                (&result.Previous.x)[source.Axis] += subInputValue.Previous.x * source.Multiplier;
                (&result.Current.x)[source.Axis] += subInputValue.Current.x * source.Multiplier;
            }
        }

        InputBinding.Value = result;
        InputBinding.IsDirty = false;
        value = result;
    }

    Internal::InputSystemBase::InputSystemBase()
    {
        InputSystems.emplace_back(this);
    }

    Internal::InputSystemBase::~InputSystemBase()
    {
        for (uint32 i{ 0 }; i < InputSystems.size(); ++i)
        {
            if (InputSystems[i] == this)
            {
                Utility::EraseUnordered(InputSystems, i);
                break;
            }
        }
    }
}