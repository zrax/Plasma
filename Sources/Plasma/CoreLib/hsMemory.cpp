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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "hsMemory.h"
#include "hsExceptions.h"

#define DO_MEMORY_REPORTS       // dumps memory reports upon start up of engine

///////////////////////////////////////////////////////////////////////////////////////////

void HSMemory::BlockMove(const void* src, void* dst, uint32_t length)
{
    memmove(dst, src, length);
}

bool HSMemory::EqualBlocks(const void* block1, const void* block2, uint32_t length)
{
    const uint8_t* byte1 = (uint8_t*)block1;
    const uint8_t* byte2 = (uint8_t*)block2;

    while (length--)
        if (*byte1++ != *byte2++)
            return false;
    return true;
}

void* HSMemory::New(uint32_t size)
{
    return new uint32_t[(size + 3) >> 2];
}

void HSMemory::Delete(void* block)
{
    delete[] (uint32_t*)block;
}

void* HSMemory::Copy(uint32_t length, const void* source)
{
    void* destination = HSMemory::New(length);

    HSMemory::BlockMove(source, destination, length);
    return destination;
}

void HSMemory::Clear(void* m, uint32_t byteLen)
{
    uint8_t*  mem = (uint8_t*)m;
    uint8_t*  memStop = mem + byteLen;

    if (byteLen > 8)
    {   while (uintptr_t(mem) & 3)
            *mem++ = 0;
        
        uint32_t* mem32 = (uint32_t*)mem;
        uint32_t* mem32Stop = (uint32_t*)(uintptr_t(memStop) & ~3);
        do {
            *mem32++ = 0;
        } while (mem32 < mem32Stop);
        
        mem = (uint8_t*)mem32;
        // fall through to finish any remaining bytes (0..3)
    }
    while (mem < memStop)
        *mem++ = 0;

    hsAssert(mem == memStop, "oops");
}

//////////////////////////////////////////////////////////////////////////////////////

void* HSMemory::SoftNew(uint32_t size)
{
    uint32_t* p;

    hsTry {
        p = new uint32_t[(size + 3) >> 2];
    } hsCatch(...) {
        p = nil;
    }
    return p;
}

//////////////////////////////////////////////////////////////////////////////////////

struct hsPrivateChunk {
    hsPrivateChunk* fNext;
    char*           fAvailableAddr;
    uint32_t          fAvailableSize;

    hsDebugCode(uint32_t  fSize;)
    hsDebugCode(uint32_t  fCount;)

    static hsPrivateChunk* NewPrivateChunk(hsPrivateChunk* next, uint32_t chunkSize);
};

hsPrivateChunk* hsPrivateChunk::NewPrivateChunk(hsPrivateChunk* next, uint32_t chunkSize)
{
    hsPrivateChunk* chunk = (hsPrivateChunk*)HSMemory::New(sizeof(hsPrivateChunk) + chunkSize);

    chunk->fNext            = next;
    chunk->fAvailableAddr   = (char*)chunk + sizeof(hsPrivateChunk);
    chunk->fAvailableSize   = chunkSize;
    hsDebugCode(chunk->fSize = chunkSize;)
    hsDebugCode(chunk->fCount = 0;)

    return chunk;
}

hsChunkAllocator::hsChunkAllocator(uint32_t chunkSize) : fChunkSize(chunkSize), fChunk(nil)
{
    hsDebugCode(fChunkCount = 0;)
}

hsChunkAllocator::~hsChunkAllocator()
{
    this->Reset();
}

void hsChunkAllocator::Reset()
{
    hsPrivateChunk* chunk = fChunk;

    while (chunk)
    {   hsPrivateChunk* next = chunk->fNext;
        HSMemory::Delete(chunk);
        chunk = next;
    }
    fChunk = nil;
    hsDebugCode(fChunkCount = 0;)
}

void hsChunkAllocator::SetChunkSize(uint32_t chunkSize)
{
    fChunkSize = chunkSize;
}

