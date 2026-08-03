#include "../core/Private.h"
#include "CandidateWindow.h"
#include "../core/Globals.h"

const WCHAR* CCandidateWindow::WINDOW_CLASS_NAME = L"BornoTSFCandidateWindow";

CCandidateWindow& CCandidateWindow::GetInstance()
{
    static CCandidateWindow instance;
    return instance;
}

CCandidateWindow::CCandidateWindow()
    : _hwnd(NULL)
    , _selectedIndex(0)
    , _visible(false)
    , _hFontCandidate(NULL)
    , _width(200)
    , _height(150)
{
}

CCandidateWindow::~CCandidateWindow()
{
    Destroy();
}

BOOL CCandidateWindow::Create(HWND hParent)
{
    if (_hwnd != NULL) return TRUE;

    HINSTANCE hInst = Global::dllInstanceHandle;

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW | CS_IME;
    wc.lpfnWndProc = CCandidateWindow::WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = WINDOW_CLASS_NAME;

    RegisterClassExW(&wc);

    _hwnd = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        WINDOW_CLASS_NAME,
        L"Borno Candidate Window",
        WS_POPUP,
        0, 0, _width, _height,
        hParent,
        NULL,
        hInst,
        this
    );

    if (!_hwnd) return FALSE;

    _hFontCandidate = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, NULL);

    return TRUE;
}

void CCandidateWindow::Destroy()
{
    if (_hwnd)
    {
        DestroyWindow(_hwnd);
        _hwnd = NULL;
    }
    if (_hFontCandidate)
    {
        DeleteObject(_hFontCandidate);
        _hFontCandidate = NULL;
    }
    _visible = false;
}

void CCandidateWindow::Show()
{
    if (!_hwnd) Create();
    if (_hwnd && !_candidates.empty())
    {
        RecalculateSize();
        ShowWindow(_hwnd, SW_SHOWNOACTIVATE);
        SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        InvalidateRect(_hwnd, NULL, TRUE);
        _visible = true;
    }
}

void CCandidateWindow::Hide()
{
    if (_hwnd && _visible)
    {
        ShowWindow(_hwnd, SW_HIDE);
        _visible = false;
    }
}

void CCandidateWindow::SetCandidates(const std::vector<std::wstring>& candidates)
{
    _candidates = candidates;
    _selectedIndex = 0;
    if (_candidates.empty())
    {
        Hide();
    }
    else
    {
        Show();
    }
}

void CCandidateWindow::ClearCandidates()
{
    _candidates.clear();
    _selectedIndex = 0;
    Hide();
}

void CCandidateWindow::SetSelectedIndex(int index)
{
    if (index >= 0 && index < (int)_candidates.size())
    {
        _selectedIndex = index;
        if (_hwnd && _visible)
        {
            InvalidateRect(_hwnd, NULL, FALSE);
        }
    }
}

std::wstring CCandidateWindow::GetCandidate(size_t index) const
{
    if (index < _candidates.size())
    {
        return _candidates[index];
    }
    return L"";
}

void CCandidateWindow::SelectNext()
{
    if (!_candidates.empty())
    {
        _selectedIndex = (_selectedIndex + 1) % _candidates.size();
        if (_hwnd && _visible) InvalidateRect(_hwnd, NULL, FALSE);
    }
}

void CCandidateWindow::SelectPrev()
{
    if (!_candidates.empty())
    {
        _selectedIndex = (_selectedIndex - 1 + (int)_candidates.size()) % _candidates.size();
        if (_hwnd && _visible) InvalidateRect(_hwnd, NULL, FALSE);
    }
}

