/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>

#include "ww/Space.h"
#include "ww/_WidgetWithWindow.h"
#include "ww/HSplitter.h"
#include "ww/VSplitter.h"

class Archive;

namespace ww{

class Workspace;
class RootSpace : public Space{
public:
	explicit RootSpace(Workspace* workspace)
	: Space()
	, workspace_(workspace)
	{
	}

	void serialize(Archive& ar);
	void replaceSpace(Space* oldSpace, Space* newSpace);

	void reparent();
	void setSpace(Space* space);
	Widget* widget();

	Space* spaceByPoint(Vect2 screenPoint);
	Workspace* workspace() { return workspace_; }
protected:
	SharedPtr<Space> child_;
	Workspace* workspace_;
};

class SpaceBox : public Space{
public:
	bool isLeaf() const{ return false; }
	void serialize(Archive& ar);

	Space* spaceByPoint(Vect2 screenPoint);
	void replaceSpace(Space* oldSpace, Space* newSpace);
	void removeSpace(int index);
	ww::Widget* widget() { return splitter_; }
	bool selfSplit(Vect2 point);
	virtual bool vertical() const = 0;
	void setParent(Space* parent);
protected:
	typedef std::vector<SharedPtr<Space> > Spaces;
	Spaces spaces_;
	SharedPtr<Splitter> splitter_;
};

class SpaceHBox : public SpaceBox{
public:
	SpaceHBox();
	SpaceHBox(Space* oldSpace, Vect2 screenSplitPoint);
	bool vertical() const { return false; }
};


class SpaceVBox : public SpaceBox{
public:
	SpaceVBox();
	SpaceVBox(Space* oldSpace, Vect2 screenSplitPoint);
	bool vertical() const { return true; }
};

class WorkspaceImpl;

class Workspace : public _ContainerWithWindow{
public:
	Workspace(int border = 0);
	~Workspace();

	void serialize(Archive& ar);

	void setSpace(Space* space);
	void setRootWidget(Widget* widget);
	void visitChildren(WidgetVisitor& visitor) const;

	void startSplit();
	
	void setStateFile(const char* fileName);
	void saveState();
	void revertState();
protected:
	friend class WorkspaceImpl;
	WorkspaceImpl* impl();
	void _arrangeChildren();
	void _setParent(Container* container);
	void _setFocus();

	void loadState(const char* fileName);
	void saveState(const char* fileName);

	std::string stateFile_;

	//SharedPtr<Space> root_;
	RootSpace rootSpace_;
	SharedPtr<Widget> rootWidget_;
};

}