void* hsChunkAllocator::Allocate(uint32_t size, const void* data)
{
    void*   addr;

    if (fChunk == nil || fChunk->fAvailableSize < size)
    {   if (size > fChunkSize)
            fChunkSize = size;
        fChunk = hsPrivateChunk::NewPrivateChunk(fChunk, fChunkSize);
        hsDebugCode(fChunkCount += 1;)
    }

    addr = fChunk->fAvailableAddr;
    fChunk->fAvailableAddr += size;
    fChunk->fAvailableSize -= size;
    hsDebugCode(fChunk->fCount += 1;)

    if (data)
        HSMemory::BlockMove(data, addr, size);

    return addr;
}

void* hsChunkAllocator::SoftAllocate(uint32_t size, const void* data)
{
    void*   addr;

    hsTry {
        addr = this->Allocate(size, data);
    }
    hsCatch(...) {
        addr = nil;
    }
    return addr;
}

//////////////////////////////////////////////////////////////////////////////////////

struct hsAppenderHead {
    struct hsAppenderHead*  fNext;
    struct hsAppenderHead*  fPrev;
    void*   fFirst;
    void*   fStop;
    void*   fBottom;
    
    void*   GetTop() const { return (char*)this + sizeof(*this); }
    void*   GetBottom() const { return fBottom; }
    void*   GetStop() const { return fStop; }

    void*    GetFirst() const { return fFirst; }
    void*    GetLast(uint32_t elemSize) const { return (char*)fStop - elemSize; }
    uint32_t GetSize() const { return (char*)fStop - (char*)fFirst; }

    bool    CanPrepend() const { return fFirst != this->GetTop(); }
    int     PrependSize() const { return (char*)fFirst - (char*)this->GetTop(); }
    bool    CanAppend() const { return fStop != this->GetBottom(); }
    int     AppendSize() const { return (char*)this->GetBottom() - (char*)fStop; }
    
    void* Prepend(uint32_t elemSize)
    {
        hsAssert(this->CanPrepend(), "bad prepend");
        fFirst = (char*)fFirst - elemSize;
        hsAssert((char*)fFirst >= (char*)this->GetTop(), "bad elemSize");
        return fFirst;
    }
    void* Append(uint32_t elemSize)
    {
        hsAssert(this->CanAppend(), "bad append");
        void* data = fStop;
        fStop = (char*)fStop + elemSize;
        hsAssert((char*)fStop <= (char*)fBottom, "bad elemSize");
        return data;
    }
    bool PopHead(uint32_t elemSize, void* data)
    {
        hsAssert(fFirst != fStop, "Empty");
        if( data )
            HSMemory::BlockMove(fFirst, data, elemSize);
        fFirst = (char*)fFirst + elemSize;
        return fFirst == fStop;
    }
    bool PopTail(uint32_t elemSize, void* data)
    {
        hsAssert(fFirst != fStop, "Empty");
        fStop = (char*)fStop - elemSize;
        if( data )
            HSMemory::BlockMove(fStop, data, elemSize);
        return fFirst == fStop;
    }

    static hsAppenderHead* NewAppend(uint32_t elemSize, uint32_t elemCount, hsAppenderHead* prev)
    {
        uint32_t          dataSize = elemSize * elemCount;
         hsAppenderHead*    head = (hsAppenderHead*)HSMemory::New(sizeof(hsAppenderHead) + dataSize);

        head->fNext = nil;
        head->fPrev = prev;
        head->fFirst    = head->GetTop();
        head->fStop = head->fFirst;
        head->fBottom   = (char*)head->fFirst + dataSize;
        return head;
    }
    static hsAppenderHead* NewPrepend(uint32_t elemSize, uint32_t elemCount, hsAppenderHead* next)
    {
        uint32_t          dataSize = elemSize * elemCount;
         hsAppenderHead*    head = (hsAppenderHead*)HSMemory::New(sizeof(hsAppenderHead) + dataSize);

        head->fNext = next;
        head->fPrev = nil;
        head->fBottom   = (char*)head->GetTop() + dataSize;
        head->fFirst    = head->fBottom;
        head->fStop = head->fBottom;
        return head;
    }
};

////////////////////////////////////////////////////////////////////////////////////////

hsAppender::hsAppender(uint32_t elemSize, uint32_t elemCount)
        : fFirstBlock(nil), fElemSize(elemSize), fElemCount(elemCount), fCount(0)
{
}

