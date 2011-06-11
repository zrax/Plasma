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

#include "plString.h"

#include <cstring>

const plString plString::Null;

#if !defined(WCHAR_BYTES) || (WCHAR_BYTES != 2) && (WCHAR_BYTES != 4)
#error "WCHAR_BYTES must be either 2 (16-bit) or 4 (32-bit)!"
#endif

#if WCHAR_BYTES == 2
#define u16slen(str) wcslen((const wchar_t *)(str))
#else
static inline size_t u16slen(const UInt16 *ustr)
{
    size_t length = 0;
    for ( ; *ustr++; ++length)
        ;
    return length;
}
#endif

#define BADCHAR_REPLACEMENT (0xFFFDul)

void plString::IConvertFromUtf8(const char *utf8, size_t size, bool steal)
{
    if (size == kSizeAuto)
        size = strlen(utf8);

#ifdef _DEBUG
    // Check to make sure the string is actually valid UTF-8
    const char *sp = utf8;
    while (sp < utf8 + size) {
        unsigned char unichar = *sp++;
        if ((unichar & 0xF8) == 0xF0) {
            // Four bytes
            hsAssert((*sp++) & 0x80, "Invalid UTF-8 sequence byte (1)");
            hsAssert((*sp++) & 0x80, "Invalid UTF-8 sequence byte (2)");
            hsAssert((*sp++) & 0x80, "Invalid UTF-8 sequence byte (3)");
        } else if ((unichar & 0xF0) == 0xE0) {
            // Three bytes
            hsAssert((*sp++) & 0x80, "Invalid UTF-8 sequence byte (1)");
            hsAssert((*sp++) & 0x80, "Invalid UTF-8 sequence byte (2)");
        } else if ((unichar & 0xE0) == 0xC0) {
            // Two bytes
            hsAssert((*sp++) & 0x80, "Invalid UTF-8 sequence byte (1)");
        } else if ((unichar & 0xC0) == 0x80) {
            hsAssert(0, "Invalid UTF-8 marker byte");
        } else if ((unichar & 0x80) != 0) {
            hsAssert(0, "UTF-8 character out of range");
        }
    }
#endif

    fUtf8Buffer = steal ? plStringBuffer<char>::Steal(utf8, size)
                        : plStringBuffer<char>(utf8, size);
}

