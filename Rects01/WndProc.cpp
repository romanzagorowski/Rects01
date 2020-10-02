#include "framework.h"
#include "resource.h"
#include "About.h"
#include "rectfun.h"

#include <vector>

extern HINSTANCE g_hInstance;

static RECT g_rcDraged = {};
static BOOL g_bCaptured = FALSE;

static std::vector<RECT> g_rects;

static HDC      g_hdcMem = NULL;
static HBITMAP  g_hbmMem = NULL;
static HGDIOBJ  g_hbmOld = NULL;
static UINT     g_uCount = 0;
       UINT     g_uCount2 = 0;

static
void Cls_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    SetRect(&g_rcDraged, x, y, x, y);
    
    SetCapture(hwnd);

    g_bCaptured = TRUE;
}

static
void Cls_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    if(g_bCaptured)
    {
        ReleaseCapture();
        g_bCaptured = FALSE;

        RECT rcNormalized = {};
        CopyRect(&rcNormalized, &g_rcDraged);
        
        NormalizeRect(rcNormalized);

        if(!IsRectEmpty(&rcNormalized))
        {
            g_rects.push_back(rcNormalized);
        }
    }
}

static
void Cls_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
    if(g_bCaptured)
    {
        g_rcDraged.right = x;
        g_rcDraged.bottom = y;

        InvalidateRect(hwnd, NULL, TRUE);
    }
}

static
void PaintRects(HDC hdc, HBRUSH hbr)
{
    for(size_t i = 0; i < g_rects.size(); i++)
    {
        FrameRect(hdc, &g_rects[i], hbr);
    }
}

static
void
PaintCurrentlyDragedRect(HDC hdc, HBRUSH hbr)
{
    if(g_bCaptured)
    {
        RECT rcNormalized = {};
        CopyRect(&rcNormalized, &g_rcDraged);
        
        NormalizeRect(rcNormalized);

        FrameRect(hdc, &rcNormalized, hbr);
    }
}

static
void Cls_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps = {};

    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rcClient = {};
    GetClientRect(hwnd, &rcClient);

    /*
    HDC hdcMem = CreateCompatibleDC(hdc);
    _ASSERT(hdcMem);

    HBITMAP hbmMem = CreateCompatibleBitmap(hdc, GetRectWidth(rcClient), GetRectHeight(rcClient));
    _ASSERT(hbmMem);

    HGDIOBJ hbmOld = SelectObject(hdcMem, hbmMem);
    */

    {
        HBRUSH hbrBack = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
        _ASSERT(hbrBack);

        FillRect(g_hdcMem, &rcClient, hbrBack);

        DeleteObject(hbrBack);
    }

    HBRUSH hbr = GetStockBrush(BLACK_BRUSH);

    PaintRects(g_hdcMem, hbr);
    PaintCurrentlyDragedRect(g_hdcMem, hbr);

    BitBlt(
        hdc,
        rcClient.left, rcClient.top,
        GetRectWidth(rcClient), GetRectHeight(rcClient),
        g_hdcMem,
        0, 0,
        SRCCOPY
    );

    /*
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    */

    EndPaint(hwnd, &ps);
}

static
BOOL Cls_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    return TRUE;
}

static
void DeleteMemDC()
{
    if(g_hdcMem)
    {
        if(g_hbmOld)
        {
            SelectObject(g_hdcMem, g_hbmOld);
            g_hbmOld = NULL;
        }

        if(g_hbmMem)
        {
            DeleteObject(g_hbmMem);
            g_hbmMem = NULL;
        }

        DeleteDC(g_hdcMem);
        g_hdcMem = NULL;

        g_uCount2--;
    }
}

static
void
CreateMemDC(HWND hwnd, int cx, int cy)
{
    HDC hdc = GetDC(hwnd);
    _ASSERT(hdc);

    g_hdcMem = CreateCompatibleDC(hdc);
    _ASSERT(g_hdcMem);

    g_hbmMem = CreateCompatibleBitmap(hdc, cx, cy);
    _ASSERT(g_hbmMem);

    g_hbmOld = SelectObject(g_hdcMem, g_hbmMem);

    g_uCount++;
    g_uCount2++;
}

static
void Cls_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    DeleteMemDC();
    CreateMemDC(hwnd, cx, cy);

    WCHAR szTitle[100] = {};
    swprintf_s(szTitle, 100, L"Rects01 - { cx: %d, cy: %d } - %u, %u", cx, cy, g_uCount, g_uCount2);

    SetWindowText(hwnd, szTitle);
}

static
void Cls_OnDestroy(HWND hwnd)
{
    DeleteMemDC();

    PostQuitMessage(0);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        HANDLE_MSG(hwnd, WM_LBUTTONDOWN, Cls_OnLButtonDown);
        HANDLE_MSG(hwnd, WM_LBUTTONUP, Cls_OnLButtonUp);
        HANDLE_MSG(hwnd, WM_MOUSEMOVE, Cls_OnMouseMove);
        HANDLE_MSG(hwnd, WM_PAINT, Cls_OnPaint);
        HANDLE_MSG(hwnd, WM_ERASEBKGND, Cls_OnEraseBkgnd);
        HANDLE_MSG(hwnd, WM_SIZE, Cls_OnSize);
        HANDLE_MSG(hwnd, WM_DESTROY, Cls_OnDestroy);
//        HANDLE_ WM_DPICHANGED

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch(wmId)
        {
        case IDM_ABOUT:
            DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hwnd);
            break;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
    }
    break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}