hsAppender::~hsAppender()
{
    this->Reset();
}

uint32_t hsAppender::CopyInto(void* data) const
{
    if (data)
    {   const hsAppenderHead*   head = fFirstBlock;
        hsDebugCode(uint32_t totalSize = 0;)

        while (head != nil)
        {   uint32_t  size = head->GetSize();
            HSMemory::BlockMove(head->GetFirst(), data, size);
            
            data = (char*)data + size;
            head = head->fNext;
            hsDebugCode(totalSize += size;)
        }
        hsAssert(totalSize == fCount * fElemSize, "bad size");
    }
    return fCount * fElemSize;
}

void hsAppender::Reset()
{
    hsAppenderHead* head = fFirstBlock;

    while (head != nil)
    {   hsAppenderHead* next = head->fNext;
        HSMemory::Delete(head);
        head = next;
    }

    fCount = 0;
    fFirstBlock = nil;
    fLastBlock = nil;
}

void* hsAppender::PushHead()
{
    if (fFirstBlock == nil)
    {   fFirstBlock = hsAppenderHead::NewPrepend(fElemSize, fElemCount, nil);
        fLastBlock      = fFirstBlock;
    }
    else if (fFirstBlock->CanPrepend() == false)
        fFirstBlock = hsAppenderHead::NewPrepend(fElemSize, fElemCount, fFirstBlock);

    fCount += 1;
    return fFirstBlock->Prepend(fElemSize);
}

void hsAppender::PushHead(const void* data)
{
    void*   addr = this->PushHead();
    if (data)
        HSMemory::BlockMove(data, addr, fElemSize);
}

void* hsAppender::PeekHead() const
{
    if (fFirstBlock)
        return (char*)fFirstBlock->fFirst;
    else
        return nil;
}

bool hsAppender::PopHead(void* data)
{
    if (fCount == 0)
        return false;

    fCount -= 1;

    if (fFirstBlock->PopHead(fElemSize, data))
    {   hsAppenderHead* next = fFirstBlock->fNext;
        if (next)
            next->fPrev = nil;
        HSMemory::Delete(fFirstBlock);
        fFirstBlock = next;
        if (next == nil)
            fLastBlock = nil;
    }
    return true;
}

int hsAppender::PopHead(int count, void* data)
{
    hsThrowIfBadParam(count >= 0);

    int sizeNeeded = count * fElemSize;
    int origCount = fCount;

    while (fCount > 0)
    {   int size = fFirstBlock->GetSize();
        if (size > sizeNeeded)
            size = sizeNeeded;

        if (fFirstBlock->PopHead(size, data))
        {   hsAppenderHead* next = fFirstBlock->fNext;
            if (next)
                next->fPrev = nil;
            HSMemory::Delete(fFirstBlock);
            fFirstBlock = next;
            if (next == nil)
                fLastBlock = nil;
        }

        if (data)
            data = (void*)((char*)data + size);
        sizeNeeded -= size;
        fCount -= size / fElemSize;
        hsAssert(int(fCount) >= 0, "bad fElemSize");
    }
    return origCount - fCount;      // return number of elements popped
}

void* hsAppender::PushTail()
{
    if (fFirstBlock == nil)
    {   fFirstBlock = hsAppenderHead::NewAppend(fElemSize, fElemCount, nil);
        fLastBlock      = fFirstBlock;
    }
    else if (fLastBlock->CanAppend() == false)
    {   fLastBlock->fNext   = hsAppenderHead::NewAppend(fElemSize, fElemCount, fLastBlock);
        fLastBlock      = fLastBlock->fNext;
    }
    
    fCount += 1;
    return fLastBlock->Append(fElemSize);
}

void hsAppender::PushTail(const void* data)
{
    void*   addr = this->PushTail();
    if (data)
        HSMemory::BlockMove(data, addr, fElemSize);
}

