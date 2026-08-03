// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <windows.h>
#include <ole2.h>
#include <msctf.h>
#include <vector>
#include <string>

interface ITfContext;
interface ITfComposition;

class CCandidateWindow
{
public:
    static CCandidateWindow& GetInstance();

    CCandidateWindow();
    ~CCandidateWindow();

    BOOL Create(HWND hParent = NULL);
    void Destroy();

    void Show();
    void Hide();
    BOOL IsVisible() const { return _visible; }

    void SetCandidates(const std::vector<std::wstring>& candidates);
    void ClearCandidates();
    void SetSelectedIndex(int index);
    int GetSelectedIndex() const { return _selectedIndex; }
    size_t GetCandidateCount() const { return _candidates.size(); }
    std::wstring GetCandidate(size_t index) const;

    void Move(int x, int y);
    void MoveToComposition(TfEditCookie ec, ITfContext* pContext, ITfComposition* pComposition);

    void SelectNext();
    void SelectPrev();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnPaint(HWND hwnd);
    void RecalculateSize();

    HWND _hwnd;
    std::vector<std::wstring> _candidates;
    int _selectedIndex;
    bool _visible;
    HFONT _hFontCandidate;

    int _width;
    int _height;

    static const WCHAR* WINDOW_CLASS_NAME;
};
