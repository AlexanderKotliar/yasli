/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <cstddef>

namespace yasli{

class MemoryReader{
public:

    MemoryReader();
    explicit MemoryReader(const char* fileName);
    MemoryReader(const void* memory, size_t size, bool ownAndFree = false);
    ~MemoryReader();

    void setPosition(const char* position);
    const char* position(){ return position_; }

    template<class T>
    void read(T& value){
        read(reinterpret_cast<void*>(&value), sizeof(value));
    }
    void read(void* data, size_t size);
    bool checkedSkip(size_t size);
    bool checkedRead(void* data, size_t size);
    template<class T>
    bool checkedRead(T& t){
        return checkedRead((void*)&t, sizeof(t));
    }

    const char* buffer() const{ return memory_; }
    size_t size() const{ return size_; }

    const char* begin() const{ return memory_; }
    const char* end() const{ return memory_ + size_; }
private:
    size_t size_ = 0;
    const char* position_;
    const char* memory_;
    bool ownedMemory_;
};

}
// vim:ts=4 sw=4:
