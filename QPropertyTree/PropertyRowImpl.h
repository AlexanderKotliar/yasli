/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "yasli/STL.h"
#include "PropertyRowField.h"
#include "Serialization.h"

template<class Type>
class PropertyRowImpl;

template<class Type>
class PropertyRowImpl : public PropertyRowField{
public:
	bool assignTo(const Serializer& ser) const override{
		*reinterpret_cast<Type*>(ser.pointer()) = value();
		return true;
	}
	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }
	void setValue(const Type& value) override{ value_ = value; }
	Type& value() { return value_; }
	const Type& value() const{ return value_; }

	void setValue(const Serializer& ser) override {
		YASLI_ESCAPE(ser.size() == sizeof(Type), return);
		value_ = *(Type*)(ser.pointer());
	}

	void serializeValue(yasli::Archive& ar){
		ar(value_, "value", "Value");
	}
	WidgetPlacement widgetPlacement() const{ return WIDGET_VALUE; }
protected:
	Type value_; 
};

