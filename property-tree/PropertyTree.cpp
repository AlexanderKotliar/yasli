#include "StdAfx.h"
#include <wx/dc.h>
#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/settings.h>
#include "PropertyTree.h"
#include "PropertyOArchive.h"
#include "PropertyIArchive.h"
#include "PopupMenu.h"


PropertyItem* PropertyItemPath::get(PropertyItem& root) const
{
    Indices::const_iterator it = indices_.begin();

    PropertyItem* current = &root;
    while(it != indices_.end()){
		current = current->findByIndex(*it);
		if(!current)
			return 0;
		++it;
    }
	return current;
}

void PropertyItemPath::set(PropertyItem& root, PropertyItem* item)
{
	indices_.clear();
	if(item == 0)
		return;

	typedef std::vector<const PropertyItem*> Items;
	Items items;
	
    const PropertyItem* current = item;
	while(current != &root){
		items.push_back(current);
		current = current->parent();
		if(current == 0)
			return;
	}

	Items::reverse_iterator it = items.rbegin();
	current = &root;
	while(it != items.rend()){
		int index = current->findIndexOf( *it );
		ASSERT(index >= 0);
		indices_.push_back(index);
		current = *it;
		++it;
	}

	ASSERT(get(root) == item);
}


// ---------------------------------------------------------------------------

DEFINE_EVENT_TYPE(wxEVT_PROPERTY_TREE_CANCEL_CONTROL)

IMPLEMENT_DYNAMIC_CLASS(PropertyTree, wxScrolledWindow)

BEGIN_EVENT_TABLE (PropertyTree, wxScrolledWindow)
  EVT_LEFT_DOWN   (PropertyTree::onMouseClick)
  EVT_LEFT_DCLICK (PropertyTree::onMouseClick)
  EVT_RIGHT_DOWN  (PropertyTree::onRightMouseClick)
  EVT_PAINT       (PropertyTree::onPaint)
  EVT_SIZE        (PropertyTree::onSize)
  EVT_SET_FOCUS   (PropertyTree::onSetFocus)
  EVT_KILL_FOCUS  (PropertyTree::onKillFocus)
  EVT_KEY_DOWN    (PropertyTree::onKeyDown)
  EVT_COMMAND     (wxID_ANY, wxEVT_PROPERTY_TREE_CANCEL_CONTROL, PropertyTree::onCancelControl)
END_EVENT_TABLE()

