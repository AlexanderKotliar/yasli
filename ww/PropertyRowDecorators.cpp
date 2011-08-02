/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "PropertyRowImpl.h"
#include "yasli/Archive.h"
#include "yasli/TypesFactory.h"
#include "ww/Decorators.h"
#include "ww/PropertyTree.h"
#include "ww/PropertyDrawContext.h"
#include "ww/PropertyTreeModel.h"
#include "ww/Win32/Window.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Unicode.h"
#include "gdiplus.h"

namespace ww{

class PropertyRowButton : public PropertyRowImpl<ButtonDecorator, PropertyRowButton>{
public:
	PropertyRowButton();
	PropertyRowButton(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName);
	void redraw(const PropertyDrawContext& context);
	bool onMouseDown(PropertyTree* tree, Vect2 point, bool& changed);
	void onMouseMove(PropertyTree* tree, Vect2 point);
	void onMouseUp(PropertyTree* tree, Vect2 point);
	bool onActivate(PropertyTree* tree, bool force);
	int floorHeight() const{ return 3; }
	std::string valueAsString() const { return value_ ? value_.text : ""; }
	int widgetSizeMin() const{ 
		if (userWidgetSize())
			return userWidgetSize();
		else
			return 60; 
	}
protected:
	bool underMouse_;
	bool locked_;
};

PropertyRowButton::PropertyRowButton(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowImpl<ButtonDecorator, PropertyRowButton>(object, size, name, nameAlt, typeName)
, underMouse_(false), locked_(false)
{
}

PropertyRowButton::PropertyRowButton()
: underMouse_(false), locked_(false)
{
}
	

void PropertyRowButton::redraw(const PropertyDrawContext& context)
{
	using namespace Gdiplus;
	using Gdiplus::Color;

	Rect buttonRect(context.widgetRect);
	buttonRect.setLeft(buttonRect.left() - 1);
	buttonRect.setRight(buttonRect.right() + 1);
	buttonRect.setBottom(buttonRect.bottom() + 2);

	std::wstring text(toWideChar(value().text ? value().text : labelUndecorated()));
	bool pressed = underMouse_ && value();
	context.drawButton(buttonRect, text.c_str(), pressed, selected() && context.tree->hasFocus());
}

bool PropertyRowButton::onMouseDown(PropertyTree* tree, Vect2 point, bool& changed)
{
	if(widgetRect().pointInside(point)){
		value().pressed = !value().pressed;
		underMouse_ = true;
		tree->redraw();
		return true;
	}
	return false;
}

void PropertyRowButton::onMouseMove(PropertyTree* tree, Vect2 point)
{
	bool underMouse = widgetRect().pointInside(point);
	if(underMouse != underMouse_){
		underMouse_ = underMouse;
		tree->redraw();
	}
}

void PropertyRowButton::onMouseUp(PropertyTree* tree, Vect2 point)
{
	if(!locked_ && widgetRect().pointInside(point)){
		onActivate(tree, false);
    }
	else{
        tree->model()->push(this);
		value().pressed = false;
		tree->redraw();
	}
}

bool PropertyRowButton::onActivate(PropertyTree* tree, bool force)
{
	value().pressed = true;
	locked_ = true;
	tree->model()->rowChanged(this); // Row is recreated here, so don't unlock
	return true;
}

DECLARE_SEGMENT(PropertyRowDecorators)
REGISTER_PROPERTY_ROW(ButtonDecorator, PropertyRowButton);

// ------------------------------------------------------------------------------------------

class PropertyRowHLine : public PropertyRowImpl<HLineDecorator, PropertyRowHLine>{
public:
	PropertyRowHLine();
	PropertyRowHLine(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName);
	void redraw(const PropertyDrawContext& context);
	bool isSelectable() const{ return false; }
};

PropertyRowHLine::PropertyRowHLine(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowImpl<HLineDecorator, PropertyRowHLine>(object, size, name, nameAlt, typeName)
{
}

PropertyRowHLine::PropertyRowHLine()
{
}

void PropertyRowHLine::redraw(const PropertyDrawContext& context)
{
	int halfHeight = context.widgetRect.top() + (context.widgetRect.height()) / 2;
	RECT hlineRect = { context.widgetRect.left(), halfHeight - 1, context.widgetRect.right(), halfHeight + 1 };

	HDC dc = context.graphics->GetHDC();
	DrawEdge(dc, &hlineRect, BDR_SUNKENOUTER, BF_RECT);
	context.graphics->ReleaseHDC(dc);
}

REGISTER_PROPERTY_ROW(HLineDecorator, PropertyRowHLine);

// ------------------------------------------------------------------------------------------

class PropertyRowNot : public PropertyRowImpl<NotDecorator, PropertyRowNot>{
public:
	PropertyRowNot();
	PropertyRowNot(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName);
	bool onActivate(PropertyTree* tree, bool force);
	void redraw(const PropertyDrawContext& context);
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }
	std::string valueAsString() const { return value_ ? label() : ""; }
	virtual int widgetSizeMin() const{ return ICON_SIZE; }
};

PropertyRowNot::PropertyRowNot(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowImpl<NotDecorator, PropertyRowNot>(object, size, name, nameAlt, typeName)
{
}

PropertyRowNot::PropertyRowNot()
{
}
	
bool PropertyRowNot::onActivate(PropertyTree* tree, bool force)
{
    tree->model()->push(this);
	value() = !value();
	tree->model()->rowChanged(this);
	return true;
}

void PropertyRowNot::redraw(const PropertyDrawContext& context)
{
	Win32::drawNotCheck(context.graphics, gdiplusRect(context.widgetRect), value());
}

REGISTER_PROPERTY_ROW(NotDecorator, PropertyRowNot);

// ---------------------------------------------------------------------------
//
class PropertyRowRadio : public PropertyRowImpl<RadioDecorator, PropertyRowRadio>{
public:
	PropertyRowRadio();
	PropertyRowRadio(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName);
	bool onActivate(PropertyTree* tree, bool force);
	void redraw(const PropertyDrawContext& context);
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }
	std::string valueAsString() const { return value() ? label() : ""; }
	int widgetSizeMin() const{ return ICON_SIZE; }
};

PropertyRowRadio::PropertyRowRadio(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowImpl<RadioDecorator, PropertyRowRadio>(object, size, name, nameAlt, typeName)
{
}

PropertyRowRadio::PropertyRowRadio()
{
}
	
bool PropertyRowRadio::onActivate(PropertyTree* tree, bool force)
{
    tree->model()->push(this);
	value() = !value();
	tree->model()->rowChanged(this);
	return true;
}

void PropertyRowRadio::redraw(const PropertyDrawContext& context)
{
	Win32::drawRadio(context.graphics, gdiplusRect(context.widgetRect), value());
}

REGISTER_PROPERTY_ROW(RadioDecorator, PropertyRowRadio);

}
