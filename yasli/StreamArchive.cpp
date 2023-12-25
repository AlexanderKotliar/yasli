/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "StreamArchive.h"

using namespace std;

namespace yasli{

static const unsigned char SIZE16 = 254;
static const unsigned char SIZE32 = 255;

static const unsigned int STREAM_MAGIC = 0xb1a4c16f;

StreamOArchive::StreamOArchive()
: Archive(OUTPUT | BINARY)
{
    clear();
}

void StreamOArchive::clear()
{
    writer_.clear();
    writer_.write((const char*)&STREAM_MAGIC, sizeof(STREAM_MAGIC));
}

bool StreamOArchive::save(const char* filename)
{
  FILE* f = fopen(filename, "wb");
  if (!f)
    return false;

  if (fwrite(buffer(), 1, length(), f) != length())
  {
    fclose(f);
    return false;
  }

  fclose(f);
  return true;
}

bool StreamOArchive::operator()(bool& value, const char* name, const char* label)
{
	writer_.write(value);
	return true;
}

bool StreamOArchive::operator()(StringInterface& value, const char* name, const char* label)
{
  writer_ << value.get();
  writer_.write(char(0));
  return true;
}

bool StreamOArchive::operator()(WStringInterface& value, const char* name, const char* label)
{
	writer_ << value.get();
	writer_.write(wchar_t(0));
	return true;
}

bool StreamOArchive::operator()(float& value, const char* name, const char* label)
{
  writer_.write(value);
  return true;
}

bool StreamOArchive::operator()(double& value, const char* name, const char* label)
{
  writer_.write(value);
  return true;
}

bool StreamOArchive::operator()(i16& value, const char* name, const char* label)
{
  writer_.write(value);
  return true;
}

bool StreamOArchive::operator()(i8& value, const char* name, const char* label)
{
  writer_.write(value);
  return true;
}

bool StreamOArchive::operator()(u8& value, const char* name, const char* label)
{
  writer_.write(value);
  return true;
}

bool StreamOArchive::operator()(char& value, const char* name, const char* label)
{
  writer_.write(value);
  return true;
}

bool StreamOArchive::operator()(u16& value, const char* name, const char* label)
{
  writer_.write(value);
  return true;
}

bool StreamOArchive::operator()(i32& value, const char* name, const char* label)
{
  writer_.write(value);
  return true;
}

bool StreamOArchive::operator()(u32& value, const char* name, const char* label)
{
  writer_.write(value);
  return true;
}

bool StreamOArchive::operator()(i64& value, const char* name, const char* label)
{
  writer_.write(value);
  return true;
}

bool StreamOArchive::operator()(u64& value, const char* name, const char* label)
{
  writer_.write(value);
  return true;
}

bool StreamOArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
  ser(*this);
  return true;
}

bool StreamOArchive::operator()(ContainerInterface& ser, const char* name, const char* label)
{
  unsigned int size = (unsigned int)ser.size();
  if (size < SIZE16)
    writer_.write((unsigned char)size);
  else if (size < 0x10000) {
    writer_.write(SIZE16);
    writer_.write((unsigned short)size);
  }
  else {
    writer_.write(SIZE32);
    writer_.write(size);
  }

  if (size > 0) {
    do
      ser(*this, "", "");
    while (ser.next());
  }

  return true;
}

bool StreamOArchive::operator()(PointerInterface& ptr, const char* name, const char* label)
{
  TypeID derived = ptr.type();
  const TypeDescription* desc = 0;
  const char* typeName = "";
  if (derived) {
    desc = ptr.factory()->descriptionByType(derived);
    typeName = desc->name();
    YASLI_ASSERT(desc != 0 && "Writing unregistered class. Use YASLI_CLASS macro for registration.");
  }

  if (ptr.get()) {
    writer_ << typeName;
    writer_.write(char(0));
    ptr.serializer()(*this);
  }
  else
    writer_.write(char(0));

  return true;
}


//////////////////////////////////////////////////////////////////////////

StreamIArchive::StreamIArchive()
: Archive(INPUT | BINARY)
{
}

