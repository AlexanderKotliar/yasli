#pragma once
#include "ww/_WidgetWithWindow.h"
#include <string>

namespace ww{

class ButtonImpl;
class WW_API Button : public _WidgetWithWindow{
public:
	Button(const char* text = "Button", int border = 0);

	const char* text() const { return text_.c_str(); }
	void setText(const char* text);

	virtual void onPressed();
	sigslot::signal0& signalPressed() { return signalPressed_; }

	void serialize(Archive& ar);
	
	virtual bool canBeDefault() const { return true; }
	virtual void setDefaultFrame(bool enable);
protected:
	// ���������� �������
	ButtonImpl* window() const{ return reinterpret_cast<ButtonImpl*>(_window()); }

	sigslot::signal0 signalPressed_;
	std::string text_;
	bool defaultBtn_;
};

}

