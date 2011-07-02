#include "StdAfx.h"

#include "ww/Serialization.h"
#include "yasli/EnumDescription.h"
#include "ww/PropertyTreeModel.h"
#include "PropertyIArchive.h"
#include "ww/_PropertyRowBuiltin.h"

namespace ww{

PropertyIArchive::PropertyIArchive(PropertyTreeModel* model)
: Archive( true, true )
, model_(model)
, currentNode_(0)
, lastNode_(0)
{
}

bool PropertyIArchive::operator()(std::string& value, const char* name, const char* label)
{
	if(openRow(name, label, "string")){
		if(PropertyRowString* row = safe_cast<PropertyRowString*>(currentNode_))
 			row->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(std::wstring& value, const char* name, const char* label)
{
	if(openRow(name, label, "string")){
		if(PropertyRowString* row = safe_cast<PropertyRowString*>(currentNode_))
	 		row->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(bool& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(bool).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(char& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(char).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

// Signed types
bool PropertyIArchive::operator()(signed char& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(signed char).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(signed short& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(signed short).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(signed int& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(signed int).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(signed long& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(signed long).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(long long& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(long long).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

// Unsigned types
bool PropertyIArchive::operator()(unsigned char& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(unsigned char).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(unsigned short& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(unsigned short).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(unsigned int& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(unsigned int).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(unsigned long& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(unsigned long).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(unsigned long long& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(unsigned long long).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(float& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(float).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(double& value, const char* name, const char* label)
{
	if(openRow(name, label, typeid(double).name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(ContainerSerializationInterface& ser, const char* name, const char* label)
{
    const char* typeName = ser.type().name();
	if(!openRow(name, label, typeName))
        return false;

    size_t size = 0;
	if(currentNode_->multiValue())
		size = ser.size();
	else{
		size = currentNode_->count();
		size = ser.resize(size);
	}

	size_t index = 0;
    if(ser.size() > 0)
        while(index < size)
        {
            ser(*this, "", "<");
            ser.next();
			++index;
        }

    closeRow(name);
	return true;
}

bool PropertyIArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
	if(openRow(name, label, ser.type().name())){
		if(currentNode_->isLeaf() && !currentNode_->isRoot()){
            currentNode_->assignTo(ser.pointer(), ser.size());
            closeRow(name);
            return true;
		}
    }
	else
		return false;

    ser(*this);

    closeRow(name);
	return true;
}


bool PropertyIArchive::operator()(const PointerSerializationInterface& ser, const char* name, const char* label)
{
    const char* baseName = ser.baseType().name();

	if(openRow(name, label, baseName)){
		ASSERT(currentNode_);
		PropertyRowPointer* row = dynamic_cast<PropertyRowPointer*>(currentNode_);
        if(!row){
            closeRow(name);
			return false;
        }
	    row->assignTo(ser);
	}
    else
        return false;

    if(ser.get() != 0)
        ser.serializer()( *this );

	closeRow(name);
    return true;
}

bool PropertyIArchive::openBlock(const char* name, const char* label)
{
	if(openRow(label, label, "")){
		return true;
	}
	else
		return false;
}

void PropertyIArchive::closeBlock()
{
	closeRow("block");
}

bool PropertyIArchive::openRow(const char* name, const char* label, const char* typeName)
{
	if(!name)
		return false;
	if(name[0] == '\0' && label && label[0] != '\0')
		name = label;

	if(!currentNode_){
		lastNode_ = currentNode_ = model_->root();
		ASSERT(currentNode_);
		return true;
	}

	ESCAPE(currentNode_, return false);
	
	if(currentNode_->empty())
		return false;

	PropertyRow* node = 0;
	if(currentNode_->isContainer()){
		if(lastNode_ == currentNode_){
			node = static_cast<PropertyRow*>(currentNode_->front());
		}
		else{
			PropertyRow* row = lastNode_;
			while(row->parent() && currentNode_ != row->parent())
				row = row->parent();
			
			PropertyRow::iterator iter = std::find(currentNode_->begin(), currentNode_->end(), row);
			if(iter != currentNode_->end()){
				++iter;

				if(iter != currentNode_->end())
					node = static_cast<PropertyRow*>(&**iter);
			}
		}
	}
	else
		node = currentNode_->find(name, 0, typeName);

	if(node){
		lastNode_ = node;
		if(node->isContainer() || !node->multiValue()){
			currentNode_ = node;
			return true;
		}
	}
	return false;
}

void PropertyIArchive::closeRow(const char* name)
{
	ESCAPE(currentNode_, return);
	currentNode_ = currentNode_->parent();
}

};
