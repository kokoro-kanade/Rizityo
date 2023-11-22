#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Utility {

    // ����: ���̃N���X�̓��[�J���ɗp����
    //       �����o�ϐ��Ƃ��Ď������肵�Ȃ�
    class BinaryReader
    {
    public:
        DISABLE_COPY_AND_MOVE(BinaryReader);
        explicit BinaryReader(const uint8* buffer)
            :_Buffer{ buffer }, _Position{ buffer }
        {
            assert(buffer);
        }

        // �v���~�e�B�u�^��z��
        template<typename T>
        [[nodiscard]] T Read()
        {
            static_assert(std::is_arithmetic_v<T>, "Template argument should be a primitve type.");
            T value{ *((T*)_Position) };
            _Position += sizeof(T);
            return value;
        }

        // �Ăяo�����͓ǂ݂����̂ɏ\���ȃ��������m�ۂ��Ă���O��
        void Read(uint8* dstBuffer, size_t length)
        {
            memcpy(dstBuffer, _Position, length);
            _Position += length;
        }

        void Skip(size_t offset)
        {
            _Position += offset;
        }

        [[nodiscard]] constexpr const uint8* const BufferStart() const { return _Buffer; }
        [[nodiscard]] constexpr const uint8* const Position() const { return _Position; }
        [[nodiscard]] constexpr size_t Offset() const { return _Position - _Buffer; }

    private:
        const uint8* const _Buffer;
        const uint8* _Position;
    };

    // ����: ���̃N���X�̓��[�J���ɗp����
    //       �����o�ϐ��Ƃ��Ď������肵�Ȃ�
    class BinaryWriter
    {
    public:
        DISABLE_COPY_AND_MOVE(BinaryWriter);
        explicit BinaryWriter(uint8* dstBuffer, size_t buffer_size)
            :_Buffer{ dstBuffer }, _Position{ dstBuffer }, _BufferSize{ buffer_size }
        {
            assert(dstBuffer&& buffer_size);
        }

        // �v���~�e�B�u�^��z��
        template<typename T>
        void Write(T value)
        {
            static_assert(std::is_arithmetic_v<T>, "Template argument should be a primitve type.");
            assert(&_Position[sizeof(T)] <= &_Buffer[_BufferSize]);
            *((T*)_Position) = value;
            _Position += sizeof(T);
        }

        // buffer����length����������������.
        void Write(const char* srcBuffer, size_t length)
        {
            assert(&_Position[length] <= &_Buffer[_BufferSize]);
            memcpy(_Position, srcBuffer, length);
            _Position += length;
        }

        // buffer����length�T�C�Y�̃o�C�g����������
        void Write(const uint8* srcBuffer, size_t length)
        {
            assert(&_Position[length] <= &_Buffer[_BufferSize]);
            memcpy(_Position, srcBuffer, length);
            _Position += length;
        }

        void Skip(size_t offset)
        {
            assert(&_Position[offset] <= &_Buffer[_BufferSize]);
            _Position += offset;
        }

        [[nodiscard]] constexpr const uint8* const buffer_start() const { return _Buffer; }
        [[nodiscard]] constexpr const uint8* const buffer_end() const { return &_Buffer[_BufferSize]; }
        [[nodiscard]] constexpr const uint8* const position() const { return _Position; }
        [[nodiscard]] constexpr size_t offset() const { return _Position - _Buffer; }

    private:
        uint8* const _Buffer;
        uint8* _Position;
        size_t _BufferSize;

    };

}
