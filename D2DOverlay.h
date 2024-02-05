#pragma once

#include <d2d1.h>

class D2DOverlay {
public:
    D2DOverlay(HWND hWnd);
    ~D2DOverlay();

    void BeginDraw();
    void EndDraw();

    void DrawBox(float x, float y, float width, float height, D2D1::ColorF color);

private:
    ID2D1Factory* d2dFactory;
    ID2D1HwndRenderTarget* renderTarget;
};