bool StreamIArchive::load(const char* filename)
{
  close();
  FILE* f = fopen(filename, "rb");
  if (!f)
    return false;
  fseek(f, 0, SEEK_END);
  size_t length = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (length == 0) {
    fclose(f);
    return false;
  }
  loadedData_ = new char[length];
  if (fread((void*)loadedData_, 1, length, f) != length || !open(loadedData_, length)) {
    close();
    fclose(f);
    return false;
  }
  fclose(f);
  return true;
}

bool StreamIArchive::open(const char* buffer, size_t size)
{
  if (!buffer)
    return false;
  if (size < sizeof(unsigned int))
    return false;
  if (memcmp(buffer, &STREAM_MAGIC, sizeof(unsigned int)))
    return false;
  buffer += sizeof(unsigned int);
  size -= sizeof(unsigned int);

  reader_ = MemoryReader(buffer, size, false);
  return true;
}

void StreamIArchive::close()
{
  if (loadedData_)
    delete[] loadedData_;
  loadedData_ = 0;
}

bool StreamIArchive::operator()(bool& value, const char* name, const char* label)
{
  read(value);
  return true;
}

bool StreamIArchive::operator()(StringInterface& value, const char* name, const char* label)
{
  value.set(reader_.position());
  reader_.checkedSkip(strlen(reader_.position()) + 1);
  return true;
}

bool StreamIArchive::operator()(WStringInterface& value, const char* name, const char* label)
{
  const wchar_t* wstr = (const wchar_t*)reader_.position();
  value.set(wstr);
  reader_.checkedSkip((wcslen(wstr) + 1)*sizeof(wchar_t));
  return true;
}

bool StreamIArchive::operator()(float& value, const char* name, const char* label)
{
  read(value);
  return true;
}

bool StreamIArchive::operator()(double& value, const char* name, const char* label)
{
  read(value);
  return true;
}

bool StreamIArchive::operator()(i16& value, const char* name, const char* label)
{
  read(value);
  return true;
}

bool StreamIArchive::operator()(u16& value, const char* name, const char* label)
{
  read(value);
  return true;
}

bool StreamIArchive::operator()(i32& value, const char* name, const char* label)
{
  read(value);
  return true;
}

bool StreamIArchive::operator()(u32& value, const char* name, const char* label)
{
  read(value);
  return true;
}

bool StreamIArchive::operator()(i64& value, const char* name, const char* label)
{
  read(value);
  return true;
}

bool StreamIArchive::operator()(u64& value, const char* name, const char* label)
{
  read(value);
  return true;
}

bool StreamIArchive::operator()(i8& value, const char* name, const char* label)
{
  read(value);
  return true;
}

bool StreamIArchive::operator()(u8& value, const char* name, const char* label)
{
  read(value);
  return true;
}

bool StreamIArchive::operator()(char& value, const char* name, const char* label)
{
    read(value);
    return true;
}

bool StreamIArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
  ser(*this);
  return true;
}

bool StreamIArchive::operator()(ContainerInterface& ser, const char* name, const char* label)
{
  size_t size = readPackedSize();
  ser.resize(size);
  if (size > 0) {
    if (size > 0) {
      do
        ser(*this, "", "");
      while (ser.next());
    }
  }
  return true;
}

bool StreamIArchive::operator()(PointerInterface& ptr, const char* name, const char* label)
{
  string typeName(reader_.position());
  reader_.checkedSkip(strlen(reader_.position()) + 1);
  TypeID type;
  if (!typeName.empty())
    type = ptr.factory()->findTypeByName(typeName.c_str());
  if (ptr.type() && (!type || (type != ptr.type())))
    ptr.create(TypeID()); // 0

  if (type && !ptr.get())
    ptr.create(type);

  if (Serializer ser = ptr.serializer())
    ser(*this);

  return true;
}

unsigned StreamIArchive::readPackedSize() {
  unsigned char size8 = 0;
  read(size8);
  if (size8 < SIZE16)
    return size8;
  if (size8 == SIZE16) {
    unsigned short size16;
    read(size16);
    return size16;
  }
  unsigned int size32;
  read(size32);
  return size32;
}

}
