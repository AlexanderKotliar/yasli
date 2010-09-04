#include "StdAfx.h"

#include "ww/HLine.h"
#include "ww/_WidgetWindow.h"
#include <windowsx.h>

#include "ww/Serialization.h"
#include "yasli/TypesFactory.h"

namespace ww{

YASLI_CLASS(Widget, HLine, "Horizontal Line");
#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list

class HLineImpl: public _WidgetWindow{
public:
	HLineImpl(HLine* owner);
	const wchar_t* className() const{ return L"ww.HLine"; }
	void onMessagePaint();
	BOOL onMessageEraseBkgnd(HDC dc);
protected:
	ww::HLine* owner_;
};

#pragma warning(push)
#pragma warning(disable: 4312) // 'type cast' : conversion from 'LONG' to 'HINSTANCE' of greater size

HLineImpl::HLineImpl(HLine* owner)
: _WidgetWindow(owner)
, owner_(owner)
{
	VERIFY(create(L"", WS_CHILD, Recti(0, 0, 42, 42), *Win32::_globalDummyWindow));
}

#pragma warning(pop)

void HLineImpl::onMessagePaint()
{
	PAINTSTRUCT paintStruct;
	HDC paintDC = ::BeginPaint(handle_, &paintStruct);
	{
		HDC dc = paintDC;
		RECT clientRect;
		Recti rect;
		::GetClientRect(*this, &clientRect);
		::FillRect(dc, &clientRect, ::GetSysColorBrush(COLOR_BTNFACE));
		::DrawEdge(dc, &clientRect, EDGE_ETCHED, BF_TOP);
	}
	EndPaint(handle_, &paintStruct);
}

BOOL HLineImpl::onMessageEraseBkgnd(HDC dc)
{
	return FALSE;
}


// --------------------------------------------------------------------------------------------

HLine::HLine(int border)
: _WidgetWithWindow(new HLineImpl(this), border)
{
	_setMinimalSize(Vect2i(GetSystemMetrics(SM_CXEDGE), GetSystemMetrics(SM_CYEDGE)));
}

};


#pragma warning(pop)

