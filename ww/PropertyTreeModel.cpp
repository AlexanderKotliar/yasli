/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/PropertyTreeModel.h"
#include "ww/PropertyTree.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"

namespace ww{

PropertyTreeModel::PropertyTreeModel()
: expandLevels_(0)
, undoEnabled_(true)
, fullUndo_(false)
{
	clear();
}

TreePath PropertyTreeModel::pathFromRow(PropertyRow* row)
{
	TreePath result;
	if(row)
		while(row->parent()){
            int childIndex = row->parent()->childIndex(row);
            ESCAPE(childIndex >= 0, return TreePath());
			result.insert(result.begin(), childIndex);
			row = row->parent();
		}
		return result;
}

void PropertyTreeModel::selectRow(PropertyRow* row, bool select, bool exclusive)
{
	if(exclusive)
		deselectAll();

	row->setSelected(select);

	Selection::iterator it = std::find(selection_.begin(), selection_.end(), pathFromRow(row));
	if(select){
		if(it == selection_.end())
			selection_.push_back(pathFromRow(row));
		setFocusedRow(row);
	}
	else if(it != selection_.end()){
		PropertyRow* it_row = rowFromPath(*it);
		ASSERT(it_row->refCount() > 0 && it_row->refCount() < 0xFFFF);
		selection_.erase(it);
	}
}

void PropertyTreeModel::deselectAll()
{
	Selection::iterator it;
	FOR_EACH(selection_, it){
		PropertyRow* row = rowFromPath(*it);
		row->setSelected(false);
	}
	selection_.clear();
}

PropertyRow* PropertyTreeModel::rowFromPath(const TreePath& path)
{
	PropertyRow* row = root();
	TreePath::const_iterator it;
	for(it = path.begin(); it != path.end(); ++it){
		int index = it->index;
		if(index < int(row->count()) && index >= 0){
			PropertyRow* nextRow = row->childByIndex(index);
			if(!nextRow)
				return row;
			else
				row = nextRow;
		}
		else
			return row;
	}
	return row;
}

void PropertyTreeModel::setSelection(const Selection& selection)
{
	deselectAll();
	Selection::const_iterator it;
	FOR_EACH(selection, it){
		const TreePath& path = *it;
		PropertyRow* row = rowFromPath(path);
		if(row)
			selectRow(row, true, false);
	}
}

void PropertyTreeModel::clear()
{
	if(root_)
		root_->clear();
	root_ = 0;
	setRoot(new PropertyRow("", "root", ""));
	selection_.clear();
}

void PropertyTreeModel::onUpdated()
{
    signalUpdated().emit();
}

void PropertyTreeModel::applyOperator(PropertyTreeOperator* op, bool createRedo)
{
    ESCAPE(op, return);
    PropertyRow *dest = rowFromPath(op->path_);
    ESCAPE(dest && "Unable to apply operator!", return);
    if(op->type_ == PropertyTreeOperator::NONE)
        return;
    ESCAPE(op->row_, return);
    if(dest->parent())
        dest->parent()->replaceAndPreserveState(dest, op->row_, false);
    else{
        op->row_->assignRowProperties(root_);
        root_ = op->row_;
    }
    PropertyRow* newRow = op->row_;
    op->row_ = 0;
    rowChanged(newRow);
    // TODO: redo
}

void PropertyTreeModel::undo()
{
    ESCAPE(!undoOperators_.empty(), return);
    applyOperator(&undoOperators_.back(), true);
    undoOperators_.pop_back();
}

PropertyTreeModel::UpdateLock PropertyTreeModel::lockUpdate()
{
	if(updateLock_)
		return updateLock_;
	else{
		UpdateLock lock = new PropertyTreeModel::LockedUpdate(this);;
		updateLock_ = lock;
		lock->release();
		return lock;
	}
}

void PropertyTreeModel::dismissUpdate()
{
	if(updateLock_)
		updateLock_->dismissUpdate();
}

void PropertyTreeModel::requestUpdate()
{
	if(updateLock_)
		updateLock_->requestUpdate();
	else
		onUpdated();
}

struct RowObtainer {
	RowObtainer(std::vector<char>& states) : states_(states) {}
	ScanResult operator()(PropertyRow* row)
	{
		states_.push_back(row->expanded() ? 1 : 0);
		return row->expanded() ? SCAN_CHILDREN_SIBLINGS : SCAN_SIBLINGS;
	}
protected:
	std::vector<char>& states_;
};

struct RowExpander {
	RowExpander(const std::vector<char>& states) : states_(states), index_(0) {}
	ScanResult operator()(PropertyRow* row, PropertyTree* tree)
	{
		if(size_t(index_) >= states_.size())
			return SCAN_FINISHED;

		if(states_[index_++]){
			if(row->canBeToggled(tree))
				row->_setExpanded(true);
			return SCAN_CHILDREN_SIBLINGS;
		}
		else{
			row->_setExpanded(false);
			return SCAN_SIBLINGS;
		}
	}
protected:
	int index_;
	const std::vector<char>& states_;
};

void PropertyTreeModel::serialize(Archive& ar, PropertyTree* tree)
{
	if(ar.filter(SERIALIZE_STATE)){
		ar.serialize(focusedRow_, "focusedRow", 0);		
		ar.serialize(selection_, "selection", 0);

		std::vector<char> expanded;
		if(ar.isOutput())
			root()->scanChildren(RowObtainer(expanded));
		ar.serialize(expanded, "expanded", 0);
		if(ar.isInput()){
			Selection sel = selection_;
            setSelection(sel);
			root()->scanChildren(RowExpander(expanded), tree);
		}
	}
}

void PropertyTreeModel::pushUndo(const PropertyTreeOperator& op)
{
    PropertyTreeOperator oper = op;
    bool handled = false;
    signalPushUndo_.emit(&oper, &handled);
    if(!handled && oper.row_ != 0)
        undoOperators_.push_back(oper);
}

void PropertyTreeModel::push(PropertyRow* row)
{
    ESCAPE(row, return);
    if(fullUndo_){
        if(undoEnabled_){
            PropertyRow* clonedRow = root()->clone();
            clonedRow->assignRowState(*root(), true);
            pushUndo(PropertyTreeOperator(TreePath(), clonedRow));
        }
        else{
            pushUndo(PropertyTreeOperator(TreePath(), 0));
        }
    }
    else{
        if(undoEnabled_){
            PropertyRow* clonedRow = row->clone();
            clonedRow->assignRowState(*row, true);
            pushUndo(PropertyTreeOperator(pathFromRow(row), clonedRow));
        }
        else{
            pushUndo(PropertyTreeOperator(pathFromRow(row), 0));
        }
    }
}

void PropertyTreeModel::rowChanged(PropertyRow* row)
{
	ESCAPE(row, return);
	row->setMultiValue(false);
	while(row->parent()){
		row->setLabelChanged();
		row = row->parent();
	}
	requestUpdate();
}

bool PropertyTreeModel::defaultTypeRegistered(const char* typeName) const
{
	return defaultTypes_.find(typeName) != defaultTypes_.end();
}

void PropertyTreeModel::addDefaultType(PropertyRow* row, const char* typeName)
{
	ESCAPE(typeName != 0, return);
	defaultTypes_[typeName] = row;
}

PropertyRow* PropertyTreeModel::defaultType(const char* typeName) const
{
	DefaultTypes::const_iterator it = defaultTypes_.find(typeName);
	ESCAPE(it != defaultTypes_.end(), return 0);
	return it->second;
}

void PropertyTreeModel::addDefaultType(PropertyRow* row, const char* baseName, const char* derivedTypeName, const char* derivedTypeNameAlt)
{
	ASSERT(baseName);
	ASSERT(derivedTypeName);
	ASSERT(derivedTypeNameAlt);
	DefaultTypesPoly::iterator it = defaultTypesPoly_.find(baseName);
	BaseClass& base = (it == defaultTypesPoly_.end()) ? defaultTypesPoly_[baseName] : it->second;
	DerivedTypes::iterator dit = base.types.begin();
	while(dit != base.types.end()){
		if(dit->name == derivedTypeName){
			DerivedClass& derived = *dit;
			derived.name = derivedTypeName;
			ASSERT(derived.row == 0);
			derived.row = row;
			return;
		}
		++dit;
	}
	base.types.push_back(DerivedClass());
	DerivedClass& derived = base.types.back();
	derived.name = derivedTypeName;
	derived.row = row;
	base.strings.push_back(derivedTypeNameAlt);
}

PropertyRow* PropertyTreeModel::defaultType(const char* baseName, int derivedIndex) const
{
	DefaultTypesPoly::const_iterator it = defaultTypesPoly_.find(baseName);
	ASSERT(it != defaultTypesPoly_.end());
	const BaseClass& base = it->second;
	ASSERT(derivedIndex >= 0);
	ASSERT(derivedIndex < int(base.types.size()));
	DerivedTypes::const_iterator tit = base.types.begin();
	std::advance(tit, derivedIndex);
	return tit->row;
}

bool PropertyTreeModel::defaultTypeRegistered(const char* baseName, const char* derivedName) const
{
	DefaultTypesPoly::const_iterator it = defaultTypesPoly_.find(baseName);

	if(it != defaultTypesPoly_.end()){
		if(!derivedName)
			return true;
		const BaseClass& base = it->second;

		DerivedTypes::const_iterator dit;
		for(dit = base.types.begin(); dit != base.types.end(); ++dit){
			if(dit->name == derivedName)
				return true;
		}
		return false;
	}
	else
		return false;
}

const StringList& PropertyTreeModel::typeStringList(const char* baseTypeName) const
{
	DefaultTypesPoly::const_iterator it = defaultTypesPoly_.find(baseTypeName);

	static StringList empty;
	ESCAPE(it != defaultTypesPoly_.end(), return empty);
	const BaseClass& base = it->second;
	return base.strings;
}

// ----------------------------------------------------------------------------------

void TreePathLeaf::serialize(Archive& ar)
{
	ar.serialize(index, "", 0);
}

}

bool serialize(yasli::Archive& ar, ww::TreeSelection& value, const char* name, const char* label)
{
	return ar(static_cast<std::vector<ww::TreePath>&>(value), name, label);
}
