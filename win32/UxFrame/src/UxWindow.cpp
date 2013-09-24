#include "UxWindow.hpp"
#include "UxUtil.hpp"
#include "UxApp.hpp"
#include "UxDC.hpp"

namespace Ux {

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
Window::Window(UINT id) : hWnd_(0)
{
	HINSTANCE hInst = App::instance().getHINSTANCE();
	WinClassMaker wcm(hInst, "UxWindow", WndProc);
	wcm.registerMe();

	WinMaker wm(hInst, "UxWindow");
	hWnd_ = wm.create("Hello UxWindow!", this);

	bgImage_.load(id);
}

Window::~Window()
{
}

void Window::show()
{
	draw();

	::ShowWindow(hWnd_, SW_SHOWNORMAL);
	::UpdateWindow(hWnd_);
}

void Window::draw()
{
	WndDC dc(hWnd_);
	MemoryDC memdc(dc, bgImage_.width(), bgImage_.height());

	// draw window && children
	{
		Gdiplus::Graphics g(memdc);
		g.DrawImage(bgImage_, 0, 0, bgImage_.width(), bgImage_.height());

		for (auto it = children_.cbegin(); it != children_.cend(); ++it)
		{
			if (!it->visible()) continue;
			it->onDraw(graph);
		}
	}

	// blend to system
	BLENDFUNCTION blend;
	ZeroMemory(&blend, sizeof(blend));
	blend.BlendOp = AC_SRC_OVER;
	blend.AlphaFormat = AC_SRC_ALPHA;
	blend.SourceConstantAlpha = 255;

	SIZE size;
	size.cx = bgImage_.width();
	size.cy = bgImage_.height();
	POINT src = {0, 0};

	UpdateLayeredWindow(hWnd_, dc, NULL, &size, memdc, &src, RGB(255, 255, 255), &blend, ULW_ALPHA);
}

void Window::onDestroy()
{
	bgImage_.release(); // make sure GdiPlusBitmap release before gdiPlusShutdown()
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static Window *pWin = nullptr;
	switch (message)
	{
	case WM_NCCREATE:
		{
		CREATESTRUCT *cs = reinterpret_cast<CREATESTRUCT *>(lParam);
		pWin = reinterpret_cast<Window *>(cs->lpCreateParams);
		}
		break;

	case WM_DESTROY:
		pWin->onDestroy();
		::PostQuitMessage(0);
		return 0;
	/*
	case WM_KEYDOWN:
		{
		KeyPressWin32 kPress(wParam);
		if (pWin->GetController().get())
			pWin->GetController()->OnKeyDown(kPress.GetKey());
		}
		return 0;

	case WM_LBUTTONDOWN:
		{
		POINTS p = MAKEPOINTS(lParam);
		KeyStateWin32 kState(wParam);
		if (pWin->GetController().get())
			pWin->GetController()->OnLButtonDown(p.x, p.y, kState);
		}
		return 0;

	case WM_MOUSEMOVE:
		{
		POINTS p = MAKEPOINTS(lParam);
		KeyStateWin32 kState(wParam);
		if (pWin->GetController().get())
			pWin->GetController()->OnMouseMove(p.x, p.y, kState);
		}
		return 0;
	*/
	}

	return DefWindowProc (hWnd, message, wParam, lParam) ;
}

}