void CCandidateWindow::Move(int x, int y)
{
    if (_hwnd)
    {
        SetWindowPos(_hwnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

void CCandidateWindow::MoveToComposition(TfEditCookie ec, ITfContext* pContext, ITfComposition* pComposition)
{
    if (!_hwnd) return;

    RECT rcRange = { 0 };
    BOOL foundPos = FALSE;

    if (pContext && pComposition)
    {
        ITfRange* pRangeComp = nullptr;
        if (SUCCEEDED(pComposition->GetRange(&pRangeComp)) && pRangeComp)
        {
            ITfContextView* pView = nullptr;
            if (SUCCEEDED(pContext->GetActiveView(&pView)) && pView)
            {
                BOOL fClipped = FALSE;
                if (SUCCEEDED(pView->GetTextExt(ec, pRangeComp, &rcRange, &fClipped)))
                {
                    if (rcRange.left != 0 || rcRange.bottom != 0)
                    {
                        foundPos = TRUE;
                    }
                }
                pView->Release();
            }
            pRangeComp->Release();
        }
    }

    if (!foundPos && pContext)
    {
        ITfContextView* pView = nullptr;
        if (SUCCEEDED(pContext->GetActiveView(&pView)) && pView)
        {
            TF_SELECTION sel = { 0 };
            ULONG cFetched = 0;
            if (SUCCEEDED(pContext->GetSelection(ec, 0, 1, &sel, &cFetched)) && cFetched > 0 && sel.range)
            {
                BOOL fClipped = FALSE;
                if (SUCCEEDED(pView->GetTextExt(ec, sel.range, &rcRange, &fClipped)))
                {
                    if (rcRange.left != 0 || rcRange.bottom != 0)
                    {
                        foundPos = TRUE;
                    }
                }
                sel.range->Release();
            }
            pView->Release();
        }
    }

    if (!foundPos)
    {
        GUITHREADINFO gti = { 0 };
        gti.cbSize = sizeof(GUITHREADINFO);
        if (GetGUIThreadInfo(0, &gti) && gti.hwndCaret)
        {
            POINT pt = { gti.rcCaret.left, gti.rcCaret.bottom };
            ClientToScreen(gti.hwndCaret, &pt);
            rcRange.left = pt.x;
            rcRange.top = pt.y;
            rcRange.bottom = pt.y + 20;
            foundPos = TRUE;
        }
    }

    if (!foundPos)
    {
        POINT ptCursor = { 0 };
        GetCursorPos(&ptCursor);
        rcRange.left = ptCursor.x;
        rcRange.top = ptCursor.y;
        rcRange.bottom = ptCursor.y + 20;
    }

    int posX = rcRange.left + 2;
    int posY = rcRange.bottom + 2;

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    if (posX + _width > screenW) posX = screenW - _width - 10;
    if (posY + _height > screenH) posY = rcRange.top - _height - 4;
    if (posX < 0) posX = 0;
    if (posY < 0) posY = 0;

    Move(posX, posY);
}

void CCandidateWindow::RecalculateSize()
{
    if (!_hwnd || _candidates.empty()) return;

    HDC hdc = GetDC(_hwnd);
    HFONT hOldFont = (HFONT)SelectObject(hdc, _hFontCandidate);

    int maxTextWidth = 100;
    int itemHeight = 24;

    for (size_t i = 0; i < _candidates.size() && i < 9; ++i)
    {
        SIZE sz = { 0 };
        GetTextExtentPoint32W(hdc, _candidates[i].c_str(), (int)_candidates[i].length(), &sz);
        if (sz.cx > maxTextWidth) maxTextWidth = sz.cx;
    }

    SelectObject(hdc, hOldFont);
    ReleaseDC(_hwnd, hdc);

    _width = maxTextWidth + 24;
    if (_width < 120) _width = 120;

    int count = (int)_candidates.size();
    if (count > 9) count = 9;
    _height = count * itemHeight + 8;

    SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, _width, _height, SWP_NOMOVE | SWP_NOACTIVATE);
}

void CCandidateWindow::OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    // b fill
    HBRUSH hBgBrush = CreateSolidBrush(RGB(0xF8, 0xF8, 0xF8));
    FillRect(hdc, &rcClient, hBgBrush);
    DeleteObject(hBgBrush);

    // border
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);

    int itemHeight = 24;
    int yPos = 4;

    for (size_t i = 0; i < _candidates.size() && i < 9; ++i)
    {
        RECT rcItem = { 4, yPos, rcClient.right - 4, yPos + itemHeight };

        bool isSelected = ((int)i == _selectedIndex);

        // selected
        if (isSelected)
        {
            HBRUSH hHlBrush = CreateSolidBrush(RGB(0x48, 0x7F, 0xF9));
            FillRect(hdc, &rcItem, hHlBrush);
            DeleteObject(hHlBrush);
        }

        // candidate string
        SetBkMode(hdc, TRANSPARENT);
        HFONT hOldFont = (HFONT)SelectObject(hdc, _hFontCandidate);
        SetTextColor(hdc, isSelected ? RGB(255, 255, 255) : RGB(30, 30, 30));
        RECT rcText = { rcItem.left + 6, rcItem.top, rcItem.right - 4, rcItem.bottom };
        DrawTextW(hdc, _candidates[i].c_str(), -1, &rcText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

        SelectObject(hdc, hOldFont);
        yPos += itemHeight;
    }

    EndPaint(hwnd, &ps);
}

LRESULT CALLBACK CCandidateWindow::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CCandidateWindow* pThis = nullptr;
    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (CCandidateWindow*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else
    {
        pThis = (CCandidateWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    switch (uMsg)
    {
    case WM_PAINT:
        if (pThis) pThis->OnPaint(hwnd);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;

    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}