void plString::IConvertFromUtf16(const UInt16 *utf16, size_t size)
{
    if (size == kSizeAuto)
        size = u16slen(utf16);

    // Calculate the UTF-8 size
    size_t convlen = 0;
    const UInt16 *sp = utf16;
    while (sp < utf16 + size) {
        if (*sp >= 0xD800 && *sp <= 0xDFFF) {
            // Surrogate pair
            convlen += 4;
            ++sp;
        }
        else if (*sp > 0x7FF)
            convlen += 3;
        else if (*sp > 0x7F)
            convlen += 2;
        else
            convlen += 1;
        ++sp;
    }

    // And perform the actual conversion
    char *utf8 = TRACKED_NEW char[convlen + 1];
    char *dp = utf8;
    sp = utf16;
    while (sp < utf16 + size) {
        if (*sp >= 0xD800 && *sp <= 0xDFFF) {
            // Surrogate pair
            unsigned int unichar = 0x10000;

            if (sp + 1 >= utf16 + size) {
                hsAssert(0, "Incomplete surrogate pair in UTF-16 data");
                unichar = BADCHAR_REPLACEMENT;
            } else if (*sp < 0xDC00) {
                unichar += (*sp++ & 0x3FF) << 10;
                hsAssert(*sp >= 0xDC00 && *sp <= 0xDFFF,
                         "Invalid surrogate pair in UTF-16 data");
                unichar += (*sp   & 0x3FF);
            } else {
                unichar += (*sp++ & 0x3FF);
                hsAssert(*sp >= 0xD800 && *sp <  0xDC00,
                         "Invalid surrogate pair in UTF-16 data");
                unichar += (*sp   & 0x3FF) << 10;
            }
            *dp++ = 0xF0 | ((unichar >> 18) & 0x07);
            *dp++ = 0x80 | ((unichar >> 12) & 0x3F);
            *dp++ = 0x80 | ((unichar >>  6) & 0x3F);
            *dp++ = 0x80 | ((unichar      ) & 0x3F);
        } else if (*sp > 0x7FF) {
            *dp++ = 0xF0 | ((*sp >> 12) & 0x0F);
            *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else if (*sp > 0x7F) {
            *dp++ = 0xF0 | ((*sp >>  6) & 0x1F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else {
            *dp++ = (char)(*sp);
        }
        ++sp;
    }
    utf8[convlen] = 0;

    fUtf8Buffer = plStringBuffer<char>::Steal(utf8, convlen);
}

void plString::IConvertFromWchar(const wchar_t *wstr, size_t size)
{
#if WCHAR_BYTES == 2
    // We assume that if sizeof(wchar_t) == 2, the data is UTF-16 already
    IConvertFromUtf16((const UInt16 *)wstr, size);
#else
    if (size == kSizeAuto)
        size = wcslen(wstr);

    // Calculate the UTF-8 size
    size_t convlen = 0;
    const wchar_t *sp = wstr;
    while (sp < wstr + size) {
        if (*sp > 0x10FFFF) {
            hsAssert(0, "UCS-4 character out of range");
            convlen += 3;   // Use U+FFFD for release builds
        }
        else if (*sp > 0xFFFF)
            convlen += 4;
        else if (*sp > 0x7FF)
            convlen += 3;
        else if (*sp > 0x7F)
            convlen += 2;
        else
            convlen += 1;
        ++sp;
    }

    // And perform the actual conversion
    char *utf8 = TRACKED_NEW char[convlen + 1];
    char *dp = utf8;
    sp = wstr;
    while (sp < wstr + size) {
        if (*sp > 0x10FFFF) {
            // Character out of range; Use U+FFFD instead
            *dp++ = 0xE0 | ((BADCHAR_REPLACEMENT >> 12) & 0x0F);
            *dp++ = 0x80 | ((BADCHAR_REPLACEMENT >>  6) & 0x3F);
            *dp++ = 0x80 | ((BADCHAR_REPLACEMENT      ) & 0x3F);
        } else if (*sp > 0xFFFF) {
            *dp++ = 0xF0 | ((*sp >> 18) & 0x07);
            *dp++ = 0x80 | ((*sp >> 12) & 0x3F);
            *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else if (*sp > 0x7FF) {
            *dp++ = 0xF0 | ((*sp >> 12) & 0x0F);
            *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else if (*sp > 0x7F) {
            *dp++ = 0xF0 | ((*sp >>  6) & 0x1F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else {
            *dp++ = (char)(*sp);
        }
        ++sp;
    }
    utf8[convlen] = 0;

    fUtf8Buffer = plStringBuffer<char>::Steal(utf8, convlen);
#endif
}

void plString::IConvertFromAscii(const char *astr, size_t size)
{
    if (size == kSizeAuto)
        size = strlen(astr);

    // Calculate the UTF-8 size
    size_t convlen = 0;
    const char *sp = astr;
    while (sp < astr + size) {
        if ((*sp++) & 0x80)
            convlen += 2;
        else
            convlen += 1;
    }

    // And perform the actual conversion
    char *utf8 = TRACKED_NEW char[convlen + 1];
    char *dp = utf8;
    sp = astr;
    while (sp < astr + size) {
        if (*astr & 0x80) {
            *dp++ = 0xC0 | ((*sp >> 6) & 0x1F);
            *dp++ = 0x80 | ((*sp     ) & 0x3F);
        } else {
            *dp++ = *sp;
        }
        ++sp;
    }
    utf8[convlen] = 0;

    fUtf8Buffer = plStringBuffer<char>::Steal(utf8, convlen);
}

plStringBuffer<UInt16> plString::ToUtf16() const
{
    if (IsNull())
        return plStringBuffer<UInt16>();

    // Calculate the UTF-16 size
    size_t convlen = 0;
    const char *utf8 = fUtf8Buffer.GetData();
    const char *sp = utf8;
    size_t srcSize = fUtf8Buffer.GetSize();
    while (sp < utf8 + srcSize) {
        if ((*sp & 0xF8) == 0xF0) {
            // Will require a surrogate pair
            ++convlen;
            sp += 4;
        }
        else if ((*sp & 0xF0) == 0xE0) 
            sp += 3;
        else if ((*sp & 0xE0) == 0xC0)
            sp += 2;
        else
            sp += 1;
        ++convlen;
    }

    // And perform the actual conversion
    UInt16 *ustr = TRACKED_NEW UInt16[convlen + 1];
    UInt16 *dp = ustr;
    sp = utf8;
    while (sp < utf8 + srcSize) {
        unsigned int unichar;
        if ((*sp & 0xF8) == 0xF0) {
            unichar  = (*sp++ & 0x07) << 18;
            unichar |= (*sp++ & 0x3F) << 12;
            unichar |= (*sp++ & 0x3F) << 6;
            unichar |= (*sp++ & 0x3F);

            *dp++ = 0xD800 | ((unichar >> 10) & 0x3FF);
            *dp++ = 0xDC00 | ((unichar      ) & 0x3FF);
        } else if ((*sp & 0xF0) == 0xE0) {
            unichar  = (*sp++ & 0x0F) << 12;
            unichar |= (*sp++ & 0x3F) << 6;
            unichar |= (*sp++ & 0x3F);
            *dp++ = unichar;
        } else if ((*sp & 0xE0) == 0xC0) {
            unichar  = (*sp++ & 0x1F) << 6;
            unichar |= (*sp++ & 0x3F);
            *dp++ = unichar;
        } else {
            *dp++ = *sp++;
        }
    }
    ustr[convlen] = 0;

    return plStringBuffer<UInt16>::Steal(ustr, convlen);
}

plStringBuffer<wchar_t> plString::ToWchar() const
{
#if WCHAR_BYTES == 2
    // We assume that if sizeof(wchar_t) == 2, the data is UTF-16 already
    plStringBuffer<UInt16> utf16 = ToUtf16();
    return *reinterpret_cast<plStringBuffer<wchar_t>*>(&utf16);
#else
    if (IsNull())
        return plStringBuffer<wchar_t>();

    // Calculate the UCS-4 size
    size_t convlen = 0;
    const char *utf8 = fUtf8Buffer.GetData();
    const char *sp = utf8;
    size_t srcSize = fUtf8Buffer.GetSize();
    while (sp < utf8 + srcSize) {
        if ((*sp & 0xF8) == 0xF0)
            sp += 4;
        else if ((*sp & 0xF0) == 0xE0)
            sp += 3;
        else if ((*sp & 0xE0) == 0xC0)
            sp += 2;
        else
            sp += 1;
        ++convlen;
    }

    // And perform the actual conversion
    wchar_t *wstr = TRACKED_NEW wchar_t[convlen + 1];
    wchar_t *dp = wstr;
    sp = utf8;
    while (sp < utf8 + srcSize) {
        unsigned int unichar;
        if ((*sp & 0xF8) == 0xF0) {
            unichar  = (*sp++ & 0x07) << 18;
            unichar |= (*sp++ & 0x3F) << 12;
            unichar |= (*sp++ & 0x3F) << 6;
            unichar |= (*sp++ & 0x3F);
        } else if ((*sp & 0xF0) == 0xE0) {
            unichar  = (*sp++ & 0x0F) << 12;
            unichar |= (*sp++ & 0x3F) << 6;
            unichar |= (*sp++ & 0x3F);
        } else if ((*sp & 0xE0) == 0xC0) {
            unichar  = (*sp++ & 0x1F) << 6;
            unichar |= (*sp++ & 0x3F);
        } else {
            unichar = *sp++;
        }
        *dp++ = unichar;
    }
    wstr[convlen] = 0;

    return plStringBuffer<wchar_t>::Steal(wstr, convlen);
#endif
}

plStringBuffer<char> plString::ToAscii() const
{
    if (IsNull())
        return plStringBuffer<char>();

    // Calculate the ASCII size
    size_t convlen = 0;
    const char *utf8 = fUtf8Buffer.GetData();
    const char *sp = utf8;
    size_t srcSize = fUtf8Buffer.GetSize();
    while (sp < utf8 + srcSize) {
        if ((*sp & 0xF8) == 0xF0)
            sp += 4;
        else if ((*sp & 0xF0) == 0xE0)
            sp += 3;
        else if ((*sp & 0xE0) == 0xC0)
            sp += 2;
        else
            sp += 1;
        ++convlen;
    }

    // And perform the actual conversion
    char *astr = TRACKED_NEW char[convlen + 1];
    char *dp = astr;
    sp = utf8;
    while (sp < utf8 + srcSize) {
        unsigned int unichar;
        if ((*sp & 0xF8) == 0xF0) {
            unichar  = (*sp++ & 0x07) << 18;
            unichar |= (*sp++ & 0x3F) << 12;
            unichar |= (*sp++ & 0x3F) << 6;
            unichar |= (*sp++ & 0x3F);
        } else if ((*sp & 0xF0) == 0xE0) {
            unichar  = (*sp++ & 0x0F) << 12;
            unichar |= (*sp++ & 0x3F) << 6;
            unichar |= (*sp++ & 0x3F);
        } else if ((*sp & 0xE0) == 0xC0) {
            unichar  = (*sp++ & 0x1F) << 6;
            unichar |= (*sp++ & 0x3F);
        } else {
            unichar = *sp++;
        }
        *dp++ = (unichar < 0xFF) ? unichar : '?';
    }
    astr[convlen] = 0;

    return plStringBuffer<char>::Steal(astr, convlen);
}

// Microsoft doesn't provide this for us
#ifdef _MSC_VER
#define va_copy(dest, src)  (dest) = (src)
#endif

static plString IFormat(const char *fmt, va_list vptr)
{
    char buffer[256];
    va_list vptr_save;
    va_copy(vptr_save, vptr);

    int chars = vsnprintf(buffer, 256, fmt, vptr);
    if (chars < 0) {
        // We will need to try this multiple times until we get a
        // large enough buffer :(
        int size = 4096;
        for ( ;; ) {
            va_copy(vptr, vptr_save);
            char *bigbuffer = TRACKED_NEW char[size];
            chars = vsnprintf(bigbuffer, size, fmt, vptr);
            if (chars >= 0)
                return plString::Steal(bigbuffer);

            delete [] bigbuffer;
            size *= 2;
        }
    } else if (chars >= 256) {
        va_copy(vptr, vptr_save);
        char *bigbuffer = TRACKED_NEW char[chars+1];
        vsnprintf(bigbuffer, chars+1, fmt, vptr);
        return plString::Steal(bigbuffer);
    }

    return plString::FromUtf8(buffer);
}

plString plString::Format(const char *fmt, ...)
{
    va_list vptr;
    va_start(vptr, fmt);
    plString str = IFormat(fmt, vptr);
    va_end(vptr);
    return str;
}

int plString::Find_i(char ch) const
{
    // No need to check for null, since s_str() will return { 0 } if it is null
    const char *cp = s_str();
    while (*cp) {
        if (tolower(*cp) == tolower(ch))
            return cp - c_str();
    }
    return -1;
}

int plString::Find_ri(char ch) const
{
    if (IsEmpty())
        return -1;

    const char *cp = c_str();
    cp += strlen(cp);

    while (--cp >= c_str()) {
        if (tolower(*cp) == tolower(ch))
            return cp - c_str();
    }
    return -1;
}

static bool in_set(char key, const char *charset)
{
    for (const char *cs = charset; *cs; ++cs) {
        if (*cs == key)
            return true;
    }
    return false;
}

plString plString::TrimLeft(const char *charset) const
{
    if (IsEmpty())
        return Null;

    const char *cp = c_str();
    while (*cp && in_set(*cp, charset))
        ++cp;

    return Substr(cp - c_str());
}

plString plString::TrimRight(const char *charset) const
{
    if (IsEmpty())
        return Null;

    const char *cp = c_str();
    cp += strlen(cp);

    while (--cp >= c_str() && in_set(*cp, charset))
        ;

    return Substr(0, cp - c_str() + 1);
}

plString plString::Trim(const char *charset) const
{
    if (IsEmpty())
        return Null;

    const char *lp = c_str();
    const char *rp = lp + strlen(lp);

    while (*lp && in_set(*lp, charset))
        ++lp;
    while (--rp >= lp && in_set(*rp, charset))
        ;

    return Substr(lp - c_str(), rp - lp + 1);
}

plString plString::Substr(int start, size_t size) const
{
    size_t maxSize = GetSize();

    if (start > maxSize)
        return Null;
    if (start < 0)
        start = 0;
    if (start + size > maxSize)
        size = maxSize - start;

    if (start == 0 && size == maxSize)
        return *this;

    char *substr = TRACKED_NEW char[size + 1];
    memcpy(substr, c_str() + start, size);
    substr[size] = 0;

    // Don't re-check UTF-8 on this
    plString str;
    str.fUtf8Buffer.Steal(substr, size);
    return str;
}

plString &plString::operator+=(const plString &str)
{
    size_t catsize = GetSize() + str.GetSize();
    char *catstr = TRACKED_NEW char[catsize + 1];
    memcpy(catstr, s_str(), GetSize());
    memcpy(catstr + GetSize(), str.s_str(), str.GetSize());
    catstr[catsize] = 0;
    fUtf8Buffer.Steal(catstr, catsize);
    return *this;
}

plString operator+(const plString &left, const plString &right)
{
    size_t catsize = left.GetSize() + right.GetSize();
    char *catstr = TRACKED_NEW char[catsize + 1];
    memcpy(catstr, left.s_str(), left.GetSize());
    memcpy(catstr + left.GetSize(), right.s_str(), right.GetSize());
    catstr[catsize] = 0;

    // Don't re-check UTF-8 on this
    plString str;
    str.fUtf8Buffer.Steal(catstr, catsize);
    return str;
}

plStringStream& plStringStream::Add(const char *text)
{
    size_t length = strlen(text);
    if (fLength + length + 1 > fBufSize) {
        char *bigger = new char[fBufSize * 2];
        memcpy(bigger, fBuffer, fBufSize);
        delete [] fBuffer;
        fBuffer = bigger;
        fBufSize *= 2;
    }
    memcpy(fBuffer + fLength, text, length);
    fLength += length;
    return *this;
}

plStringStream& plStringStream::Add(Int32 num)
{
    char buffer[12];
    snprintf(buffer, 12, "%ld", num);
    return Add(buffer);
}

plStringStream& plStringStream::Add(UInt32 num)
{
    char buffer[12];
    snprintf(buffer, 12, "%lu", num);
    return Add(buffer);
}
