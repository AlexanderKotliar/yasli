#pragma once
#include <map>

#include "yasli/Assert.h"
#include "yasli/TypeID.h"
#include "yasli/API.h"

#ifdef _MSC_VER
# pragma warning(disable : 4251)
#endif

namespace yasli{

class Archive;

class YASLI_API TypeDescription{
public:
	TypeDescription(TypeID typeID, const char* name, const char *label, std::size_t size)
	: name_(name)
	, label_(label)
	, size_(size)
	, typeID_(typeID)
	{
	}
	const char* name() const{ return name_; }
	const char* label() const{ return label_; }
	std::size_t size() const{ return size_; }
	TypeID typeID() const{ return typeID_; }

protected:
	const char* name_;
	const char* label_;
	std::size_t size_;
	TypeID typeID_;
};


class ClassFactoryBase{
public: 
	ClassFactoryBase(TypeID baseType)
	: baseType_(baseType)
    , nullLabel_(0)
	{
	}

	virtual int size() const = 0;
	virtual const TypeDescription* descriptionByIndex(int index) const = 0;	
	virtual size_t sizeOf(TypeID typeID) const = 0;
	virtual void serializeNewByIndex(Archive& ar, int index, const char* name, const char* label) = 0;

    bool setNullLabel(const char* label){ nullLabel_ = label ? label : ""; return true; }
    const char* nullLabel() const{ return nullLabel_; }
protected:
	TypeID baseType_;
    const char* nullLabel_;
};

class ClassFactoryManager{
public:
	static ClassFactoryManager& the(){
		static ClassFactoryManager factoryManager;
		return factoryManager;
	}

	const ClassFactoryBase* find(TypeID baseType) const{
		Factories::const_iterator it = factories_.find(baseType);
		if(it == factories_.end())
			return 0;
		else
			return it->second;
	}

	void registerFactory(TypeID type, const ClassFactoryBase* factory){
		factories_[type] = factory;
	}
protected:
	typedef std::map<TypeID, const ClassFactoryBase*> Factories;
	Factories factories_;
};

template<class BaseType>
class ClassFactory : public ClassFactoryBase{
public:
	static ClassFactory& the(){
		static ClassFactory factory;
		return factory;
	}

	class CreatorBase{
	public:
		virtual ~CreatorBase() {}
		virtual BaseType* create() const = 0;
		const TypeDescription& description() const{ return *description_; }
	protected:
		const TypeDescription* description_;
	};

	template<class Derived>
	class Creator : public CreatorBase{
	public:
		Creator(const TypeDescription* description){
			this->description_ = description;
			ClassFactory::the().registerCreator(this);
		}
		BaseType* create() const{
			return new Derived();
		}
	};

	ClassFactory()
	: ClassFactoryBase(TypeID::get<BaseType>())
	{
		ClassFactoryManager::the().registerFactory(baseType_, this);
	}

	typedef std::map<TypeID, CreatorBase*> TypeToCreatorMap;

	BaseType* create(TypeID derivedType) const
	{
		typename TypeToCreatorMap::const_iterator it = typeToCreatorMap_.find(derivedType);
		if(it != typeToCreatorMap_.end())
			return it->second->create();
		else
			return 0;
	}

	size_t sizeOf(TypeID derivedType) const
	{
		typename TypeToCreatorMap::const_iterator it = typeToCreatorMap_.find(derivedType);
		if(it != typeToCreatorMap_.end())
			return it->second->description().size();
		else
			return 0;
	}

	BaseType* createByIndex(int index) const
	{
		ASSERT(index >= 0 && index < typeToCreatorMap_.size());
		typename TypeToCreatorMap::const_iterator it = typeToCreatorMap_.begin();
		while(index--)
			++it;
		return it->second->create();
	}

	void serializeNewByIndex(Archive& ar, int index, const char* name, const char* label)
	{
		ESCAPE(size_t(index) < typeToCreatorMap_.size(), return);
		typename TypeToCreatorMap::const_iterator it = typeToCreatorMap_.begin();
		while(index--)
			++it;
		BaseType* ptr = it->second->create();
		ar(*ptr, name, label);
		delete ptr;
	}
	// from ClassFactoryInterface:
	int size() const{ return typeToCreatorMap_.size(); }
	const TypeDescription* descriptionByIndex(int index) const{
		if(index < 0 || index >= int(typeToCreatorMap_.size()))
			return 0;
		TypeToCreatorMap::const_iterator it = typeToCreatorMap_.begin();
		std::advance(it, index);
		return &it->second->description();
	}

	// ^^^

protected:
	void registerCreator(CreatorBase* creator){
		typeToCreatorMap_[creator->description().typeID()] = creator;
	}

	TypeToCreatorMap typeToCreatorMap_;
};


class YASLI_API TypeLibrary{
public:
	static TypeLibrary& the();
	const TypeDescription* findByName(const char*) const;
	const TypeDescription* find(TypeID type) const;

	const TypeDescription* registerType(const TypeDescription* info);
protected:
	TypeLibrary();

	typedef std::map<TypeID, const TypeDescription*> TypeToDescriptionMap;
	TypeToDescriptionMap typeToDescriptionMap_;
};

}

#define YASLI_TYPE_NAME(Type, name) \
namespace{ \
	const yasli::TypeDescription Type##_Description(yasli::TypeID::get<Type>(), #Type, name, sizeof(Type)); \
	bool registered_##Type = yasli::TypeLibrary::the().registerType(&Type##_Description) != 0; \
}

#define YASLI_CLASS_NULL(BaseType, name) \
namespace { \
    bool BaseType##_NullRegistered = yasli::ClassFactory<BaseType>::the().setNullLabel(name); \
}

#define YASLI_CLASS(BaseType, Type, name) \
	const yasli::TypeDescription Type##BaseType##_DerivedDescription(yasli::TypeID::get<Type>(), #Type, name, sizeof(Type)); \
	yasli::ClassFactory<BaseType>::Creator<Type> Type##BaseType##_Creator(yasli::TypeLibrary::the().registerType(&Type##BaseType##_DerivedDescription)); \
	int dummyForType_##Type##BaseType;

#define YASLI_FORCE_CLASS(BaseType, Type) \
	extern int dummyForType_##Type##BaseType; \
	int* dummyForTypePtr_##Type##BaseType = &dummyForType_##Type##BaseType + 1;