PropertyTree::PropertyTree(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                           const wxValidator& validator, const wxString& name)
: redrawing_(false)
{

    root_.setExpanded(true);
    signalChanged().connect(this, &PropertyTree::onChanged);
    
    wxScrolledWindow::Create(parent, id, pos, size,
                             wxBORDER_NONE | wxTAB_TRAVERSAL, name);
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

PropertyTree::~PropertyTree()
{
}

void PropertyTree::serialize(Archive& ar)
{
    int oldFilter = ar.getFilter();
    ar.setFilter(PropertyItem::SERIALIZE_STATE);
    ar(root_, "root");
    ar.setFilter(oldFilter);

}

void PropertyTree::attach(Serializer s)
{
    attached_ = s;
    revert();
}

void PropertyTree::detach()
{
    attached_ = Serializer();
    revert();
}

void PropertyTree::revert()
{
    //setFocusedItem(0);
	cancelControl(true, false);
	if(attached_){
		// FIXME: ���� ������ ��� ������, �� ���������� heap corruption!
		PropertyOArchive ar(root_);
		ar(attached_, "");
	}
	else
		root_.clear();
    Refresh();
}

void PropertyTree::apply()
{
	PropertyIArchive ar(root_);
	ar(attached_, "");
}

void PropertyTree::onPaint(wxPaintEvent &event)
{
    wxAutoBufferedPaintDC paintDC(this);
	DoPrepareDC(paintDC);
	redraw(paintDC);
}

void PropertyTree::deselectAll(PropertyItem* root)
{
    if(!root)
        root = this->root();
    ASSERT(root);
    root->setSelected(false);
    PropertyItem::iterator it;
    FOR_EACH(*root, it){
        PropertyItem* item = *it;
        deselectAll(item);
    }
}

static void expandDirtyWorker(PropertyItem* root, int levelsToExpand)
{
	root->setExpanded(true);
	if(levelsToExpand == 1)
		return;
    PropertyItem::iterator it;
    FOR_EACH(*root, it){
        PropertyItem* item = *it;
		expandDirtyWorker(item, levelsToExpand ? levelsToExpand - 1 : 0);
	}
}

void PropertyTree::expandAll(int onlyFirstLevels)
{
	expandDirtyWorker(root(), onlyFirstLevels ? onlyFirstLevels + 1 : 0);
    Refresh();
}

void PropertyTree::onChanged(Refrigerator* changer)
{
    Refresh();
}

void PropertyTree::toggle(PropertyItem* item)
{
    signalChanged().freeze(this);
    item->setExpanded(!item->expanded());
    signalChanged().unfreeze();
}

void PropertyTree::select(PropertyItem* item, bool exclusive)
{
    signalChanged().freeze(this);
    if(exclusive){
        deselectAll();
        setFocusedItem(item);
    }
    item->setSelected(true);
    signalChanged().unfreeze();
}

PropertyItem* PropertyTree::focusedItem()
{
    return focusedItemPath_.get(root_); 
}

void PropertyTree::setFocusedItem(PropertyItem* focusedItem)
{
	focusedItemPath_.set(root_, focusedItem);
}

PropertyFilter& PropertyTree::currentFilter()
{
	static PropertyFilter dummy;
	return dummy;
}

void PropertyTree::initViewContext(PropertyItem::ViewContext& context, PropertyItem* forItem)
{
	if(root_.layoutChanged()){
        root_.calculateHeight(currentFilter(), 0);
		SetVirtualSize(0, root_.height());
		SetScrollRate(0, 20);
	}
	int totalHeight = root_.height();

    context.tree = this;
    context.rect = context.visibleRect = GetClientRect();
    context.rect.height = totalHeight;
	
	wxWindow* focusedWindow = wxWindow::FindFocus();
	bool controlFocused = (focusedWindow == this) || (spawnedControl_ && spawnedControl_->get() == focusedWindow);
    context.focusedItem = controlFocused ? focusedItem() : 0;

	if(forItem != 0){
		typedef std::vector<PropertyItem*> Parents;	
		Parents parents;
		PropertyItem* parent = forItem;
		while(parent->parent()){
	        parents.push_back(parent);
			parent = parent->parent();
		}
		if(!parents.empty()){
			PropertyItem::ViewContext childContext = context;
			Parents::reverse_iterator it;
			for(it = parents.rbegin(); it != parents.rend(); ++it){
				PropertyItem* item = *it;
				item->prepareContext(childContext, item->parent(), context);
				context = childContext;
			}
			//context = childContext;
		}
	}
}

void PropertyTree::redraw(wxDC& dc)
{    
	if(redrawing_)
		return;
	redrawing_ = true;

    PropertyItem::ViewContext context;
	wxRect clientRect = GetClientRect();
	clientRect.SetPosition(CalcUnscrolledPosition(clientRect.GetPosition()));
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
	dc.DrawRectangle(clientRect);
    initViewContext(context, &root_);

    root_.redraw(dc, context);

	redrawing_ = false;
}

void PropertyTree::onMouseClick(wxMouseEvent& event)
{
    cancelControl();
	SetFocus();

    PropertyItem::ViewContext context;
    initViewContext(context, &root_);
    
    wxPoint position = event.GetPosition(); //ScreenToClient(event.GetPosition());
	position = CalcUnscrolledPosition(position);
    std::cout << position.x << ", " << position.y << std::endl;
    root_.onMouseClick(position, event.ButtonDClick(), context);
}

void PropertyTree::onRightMouseClick(wxMouseEvent& event)
{
	cancelControl();
	SetFocus();

    PropertyItem::ViewContext context;
    initViewContext(context, &root_);
    
    wxPoint position = event.GetPosition(); 
	position = CalcUnscrolledPosition(position);

    root_.onRightMouseClick(position, context);
}

void PropertyTree::onSize(wxSizeEvent& event)
{
    cancelControl();
    Refresh();
}


void PropertyTree::onSetFocus(wxFocusEvent& event)
{
    Refresh();
}

void PropertyTree::onKillFocus(wxFocusEvent& event)
{
    cancelControl();
    Refresh();
}

void PropertyTree::onKeyDown(wxKeyEvent& event)
{
    PropertyItem* itemToSelect = 0;
    PropertyItem* focusedItem = this->focusedItem();
	if(!focusedItem){
		if(root()->begin() != root()->end()){
			focusedItem = *root()->begin();
			setFocusedItem(focusedItem);
		}
	}
    if(!focusedItem->parent())
        return; // TODO: select first
    switch(event.GetKeyCode())
    {
    case WXK_UP:
        itemToSelect = focusedItem->parent()->findSibling(focusedItem, DIR_UP);
        break;
    case WXK_DOWN:
        itemToSelect = focusedItem->parent()->findSibling(focusedItem, DIR_DOWN);
        break;
	case WXK_LEFT:
        itemToSelect = focusedItem->parent()->findSibling(focusedItem, DIR_LEFT);
		if(!itemToSelect){
			if(focusedItem->expanded()) // FIXME: can be collapsed
				focusedItem->setExpanded(false);
			else
				itemToSelect = focusedItem->parent();
		}
		break;
	case WXK_RIGHT:
        itemToSelect = focusedItem->parent()->findSibling(focusedItem, DIR_RIGHT);
		if(!itemToSelect){
			if(!focusedItem->empty()){
				if(!focusedItem->expanded()) // FIXME: can be collapsed
					focusedItem->setExpanded(true);
				else
					itemToSelect = *focusedItem->begin();
			}
		}
		break;
	case WXK_SPACE:
		{
		PropertyItem::ViewContext context;
		initViewContext(context, focusedItem);
		focusedItem->activate(context);
		}
		break;
    default:
        break;
    }
    if(itemToSelect && itemToSelect != root()){
        cancelControl();
        deselectAll();
        itemToSelect->setSelected(true);
        setFocusedItem(itemToSelect);
    }
    Refresh();
}

void PropertyTree::cancelControl(bool now, bool commit)
{
    if(now){
        if(spawnedControl_){
			if(commit)
				spawnedControl_->commit();
            spawnedControl_ = 0;
		    SetFocus();
        }
    }
    else{
        wxCommandEvent event(wxEVT_PROPERTY_TREE_CANCEL_CONTROL, GetId());
        event.SetEventObject(this);
        GetEventHandler()->AddPendingEvent(event);
    }
}

void PropertyTree::commitChange(PropertyItem* changedOne)
{
	apply();
	revert();
	Refresh();
}

void PropertyTree::referenceFollowed(LibrarySelector& selector)
{
	signalReferenceFollowed_.emit(selector);
}

void PropertyTree::onCancelControl(wxCommandEvent& event)
{
    cancelControl(true);
}

void PropertyTree::spawnContextMenu(const PropertyItem::ViewContext& context, PropertyItem* item)
{
	::PopupMenu menu;
	PopupMenuItem& root = menu.root();
	item->onContextMenu(menu, this, PropertyItem::MENU_SECTION_MAIN);
	if(!root.empty())
		root.addSeparator();
	item->onContextMenu(menu, this, PropertyItem::MENU_SECTION_CLIPBOARD);
	menu.root().add("Copy").setSensitive(false);
	menu.root().add("Paste").setSensitive(false);
    item->onContextMenu(menu, this, PropertyItem::MENU_SECTION_DESTRUCTIVE);
    menu.spawn(this);
}

void PropertyTree::spawnInPlaceControl(const PropertyItem::ViewContext& context, PropertyWithControl* property)
{
    std::cout << "Spawning control..." << std::endl;
    cancelControl();
    spawnedControl_ = property->createControl(context);
}
