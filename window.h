#ifndef WINDOW_H
#define WINDOW_H

#undef __STRICT_ANSI__
#define _UNICODE
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN

// Standard C Libraries
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <vector>

// Disgusting Windows Libraries
#include <windows.h>
#include <winuser.h>
#include <Shlwapi.h>

// Internal dependencies
#include "file.h"

#define BUTTON_W 16
#define BUTTON_H 36
#define BUTTON_P 00
#define BUTTON_O 01
#define DISTANCE 10

#define ICON_MESSAGE    (WM_USER + 1)

#define MENU_NAME 3000
#define MENU_EXIT 3001

#define CLASSNAME       L"KeyPress"
#define WINDOWTITLE     L"KeyPress"

enum{
    B_1 = 101,
    B_2,
    B_3,
    B_4,
    B_5,
    B_6,
    B_7,
    B_8
};

const int B_[8] = {B_1, B_2, B_3, B_4, B_5, B_6, B_7, B_8};

LRESULT CALLBACK windowprocedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );


struct WINDOW{
    int num;
    HWND B[8];

    int width;
    KEYS* keys;
    char* key;
    wchar_t** shown;

    HWND hwnd;
    NOTIFYICONDATAW iconData;
    HICON icon;
    HMENU menu;

    POINT cursor;
    RECT cursorRect;
    HWND cursorWnd;
    HWND cursorParentWnd;
    POINT topLeft;
    bool under;
    //MSG msg;

    WNDCLASSEXW windowclass;
    HINSTANCE modulehandle;
    HRGN region;
    int arrowOff;

    void create(HINSTANCE thisinstance, KEYS* _keys, char* _key, wchar_t** _shown);
    void setRegion();
    void setButtons();
    void show();
    void hide();
//    void sayString(char* str, char* title);
};

#endif // WINDOW_H