void hsAppender::PushTail(int count, const void* data)
{
    hsThrowIfBadParam(count < 0);

    int sizeNeeded = count * fElemSize;

    while (sizeNeeded > 0)
    {   if (fFirstBlock == nil)
        {   hsAssert(fCount == 0, "uninited count");
            fFirstBlock = hsAppenderHead::NewAppend(fElemSize, fElemCount, nil);
            fLastBlock      = fFirstBlock;
        }
        else if (fLastBlock->CanAppend() == false)
        {   fLastBlock->fNext   = hsAppenderHead::NewAppend(fElemSize, fElemCount, fLastBlock);
            fLastBlock      = fLastBlock->fNext;
        }

        int     size = fLastBlock->AppendSize();
        hsAssert(size > 0, "bad appendsize");
        if (size > sizeNeeded)
            size = sizeNeeded;
        void*   dst = fLastBlock->Append(size);

        if (data)
        {   HSMemory::BlockMove(data, dst, size);
            data = (char*)data + size;
        }
        sizeNeeded -= size;
    }
    fCount += count;
}

void* hsAppender::PeekTail() const
{
    if (fLastBlock)
        return (char*)fLastBlock->fStop - fElemSize;
    else
        return nil;
}

bool hsAppender::PopTail(void* data)
{
    if (fCount == 0)
        return false;

    fCount -= 1;

    if (fLastBlock->PopTail(fElemSize, data))
    {   hsAppenderHead* prev = fLastBlock->fPrev;
        if (prev)
            prev->fNext = nil;
        HSMemory::Delete(fLastBlock);
        fLastBlock = prev;
        if (prev == nil)
            fFirstBlock = nil;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////

hsAppenderIterator::hsAppenderIterator(const hsAppender* list)
{
    this->ResetToHead(list);
}
            
void hsAppenderIterator::ResetToHead(const hsAppender* list)
{
    fAppender = list;
    fCurrBlock = nil;

    if (fAppender)
    {   fCurrBlock = fAppender->fFirstBlock;
        if (fCurrBlock)
            fCurrItem = fCurrBlock->GetFirst();
    }
}

void hsAppenderIterator::ResetToTail(const hsAppender* list)
{
    fAppender = list;
    fCurrBlock = nil;

    if (fAppender)
    {   fCurrBlock = fAppender->fLastBlock;
        if (fCurrBlock)
            fCurrItem = fCurrBlock->GetLast(fAppender->fElemSize);
    }
}

void* hsAppenderIterator::Next()
{
    void*   item = nil;

    if (fCurrBlock)
    {   item = fCurrItem;
        fCurrItem = (char*)fCurrItem + fAppender->fElemSize;
        if (fCurrItem == fCurrBlock->GetBottom())
        {   fCurrBlock = fCurrBlock->fNext;
            if (fCurrBlock)
                fCurrItem = fCurrBlock->GetFirst();
        }
        else if (fCurrItem == fCurrBlock->GetStop())
        {   hsAssert(fCurrBlock->fNext == nil, "oops");
            fCurrBlock = nil;
        }
    }
    return item;
}

bool hsAppenderIterator::Next(void* data)
{
    void*   addr = this->Next();
    if (addr)
    {   if (data)
            HSMemory::BlockMove(addr, data, fAppender->fElemSize);
        return true;
    }
    return false;
}

int hsAppenderIterator::Next(int count, void* data)
{
    int origCount = count;
    
    while (count > 0 && this->Next(data))
    {   if (data)
            data = (void*)((char*)data + fAppender->fElemSize);
        count -= 1;
    }
    return origCount - count;
}

void* hsAppenderIterator::Prev()
{
    void*   item = nil;

    if (fCurrBlock)
    {   item = fCurrItem;
        fCurrItem = (char*)fCurrItem - fAppender->fElemSize;
        if (item == fCurrBlock->GetTop())
        {   fCurrBlock = fCurrBlock->fPrev;
            if (fCurrBlock)
                fCurrItem = fCurrBlock->GetLast(fAppender->fElemSize);
        }
        else if (item == fCurrBlock->GetFirst())
        {   hsAssert(fCurrBlock->fPrev == nil, "oops");
            fCurrBlock = nil;
        }
    }
    return item;
}

bool hsAppenderIterator::Prev(void* data)
{
    void*   addr = this->Prev();
    if (addr)
    {   if (data)
            HSMemory::BlockMove(addr, data, fAppender->fElemSize);
        return true;
    }
    return false;
}
