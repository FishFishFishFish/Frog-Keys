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
#include "window.h"


bool shift;
bool lshift;
bool rshift;

char key;
wchar_t* shown;
KEYS keys;

HHOOK kHook;
bool running;
MSG msg;

LRESULT CALLBACK windowprocedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

WINDOW window;

void replaceChar(wchar_t unicode){
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    input.ki.wVk = VK_BACK;
    input.ki.dwFlags = 0;
    input.ki.wScan = 14;
    SendInput( 1, &input, sizeof( INPUT ) );
    input.ki.dwFlags |= KEYEVENTF_KEYUP;
    SendInput( 1, &input, sizeof( INPUT ) );

    input.type = INPUT_KEYBOARD;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    input.ki.wVk = 0;
    input.ki.dwFlags = KEYEVENTF_UNICODE;
    input.ki.wScan = unicode;
    SendInput( 1, &input, sizeof( INPUT ) );
    input.ki.dwFlags |= KEYEVENTF_KEYUP;
    SendInput( 1, &input, sizeof( INPUT ) );
}

LRESULT CALLBACK handlekeys( int code, WPARAM wp, LPARAM lp ) {
    if (code == HC_ACTION && ( (wp == WM_SYSKEYUP || wp == WM_KEYUP) || (wp == WM_SYSKEYDOWN || wp == WM_KEYDOWN) )){
        char tmp[0xFF] = {'\0'};
        DWORD msg = 1;
        KBDLLHOOKSTRUCT st_hook = *( ( KBDLLHOOKSTRUCT* )lp );

        msg += ( st_hook.scanCode << 16 );
        //msg += ( st_hook.flags << 24 );
        GetKeyNameText( msg, tmp, 0xFF );

        if (wp == WM_SYSKEYUP || wp == WM_KEYUP){

            if      (!strcmp("Shift", tmp)) {       lshift = false; shift = lshift || rshift; }
            else if (!strcmp("Right Shift", tmp)) { rshift = false; shift = lshift || rshift; }
            else if (key == tmp[0] && tmp[1] == '\0') {
                printf( "%s up\n", tmp);
                if (!shown){ key = '\0'; }
            }
        }
        else if (wp == WM_SYSKEYDOWN || wp == WM_KEYDOWN) {
            if      (!strcmp("Shift", tmp)) {       lshift = true; shift = lshift || rshift; }
            else if (!strcmp("Right Shift", tmp)) { rshift = true; shift = lshift || rshift; }
            else if (shown && tmp[1] == '\0'){
                //printf( "%s down\n", tmp);
                if (key != tmp[0]) {
                    int num = 0;

                    for (int i = 1; i < 9 && shown[i] && keys.command[i]; i++){
                        if ((char)keys.command[i] == tmp[0]){ num = i; printf("HERE!!!"); }
                    }

                    if (!num){
                        key = tmp[0];
                        shown = NULL;

                        window.hide();
                        printf("ENDED!");
                    }
                    else{
                        wprintf(L"%lc", shown[num]);

                        GUITHREADINFO info;
                        info.cbSize = sizeof(GUITHREADINFO);
                        GetGUIThreadInfo(NULL, &info);

                        RECT R = window.cursorRect;
                        RECT r = info.rcCaret;

                        if (window.cursorWnd == info.hwndCaret && R.top == r.top && R.bottom == r.bottom && R.right == r.right && R.left == r.left){ replaceChar(shown[num]); }

                        key = '\0';
                        shown = NULL;

                        window.hide();

                        return 1;
                    }
                }
                else{
                    return 1;
                }
            }
            else if (tmp[1] == '\0'){
                //printf( "%s down\n", tmp);
                if (key != tmp[0]) {
                    key = tmp[0];
                    printf( "%s down\n", tmp);
                }
                else{
//                    bool shif = GetKeyState(VK_RSHIFT) || GetKeyState(VK_LSHIFT);
//
//                    printf("\n-SHIFT: %i, %i\n", GetKeyState(VK_SHIFT) & 0x01, GetKeyState(VK_SHIFT) & 0x02);
//                    printf("RSHIFT: %i, %i\n", GetKeyState(VK_RSHIFT) & 0x01, GetKeyState(VK_RSHIFT) & 0x02);
//                    printf("LSHIFT: %i, %i\n", GetKeyState(VK_LSHIFT) & 0x01, GetKeyState(VK_LSHIFT) & 0x02);

                    bool caps = (GetKeyState(VK_CAPITAL) & 0x01);
                    bool upper = (caps && !shift) || (shift && !caps);

                    keys.search(upper, key, &shown);

                    if (shown){
                        printf("RECOGNIZED!!!\n");
                        window.show();
                        return 1;
                    }
                }
            }
            else{
                key = '\0';
                shown = NULL;

                window.hide();
            }
        }
    }
    //printf("HERE!!!");
    return CallNextHookEx(kHook, code, wp, lp);
}

