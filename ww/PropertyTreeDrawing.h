#pragma once

namespace Gdiplus{
	class Color;
    class Graphics;
    class Brush;
    class Rect;
    class Font;
	class GraphicsPath;
}

namespace ww{

	void drawingInit();
	void drawingFinish();
	void createRoundRectanglePath(Gdiplus::GraphicsPath* path, const Gdiplus::Rect &_rect, int roundness);
	void fillRoundRectangle(Gdiplus::Graphics* gr, Gdiplus::Brush* brush, const Gdiplus::Rect& r, const Gdiplus::Color& borderColor, int radius);
    void drawRoundRectangle(Gdiplus::Graphics* graphics, const Gdiplus::Rect &_r, unsigned int color, int radius, int width);
    void drawCheck(Gdiplus::Graphics* gr, const Gdiplus::Rect& rect, bool checked);
    void drawEdit(Gdiplus::Graphics* gr, const Gdiplus::Rect& rect, const wchar_t* text, const Gdiplus::Font* font, bool pathEllipsis, bool grayBackground);

    Gdiplus::Font* propertyTreeDefaultFont();
    Gdiplus::Font* propertyTreeDefaultBoldFont();

}
