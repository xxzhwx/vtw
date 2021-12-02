#pragma once
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "base/macro.h"
#include "base/noncopyable.h"

namespace vtw
{

class ByteBuffer final : noncopyable
{
public:
    ByteBuffer() :
        ByteBuffer(nullptr, 0)
    {

    }

    ByteBuffer(char *data, size_t length) :
        _data(data),
        _length(length),
        _rpos(0),
        _wpos(0)
    {

    }

    ~ByteBuffer()
    {
        SAFE_FREE(_data);
        _length = 0;
        _rpos = 0;
        _wpos = 0;
    }

public:
    template<typename T>
    ByteBuffer& operator >> (T &val)
    {
        val = ReadAtom<T>();
        return *this;
    }

    ByteBuffer& operator >> (const char *&str)
    {
        str = ReadBasicStringPtr<char, unsigned short>();
        return *this;
    }

    template<typename T>
    ByteBuffer& operator << (const T &val)
    {
        WriteAtom(val);
        return *this;
    }

    ByteBuffer& operator << (const char *str)
    {
        WriteBasicString<char, unsigned short>(str, -1);
        return *this;
    }

    ByteBuffer& operator << (char *str)
    {
        WriteBasicString<char, unsigned short>(str, -1);
        return *this;
    }

    template<typename T>
    T ReadAtom()
    {
        T val;
        size_t sz = sizeof(T);
        if (Read(&val, sz) == 0)
        {
            memset(&val, 0, sz);
        }

        return val;
    }

    template<typename T>
    void WriteAtom(const T &val)
    {
        Write(&val, sizeof(T));
    }

    template<typename TC, typename TL>
    size_t ReadBasicString(TC *buf, size_t buflen)
    {
        size_t readable = GetReadableSize();
        if (readable >= sizeof(TL) + sizeof(TC))
        {
            size_t len = *((TL*)(_data + _rpos));
            if (readable >= len * sizeof(TC) + sizeof(TL) + sizeof(TC))
            {
                if (buflen == 0) return len; // the length of buffer needed

                size_t readlen = len;
                if (readlen >= buflen)
                    readlen = buflen - 1;

                memcpy(buf, _data + _rpos, readlen * sizeof(TC));
                buf[readlen] = (TC)0;

                _rpos += len * sizeof(TC) + sizeof(TL) + sizeof(TC);
                return readlen;
            }
        }

        return 0;
    }

    template<typename TC, typename TL>
    const TC *ReadBasicStringPtr()
    {
        size_t readable = GetReadableSize();
        if (readable >= sizeof(TL) + sizeof(TC))
        {
            size_t len = *((TL*)(_data + _rpos));
            if (readable >= len * sizeof(TC) + sizeof(TL) + sizeof(TC))
            {
                const TC *str = (TC*)(_data + _rpos + sizeof(TL));
                _rpos += len * sizeof(TC) + sizeof(TL) + sizeof(TC);
                return str;
            }
        }

        return nullptr;
    }

    template<typename TC, typename TL>
    void WriteBasicString(const TC *str, size_t len)
    {
        if (str == nullptr)
            str = "";

        if (len == ((size_t)-1))
            len = strlen(str);
        else
            len = MIN(len, strlen(str));

        WriteAtom<TL>((TL)len);
        Write(str, len * sizeof(TC));
        WriteAtom<TC>((TC)0); // the string terminal character
    }

public:
    /**
    * buf - buffer to store data.
    * sz - number of bytes to read.
    */
    size_t Read(void *buf, size_t sz)
    {
        size_t readable = GetReadableSize();
        if (sz > readable)
            return 0;

        if (sz == 0)
            return 0;

        memcpy(buf, _data + _rpos, sz);
        _rpos += sz;

        return sz;
    }

    void Write(const void *buf, size_t sz)
    {
        if (sz == 0)
            return;

        size_t writable = GetWritableSize();
        if (writable < sz)
        {
            Expand(_length + sz);
        }

        memcpy(_data + _wpos, buf, sz);
        _wpos += sz;
    }

    size_t GetReadableSize() const
    {
        return (_wpos > _rpos) ? (_wpos - _rpos) : 0;
    }

    size_t GetWritableSize() const
    {
        return (_length > _wpos) ? (_length - _wpos) : 0;
    }

    size_t GetRpos() const
    {
        return _rpos;
    }

    void SetRpos(size_t rpos)
    {
        if (rpos > _wpos)
            return;

        _rpos = rpos;
    }

    size_t GetWpos() const
    {
        return _wpos;
    }

    void SetWpos(size_t wpos)
    {
        if (wpos >= _wpos)
            return;

        _wpos = wpos;
    }

    char* GetWritePtr() const
    {
        return _data + _wpos;
    }

    void AddWpos(size_t len)
    {
        if (_wpos + len > _length)
        {
            ASSERT(false);
            return;
        }

        _wpos += len;
    }

    char* GetReadPtr() const
    {
        return _data + _rpos;
    }

    void AddRpos(size_t len)
    {
        if (_rpos + len > _wpos)
        {
            ASSERT(false);
            return;
        }

        _rpos += len;
    }

    size_t GetLength()
    {
        return _length;
    }

    void Expand(size_t newLength)
    {
        if (newLength <= _length)
            return;

        char *newData = (char *)malloc(newLength);

        if (_data != nullptr && _length > 0)
        {
            memcpy(newData, _data, _length);
        }

        if (_data != nullptr)
        {
            free(_data);
        }

        _data = newData;
        _length = newLength;
    }

    char* GetData()
    {
        return _data;
    }

private:
    char *_data;
    size_t _length;
    size_t _rpos;
    size_t _wpos;
};

} // namespace vtw