LRESULT CALLBACK windowprocedure( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
    //printf("HERE!!!");

    POINT curPoint;
    UINT clicked;

    //static int passargs[4] = {0, 0, 0, 0};

    PAINTSTRUCT ps;
    HDC hdc;

    HRGN region;
    RECT rect;
    HBRUSH hBrush;

    int n;

    switch ( msg ) {
//        case WM_ERASEBKGND:
//            return (LRESULT)1;
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);

            GetClientRect(hwnd, &rect);
            region = CreateRectRgnIndirect(&rect);
            hBrush = CreateSolidBrush(RGB(106,162,231));
            FrameRgn(hdc, region, hBrush,1,1);

//            FillRgn(hdc, window.region, CreateSolidBrush(RGB(255,255,255)));
//            FrameRgn(hdc, window.region, CreateSolidBrush(RGB(200,200,200)),2,2);

            EndPaint(hwnd, &ps);
            break;
        case WM_CLOSE:
            running = false;
            DestroyWindow(window.hwnd);
            return 0;
        case WM_CREATE:
            HFONT font;
            font = CreateFontW(14, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, 0, L"Arial");


            for (int i = 0; i < 8; i++){
                window.B[i] = CreateWindowExW(0, L"BUTTON", L" ", BS_TEXT | BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE | BS_MULTILINE,
                                             BUTTON_O + (BUTTON_P + BUTTON_W)*i, BUTTON_O, BUTTON_W, BUTTON_H,
                                             hwnd,(HMENU)B_[i], GetModuleHandle(NULL), NULL);
                SendMessage(window.B[i], WM_SETFONT, (WPARAM) font, 0);
            }

            window.menu = CreatePopupMenu();
            AppendMenu( window.menu, MF_STRING | MF_DISABLED, MENU_NAME, "Frog-Keys" );
            AppendMenu( window.menu, MF_SEPARATOR, 0, NULL );
            AppendMenu( window.menu, MF_STRING, MENU_EXIT, "Exit" );
            break;
        case ICON_MESSAGE:
            switch( lparam ) {
                case WM_RBUTTONDOWN:
                    GetCursorPos( &curPoint ) ;
                    clicked = TrackPopupMenuEx( window.menu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, hwnd, NULL );
                    SendMessage( hwnd, WM_NULL, 0, 0 );

                    switch ( clicked ) {
                        case MENU_EXIT:
                            PostMessage( hwnd, WM_CLOSE, 0, 0 );
                            break;
                        default:
                            return DefWindowProc( hwnd, msg, wparam, lparam );
                    };
                    break;
            };
            break;
        case WM_SETFOCUS:
            SetFocus((HWND)wparam);
            return 0;
        case WM_COMMAND:
            n = 0;
            switch(LOWORD(wparam)) {
                case B_8: n++;
                case B_7: n++;
                case B_6: n++;
                case B_5: n++;
                case B_4: n++;
                case B_3: n++;
                case B_2: n++;
                case B_1: n++;
                break;
            }
            if (n){ replaceChar(shown[n]); }
            break;
        case WM_DESTROY:
            break;
        default:
            return DefWindowProc( hwnd, msg, wparam, lparam );
    };
    return DefWindowProc( hwnd, msg, wparam, lparam );
}

int WINAPI WinMain( HINSTANCE thisinstance, HINSTANCE previnstance, LPSTR cmdline, int ncmdshow ) {
    keys.load("keys.txt", "shift.txt");

    window.create(thisinstance, &keys, &key, &shown);

//    rshift = GetKeyState(VK_RSHIFT);
//    lshift = GetKeyState(VK_LSHIFT);
//    shift = lshift || rshift;
    printf("\nCAPSLOCK: %i\n", GetKeyState(VK_CAPITAL));
    GetKeyState(VK_CAPITAL);

    kHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)handlekeys, window.modulehandle, 0);

    running = true;

    while ( running ) {
        if ( !GetMessage( &msg, NULL, 0, 0 ) ) {
            break;
        }
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    Shell_NotifyIconW(NIM_DELETE, &window.iconData);

    Sleep(500);

    return 0;
}


