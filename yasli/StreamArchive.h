/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/Archive.h"
#include "yasli/MemoryWriter.h" 
#include "yasli/MemoryReader.h" 

namespace yasli{

class StreamOArchive : public Archive{
public:
	StreamOArchive();

	void clear();
	size_t length() const { return writer_.position(); }
	const char* buffer() const { return writer_.buffer(); }
	bool save(const char* fileName);

	bool operator()(bool& value, const char* name, const char* label) override;
	bool operator()(StringInterface& value, const char* name, const char* label) override;
	bool operator()(WStringInterface& value, const char* name, const char* label) override;
	bool operator()(float& value, const char* name, const char* label) override;
	bool operator()(double& value, const char* name, const char* label) override;
	bool operator()(i8& value, const char* name, const char* label) override;
	bool operator()(i16& value, const char* name, const char* label) override;
	bool operator()(i32& value, const char* name, const char* label) override;
	bool operator()(i64& value, const char* name, const char* label) override;
	bool operator()(u8& value, const char* name, const char* label) override;
	bool operator()(u16& value, const char* name, const char* label) override;
	bool operator()(u32& value, const char* name, const char* label) override;
	bool operator()(u64& value, const char* name, const char* label) override;

	bool operator()(char& value, const char* name, const char* label) override;

	bool operator()(const Serializer &ser, const char* name, const char* label) override;
	bool operator()(ContainerInterface &ser, const char* name, const char* label) override;
	bool operator()(PointerInterface &ptr, const char* name, const char* label) override;

	using Archive::operator();

private:
	MemoryWriter writer_;
};

//////////////////////////////////////////////////////////////////////////

class StreamIArchive : public Archive{
public:
  StreamIArchive();

	bool load(const char* fileName);
	bool open(const char* buffer, size_t length); // не копирует буффер!!!
	bool open(const StreamOArchive& ar) { return open(ar.buffer(), ar.length()); }
	void close();

	bool operator()(bool& value, const char* name, const char* label) override;
	bool operator()(char& value, const char* name, const char* label) override;
	bool operator()(float& value, const char* name, const char* label) override;
	bool operator()(double& value, const char* name, const char* label) override;
	bool operator()(i8& value, const char* name, const char* label) override;
	bool operator()(u8& value, const char* name, const char* label) override;
	bool operator()(i16& value, const char* name, const char* label) override;
	bool operator()(u16& value, const char* name, const char* label) override;
	bool operator()(i32& value, const char* name, const char* label) override;
	bool operator()(u32& value, const char* name, const char* label) override;
	bool operator()(i64& value, const char* name, const char* label) override;
	bool operator()(u64& value, const char* name, const char* label) override;


	bool operator()(StringInterface& value, const char* name, const char* label) override;
	bool operator()(WStringInterface& value, const char* name, const char* label) override;
	bool operator()(const Serializer& ser, const char* name, const char* label) override;
	bool operator()(ContainerInterface& ser, const char* name, const char* label) override;
	bool operator()(PointerInterface& ptr, const char* name, const char* label) override;

	using Archive::operator();

private:
  const char* loadedData_ = 0;
  MemoryReader reader_;

	template<class T>
	void read(T& t) { reader_.read(t); }
  unsigned int readPackedSize();

};
}
