/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>
#include "yasli/Assert.h"
#include "yasli/TypeID.h"
#include "yasli/Config.h"

namespace yasli{

class Archive;

typedef bool(*SerializeStructFunc)(void*, Archive&);

typedef bool(*SerializeContainerFunc)(void*, Archive&, size_t index);
typedef size_t(*ContainerResizeFunc)(void*, size_t size);
typedef size_t(*ContainerSizeFunc)(void*);

// Struct serializer. 
class Serializer{/*{{{*/
	friend class Archive;
public:
	Serializer()
	: object_(0)
	, size_(0)
	, serializeFunc_(0)
	{
	}

	Serializer(TypeID type, void* object, size_t size, SerializeStructFunc serialize)
	: type_(type)
	, object_(object)
	, size_(size)
	, serializeFunc_(serialize)
	{
		YASLI_ASSERT(object != 0);
	}

	Serializer(const Serializer& _original)
	: type_(_original.type_)
	, object_(_original.object_)
	, size_(_original.size_)
	, serializeFunc_(_original.serializeFunc_)
	{
	}

	template<class T>
	explicit Serializer(const T& object){
		YASLI_ASSERT(TypeID::get<T>() != TypeID::get<Serializer>());
		type_=  TypeID::get<T>();
		object_ = (void*)(&object);
		size_ = sizeof(T);
		serializeFunc_ = &Serializer::serializeRaw<T>;
	}

	template<class T>
	explicit Serializer(T& object, TypeID type){
		type_ =  type;
		object_ = (void*)(&object);
		size_ = sizeof(T);
		serializeFunc_ = &Serializer::serializeRaw<T>;
	}

	template<class T>
	T* cast() const{ return type_ == TypeID::get<T>() ? (T*)object_ : 0; }
	bool operator()(Archive& ar) const;
	operator bool() const{ return object_ && serializeFunc_; }
	bool operator==(const Serializer& rhs) { return object_ == rhs.object_ && serializeFunc_ == rhs.serializeFunc_; }
	bool operator!=(const Serializer& rhs) { return !operator==(rhs); }
	void* pointer() const{ return object_; }
	TypeID type() const{ return type_; }
	size_t size() const{ return size_; }
	SerializeStructFunc serializeFunc() const{ return serializeFunc_; }

	template<class T>
	static bool serializeRaw(void* rawPointer, Archive& ar){
		YASLI_ESCAPE(rawPointer, return false);
		((T*)(rawPointer))->YASLI_SERIALIZE_METHOD(ar);
		return true;
	}

private:
	TypeID type_;
	void* object_;
	size_t size_;
	SerializeStructFunc serializeFunc_;
};/*}}}*/
typedef std::vector<Serializer> Serializers;

// ---------------------------------------------------------------------------

class ContainerInterface{
public:
	virtual ~ContainerInterface(){}
	virtual size_t size() const = 0;
	virtual size_t resize(size_t size) = 0;
	virtual bool isFixedSize() const{ return false; }

	virtual void* pointer() const = 0;
	virtual TypeID elementType() const = 0;
	virtual TypeID containerType() const = 0;
	virtual bool next() = 0;

	virtual bool operator()(Archive& ar, const char* name, const char* label) = 0;
	virtual void serializeNewElement(Archive& ar, const char* name, const char* label = "&") const = 0;
};

template<class T, size_t Size>
class ContainerArray : public ContainerInterface{
	friend class Archive;
public:
	explicit ContainerArray(T* array = 0)
	: array_(array)
	, index_(0)
	{
	}

	// from ContainerInterface:
	size_t size() const{ return Size; }
	size_t resize(size_t size){
		index_ = 0;
		return Size;
	}

	void* pointer() const{ return reinterpret_cast<void*>(array_); }
	TypeID containerType() const{ return TypeID::get<T[Size]>(); }
	TypeID elementType() const{ return TypeID::get<T>(); }
	virtual bool isFixedSize() const{ return true; }

	bool operator()(Archive& ar, const char* name, const char* label){
		YASLI_ESCAPE(size_t(index_) < Size, return false);
		return ar(array_[index_], name, label);
	}
	bool next(){
		++index_;
		return size_t(index_) < Size;
	}
	void serializeNewElement(Archive& ar, const char* name, const char* label) const{
		T element;
		ar(element, name, label);
	}
	// ^^^

private:
	T* array_;
	int index_;
};

class ClassFactoryBase;
class PointerInterface
{
public:
	virtual ~PointerInterface(){}
	virtual TypeID type() const = 0;
	virtual void create(TypeID type) const = 0;
	virtual TypeID baseType() const = 0;
	virtual Serializer serializer() const = 0;
	virtual void* get() const = 0;
	virtual ClassFactoryBase* factory() const = 0;
	
	void YASLI_SERIALIZE_METHOD(Archive& ar) const;
};

class StringInterface
{
public:
	virtual ~StringInterface(){}
	virtual void set(const char* value) = 0;
	virtual const char* get() const = 0;
};
class WStringInterface
{
public:
	virtual ~WStringInterface(){}
	virtual void set(const wchar_t* value) = 0;
	virtual const wchar_t* get() const = 0;
};

struct TypeIDWithFactory;
bool serialize(Archive& ar, TypeIDWithFactory& value, const char* name, const char* label);

}


// vim:ts=4 sw=4:
