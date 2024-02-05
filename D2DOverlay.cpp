#include "D2DOverlay.h"

D2DOverlay::D2DOverlay(HWND hWnd) {
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory);

    D2D1_SIZE_U size = D2D1::SizeU(500, 300); // Set your size here
    d2dFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hWnd, size),
        &renderTarget
    );
}

D2DOverlay::~D2DOverlay() {
    if (renderTarget) {
        renderTarget->Release();
    }
    if (d2dFactory) {
        d2dFactory->Release();
    }
}

void D2DOverlay::BeginDraw() {
    if (renderTarget) {
        renderTarget->BeginDraw();
    }
}

void D2DOverlay::EndDraw() {
    if (renderTarget) {
        renderTarget->EndDraw();
    }
}

void D2DOverlay::DrawBox(float x, float y, float width, float height, D2D1::ColorF color) {
    if (renderTarget) {
        ID2D1SolidColorBrush* brush;
        renderTarget->CreateSolidColorBrush(color, &brush);

        D2D1_RECT_F rect = D2D1::RectF(x, y, x + width, y + height);
        renderTarget->FillRectangle(rect, brush);
    }
}
