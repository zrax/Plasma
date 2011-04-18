/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#ifndef plString_Defined
#define plString_Defined

#include "hsTypes.h"
#include <stddef.h>

template <typename _Ch>
class plStringBuffer
{
private:
    struct StringRef
    {
        unsigned int fRefs;
        const _Ch *fStringData;
        const size_t fSize;

        StringRef(const _Ch *data, const size_t size)
            : fRefs(1), fStringData(data), fSize(size) { }

        inline void AddRef() { ++fRefs; }
        inline void DecRef()
        {
            if (--fRefs == 0) {
                delete [] fStringData;
                delete this;
            }
        }
    };

    StringRef *fData;

public:
    plStringBuffer() : fData(0) { }

    plStringBuffer(const plStringBuffer<_Ch> &copy)
    {
        fData = copy.fData;
        if (fData)
            fData->AddRef();
    }
    
    plStringBuffer(const _Ch *data, size_t size)
    {
        _Ch *copyData = new _Ch[size + 1];
        memcpy(copyData, data, size);
        copyData[size] = 0;

        fData = new StringRef(copyData, size);
    }

    ~plStringBuffer<_Ch>()
    {
        if (fData)
            fData->DecRef();
    }

    static plStringBuffer<_Ch> Steal(const _Ch *data, size_t size)
    {
        plStringBuffer<_Ch> string;
        string.fData = new StringRef(data, size);
        return string;
    }

    plStringBuffer<_Ch> &operator=(const plStringBuffer<_Ch> &copy)
    {
        if (copy.fData)
            copy.fData->AddRef();
        if (fData)
            fData->DecRef();
        fData = copy.fData;
        return *this;
    }

    const _Ch *GetData() const { return fData ? fData->fStringData : 0; }
    const size_t GetSize() const { return fData ? fData->fSize : 0; }
};


class plString
{
    enum {
        kSizeAuto = (size_t)(-1)
    };

private:
    plStringBuffer<char> fUtf8Buffer;

    void IConvertFromUtf8(const char *utf8, size_t size);
    void IConvertFromUtf16(const UInt16 *utf16, size_t size);
    void IConvertFromWchar(const wchar_t *wstr, size_t size);
    void IConvertFromAscii(const char *astr, size_t size);

public:
    plString() { }

    plString(const char *utf8) { IConvertFromUtf8(utf8, kSizeAuto); }
    plString(const wchar_t *wstr) { IConvertFromWchar(wstr, kSizeAuto); }

    static inline plString FromUtf8(const char *utf8, size_t size = kSizeAuto)
    {
        plString str;
        str.IConvertFromUtf8(utf8, size);
        return str;
    }

    static inline plString FromUtf16(const UInt16 *utf16, size_t size = kSizeAuto)
    {
        plString str;
        str.IConvertFromUtf16(utf16, size);
        return str;
    }

    static inline plString FromWchar(const wchar_t *wstr, size_t size = kSizeAuto)
    {
        plString str;
        str.IConvertFromWchar(wstr, size);
        return str;
    }

    static inline plString FromAscii(const char *astr, size_t size = kSizeAuto)
    {
        plString str;
        str.IConvertFromAscii(astr, size);
        return str;
    }

    const char *c_str() const { return fUtf8Buffer.GetData(); }
    plStringBuffer<char> ToUtf8() const { return fUtf8Buffer; }
    plStringBuffer<UInt16> ToUtf16() const;
    plStringBuffer<wchar_t> ToWchar() const;
    plStringBuffer<char> ToAscii() const;

    size_t GetSize() const { return fUtf8Buffer.GetSize(); }
    bool IsEmpty() const { return fUtf8Buffer.GetSize() == 0; }
    bool IsNull() const { return fUtf8Buffer.GetData() == 0; }
};

#endif //plString_Defined
