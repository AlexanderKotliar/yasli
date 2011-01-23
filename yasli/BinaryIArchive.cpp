#include "StdAfx.h"
#include "BinaryIArchive.h"
#include "Unicode.h"

using std::string;

namespace yasli{

const unsigned int BINARY_MAGIC = 0xb1a4c17e;

BinaryIArchive::BinaryIArchive(bool pretendToBeEdit)
: Archive(true, pretendToBeEdit)
, buffer_(0)
, size_(0)
, end_(0)
, loadedData_(0)
{

}

BinaryIArchive::~BinaryIArchive()
{
    if(loadedData_)
        delete[] loadedData_;
}

bool BinaryIArchive::load(const char* filename)
{
    if(loadedData_)
    {
        delete[] loadedData_;
        loadedData_ = 0;
    }
    FILE* f = ::yasli::fopen(filename, "rb");
    if(!f)
        return false;
    fseek(f, 0, SEEK_END);
    size_t length = ftell(f);
	fseek(f, 0, SEEK_SET);
    if(length == 0)
    {
        fclose(f);
        return false;
    }
    loadedData_ = new char[length];
	if(fread((void*)loadedData_, 1, length, f) != length)
	{
		delete[] loadedData_;
		loadedData_ = 0;
		fclose(f);
		return false;
	}
	if(!open(loadedData_, length))
	{
		delete[] loadedData_;
		loadedData_ = 0;
		fclose(f);
		return false;
	}
    fclose(f);
    return true;
}

bool BinaryIArchive::open(const char* buffer, size_t length)
{
    buffer_ = buffer;
    size_ = length;
    end_ = buffer_ + size_;
    pos_ = buffer_;

    if(*(unsigned int*)(pos_) != BINARY_MAGIC)
        return false;
    pos_ += sizeof(unsigned int);

    blocks_.push_back(Block(buffer_, pos_, end_));
    return true;
}

void BinaryIArchive::close()
{
    buffer_ = 0;
    size_ = 0;
    end_ = 0;
}

bool BinaryIArchive::read(char *_data, int _size)
{
    if(pos_ + _size <= blocks_.back().end)
    {
        memcpy(_data, pos_, _size);
        pos_ += _size;
        return true;
    }
    else
        return false;
}

bool BinaryIArchive::read(Token* str)
{
  unsigned char len;
  if(!read(&len))
    return false;
  ESCAPE(pos_ + len <= end_, return false;);
  *str = Token(pos_, pos_ + len);
  pos_ += len;
  return true;
}


bool BinaryIArchive::readUnsafe(char *_data, int _size)
{
    if(pos_ + _size <= end_)
    {
        memcpy(_data, pos_, _size);
        pos_ += _size;
        return true;
    }
    else
        return false;
}

bool BinaryIArchive::openStruct(const char *_name, Token* typeName)
{
	ESCAPE(!blocks_.empty(), return false);
    const char *start;
    const char *end;
    if(findNode (BINARY_NODE_STRUCT, Token(_name), &start, &end))
    {
        ESCAPE(read(typeName), return false;);
        blocks_.push_back(Block(start, pos_, end));
        pullPosition_ = 0;
        return true;
    }  
    return false;
}

bool BinaryIArchive::openContainer(const char *_name, Token *_typeName, size_t *_size)
{
    const char *start, *end;
    if(findNode(BINARY_NODE_CONTAINER, Token(_name), &start, &end))
    {
        ESCAPE(read(_typeName), return false);
        ESCAPE(read(_size), return false);
        blocks_.push_back(Block(start, pos_, end));
        pullPosition_ = 0;
        return true;
    }
    return false;
}

bool BinaryIArchive::openPointer(const char *_name, Token *_baseType, Token *_type)
{
    const char *start, *end;
    if(findNode(BINARY_NODE_POINTER, Token(_name), &start, &end))
    {
        ESCAPE(read(_baseType), return false;);
        ESCAPE(read(_type), return false;;);
        blocks_.push_back(Block(start, pos_, end));
        pullPosition_ = 0;
        return true;
    }
    return false;
}

void BinaryIArchive::closeStruct()
{
    Block block = blocks_.back();
    blocks_.pop_back();

    pullPosition_ = block.start;
    pos_ = block.end;

    ASSERT(*((unsigned int*)pullPosition_) == block.end - block.start);
}

void BinaryIArchive::closeContainer()
{
    closeStruct();
}

void BinaryIArchive::closePointer()
{
    closeStruct();
}

size_t BinaryIArchive::readNodeHeader(BinaryNode* _type, Token* name)
{
    unsigned int blockSize;
    read(&blockSize);
    ASSERT(blockSize >= 4 && blockSize <= (unsigned int)(blocks_.back().end - blocks_.back().start ));
    unsigned char type;
    read(&type);
    *_type = BinaryNode(type);

    ESCAPE(read(name), return blockSize);
    return blockSize;
}

bool BinaryIArchive::findNode(BinaryNode _type, const char *_name)
{
    const char* start;
    const char* end;
    return findNode(_type, Token(_name), &start, &end);
}

bool BinaryIArchive::findNode(BinaryNode _type, const Token &_name, const char** start, const char** end)
{
    if(blocks_.back().innerStart == blocks_.back().end)
        return false;
    if(pos_ == blocks_.back().end)
        pos_ = blocks_.back().innerStart;

    const char* stopPosition = pos_;

    while (true)
    {
        BinaryNode type;
        Token nodeName;

        *start = pos_;
        *end = *start + readNodeHeader(&type, &nodeName);
        if(nodeName == _name)
        {
            if(type == _type)
                return true;
            else
            {
                pos_ = stopPosition;
                return false;
            }
        }
        pos_ = *end;
        if(pos_ == blocks_.back().end)
            pos_ = blocks_.back().innerStart;
        if(pos_ == stopPosition)
            return false;
    }
}

void BinaryIArchive::closeNode() 
{

}

Token BinaryIArchive::pullNode(BinaryNode* type)
{
    Token name;

    const char* oldPosition = pos_;
    if(pullPosition_ == 0)
        pullPosition_ = blocks_.back().innerStart;
    else
    {
        pos_ = pullPosition_;
        size_t blockSize;
        read(&blockSize);
        pullPosition_ += blockSize;
        ASSERT( pullPosition_ >= blocks_.back().innerStart && 
                pullPosition_ <= blocks_.back().end );
        if(pullPosition_ == blocks_.back().end)
        {
            pos_ = oldPosition;
            return Token(0);
        }
    }

    pos_ = pullPosition_;
    if(pos_ == blocks_.back().end && oldPosition == pos_)
        return Token(0);

    size_t blockSize = readNodeHeader(type, &name);
    if(blockSize > 0)
    {
        pos_ = oldPosition;
        return name;
    }
    else
        return Token(0);

}

bool BinaryIArchive::operator()(bool& value, const char* name, const char* label)
{
    if(findNode(BINARY_NODE_BOOL, name))
    {
        read(&value);
        return true;
    }
    return false;
}

bool BinaryIArchive::operator()(string& value, const char* name, const char* label)
{
    const char *start, *end;

    if(findNode(BINARY_NODE_STRING, Token(name), &start, &end))
    {
        value.assign(pos_, end);
        pos_ = end;
        return true;
    }
    return false;
}

bool BinaryIArchive::operator()(float& value, const char* name, const char* label)
{
    if(findNode(BINARY_NODE_FLOAT, name))
    {
        read(&value);
        return true;
    }
    return false;
}

bool BinaryIArchive::operator()(double& value, const char* name, const char* label)
{
    if(findNode(BINARY_NODE_DOUBLE, name))
    {
        read(&value);
        return true;
    }
    return false;
}

bool BinaryIArchive::operator()(short& value, const char* name, const char* label)
{
    if(findNode(BINARY_NODE_INT16, name))
    {
        read(&value);
        return true;
    }
    return false;
}

bool BinaryIArchive::operator()(unsigned short& value, const char* name, const char* label)
{
    if(findNode(BINARY_NODE_UINT16, name))
    {
        read(&value);
        return true;
    }
    return false;
}


bool BinaryIArchive::operator()(int& value, const char* name, const char* label)
{
    if(findNode(BINARY_NODE_INT32, name))
    {
        read(&value);
        return true;
    }
    return false;
}

bool BinaryIArchive::operator()(unsigned int& value, const char* name, const char* label)
{
    if(findNode(BINARY_NODE_UINT32, name))
    {
        read(&value);
        return true;
    }
    return false;
}

bool BinaryIArchive::operator()(__int64& value, const char* name, const char* label)
{
    if(findNode(BINARY_NODE_INT64, name))
    {
        read(&value);
        return true;
    }
    return false;
}


bool BinaryIArchive::operator()(signed char& value, const char* name, const char* label)
{
    if(findNode(BINARY_NODE_SBYTE, name))
    {
        read(&value);
        return true;
    }
    return false;
}

bool BinaryIArchive::operator()(unsigned char& value, const char* name, const char* label)
{
    if(findNode(BINARY_NODE_BYTE, name))
    {
        read(&value);
        return true;
    }
    return false;
}

bool BinaryIArchive::operator()(char& value, const char* name, const char* label)
{
    if(findNode(BINARY_NODE_SBYTE, name))
    {
        read(&value);
        return true;
    }
    return false;
}


bool BinaryIArchive::operator()(const Serializer& ser, const char* _name, const char* label)
{
    Token typeName;
    if(openStruct(_name, &typeName))
    {
        ser(*this);

        closeStruct();
        return true;
    }
    return false;
}

bool BinaryIArchive::operator()(ContainerSerializationInterface& ser, const char* name, const char* label)
{
    size_t size;
    Token typeName;
    if(openContainer( name, &typeName, &size))
    {
        ser.resize(size);

        if(ser.size() > 0)
        {
            do
            {
                ser(*this, "", "");
            } while(ser.next());
        }

        closeContainer();
        return true;
    }
    return false;
}

bool BinaryIArchive::operator()(const PointerSerializationInterface& ptr, const char* name, const char* label)
{
    Token baseTypeName;
    Token typeName;
    if(openPointer(name, &baseTypeName, &typeName))
    { 
        TypeID oldType = ptr.type();
        TypeID baseType;
        if(baseTypeName.length() > 0)
            baseType = TypeID(baseTypeName.str().c_str());
        TypeID type;
        if(typeName.length() > 0)
            type = TypeID(typeName.str().c_str());
            // type = TypeID(baseType, typeName.str().c_str()); TODO

        if(oldType && (!type || (type != oldType)))
            ptr.create(TypeID()); // 0

        if(type && !ptr.get())
            ptr.create(type);

		if(Serializer ser = ptr.serializer())
			ser(*this);

        closePointer();
        return true;
    }
    return false;
}

const char* BinaryIArchive::pull()
{
    BinaryNode type;
    pulledName_ = pullNode(&type).str();
	return pulledName_.c_str();
}

}

