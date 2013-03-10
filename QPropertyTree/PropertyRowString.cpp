/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include <math.h>

#include "PropertyRowString.h"
#include "PropertyTreeModel.h"
#include "PropertyDrawContext.h"
#include "QPropertyTree.h"

#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include <QtGui/QMenu>
#include "Unicode.h"

// ---------------------------------------------------------------------------
YASLI_CLASS(PropertyRow, PropertyRowString, "string");

PropertyRowString::PropertyRowString()
: PropertyRowImpl<yasli::wstring, PropertyRowString>()
{
}

PropertyRowString::PropertyRowString(const char* name, const char* label, const char* typeName)
: PropertyRowImpl<yasli::wstring, PropertyRowString>(name, label, typeName)
{

}

bool PropertyRowString::assignTo(yasli::string& str)
{
    str = fromWideChar(value_.c_str());
    return true;
}

bool PropertyRowString::assignTo(yasli::wstring& str)
{
    str = value_;
    return true;
}

PropertyRowWidget* PropertyRowString::createWidget(QPropertyTree* tree)
{
	return new PropertyRowWidgetString(this, tree);
}

yasli::string PropertyRowString::valueAsString() const
{
	return fromWideChar(value_.c_str());
}

void PropertyRowString::setValue(const wchar_t* str)
{
	value_ = str;
}

void PropertyRowString::setValue(const char* str)
{
	value_ = toWideChar(str);
}

// vim:ts=4 sw=4:
