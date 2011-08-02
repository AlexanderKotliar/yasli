/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/_WidgetWithWindow.h"

namespace ww{

class Space;
class SpaceHeader : public _WidgetWithWindow{
public:
	SpaceHeader(Space* space, int border = 0);

	void serialize(Archive& ar);
	
	const char* label() const{ return label_.c_str(); }
	void setLabel(const char* label);

	Space* space() { return space_; }

	void onMenuReplaceSpace(int index);
	void onMenuSplit();
protected:
	friend class SpaceHeaderImpl;

	void updateMinimalSize();

	std::string label_;
	Space* space_;
};

}

