#include "window.h"

void WINDOW::create(HINSTANCE thisinstance, KEYS* _keys, char* _key, wchar_t** _shown){
    keys = _keys;
    key = _key;
    shown = _shown;

    windowclass.hInstance = thisinstance;
    windowclass.lpszClassName = CLASSNAME;
    windowclass.lpfnWndProc = windowprocedure;
    //windowclass.style = (CS_DBLCLKS&~WS_VISIBLE) | 0x00020000;
    windowclass.style = (CS_DBLCLKS) | 0x00020000;
    windowclass.cbSize = sizeof( WNDCLASSEX );
    windowclass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    windowclass.hIconSm = LoadIcon( NULL, IDI_APPLICATION );
    windowclass.hCursor  = NULL;
    windowclass.lpszMenuName = NULL;
    windowclass.cbClsExtra = 0;
    windowclass.cbWndExtra = 0;
    windowclass.hbrBackground =  CreateSolidBrush( RGB( 255, 255, 255 ) );

    if (!RegisterClassExW(&windowclass)){ printf("ERROR1"); }

    HWND fgwindow = GetForegroundWindow();

    hwnd = CreateWindowExW( WS_EX_TOOLWINDOW, CLASSNAME, WINDOWTITLE, WS_EX_LAYERED| WS_CLIPSIBLINGS | WS_CHILD,
                           CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, GetDesktopWindow(), NULL, //HWND_DESKTOP
                           thisinstance, NULL );

    if (!hwnd){ printf("ERROR2"); }



    icon = LoadIcon(thisinstance, MAKEINTRESOURCE(2));

    iconData.cbSize = sizeof( NOTIFYICONDATA );
    iconData.uID = 100;
    iconData.hWnd = hwnd;
    //nid.uVersion = NOTIFYICON_VERSION;
    iconData.uCallbackMessage = ICON_MESSAGE;
    iconData.hIcon = icon;
    swprintf(iconData.szTip, L"Frog-keys");
    iconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

    Shell_NotifyIconW( NIM_ADD, &iconData );



    hide();

    SetForegroundWindow( fgwindow );

    modulehandle = GetModuleHandle( NULL );
}
void WINDOW::setRegion(){
    //region = CreateRoundRectRgn(0, 0, width, BUTTON_H + 2*BUTTON_P, BUTTON_P, BUTTON_P);
    HRGN rect = CreateRoundRectRgn(0, 0, width, BUTTON_H + 2*BUTTON_P, 4*BUTTON_P, 4*BUTTON_P);
    //HRGN rect = CreateRoundRectRgn(topLeft.x, topLeft.y, topLeft.x + width, topLeft.y + BUTTON_H + 2*BUTTON_P, BUTTON_P, BUTTON_P);

    POINT p1, p2, p3;

    if (!under){
        p1.x = width/2;
        p1.y = BUTTON_H + 2*BUTTON_P;

        p3 = p2 = p1;

        p1.x -= 4*BUTTON_P;
        p2.x += 4*BUTTON_P;
        p3.y += 4*BUTTON_P;
    }
    const POINT points[3] = {p1,p2,p3};
    HRGN arrow = CreatePolygonRgn(points, 3, WINDING);

    CombineRgn(region, rect, arrow, RGN_OR);

    //DeleteObject(rect);
    //DeleteObject(arrow);

    SetWindowRgn(hwnd, region, true);
}
void WINDOW::setButtons(){
    if (*shown){
        wchar_t str[8];
        for (int i = 0; i < 8; i++){
            if (i < num){
                swprintf(str, L"%lc\n%i", (*shown)[i+1], i+1);
                SetWindowTextW(B[i], str);
            }
            else {
                swprintf(str, L" ");
                SetWindowTextW(B[i], str);
            }
        }
    }
}
void WINDOW::show(){
    if (!(*shown)){ return; }

    num = 0;
    while ((*shown)[num+1]){
        num++;
    }

    printf("num: %i", num);

    if (num){
        GUITHREADINFO info;
        info.cbSize = sizeof(GUITHREADINFO);

        bool b = GetGUIThreadInfo(NULL, &info);

        if (b){ printf("TRUE"); }
        else{ printf("FALSE"); }

        cursorRect = info.rcCaret;

        RECT rectWnd;

        GetWindowRect(info.hwndCaret, &rectWnd);

        cursor.x = rectWnd.left + (cursorRect.left + cursorRect.right)/2;
        cursor.y = rectWnd.top  + (cursorRect.bottom + cursorRect.top)/2;

        int h = (cursorRect.bottom - cursorRect.top)/2;

        printf("l=%i, r=%i, t=%i, b=%i\n", cursorRect.left, cursorRect.right, cursorRect.top, cursorRect.bottom);
        printf("l=%i, r=%i, t=%i, b=%i\n", rectWnd.left, rectWnd.right, rectWnd.top, rectWnd.bottom);
        //printf("x=%i, y=%i\n", cursor.x, cursor.y);
        //printf("h=%i\n", h);

        cursorWnd = info.hwndCaret;
        cursorParentWnd = info.hwndActive;

        width = num*(BUTTON_W + BUTTON_P) - BUTTON_P + 2*BUTTON_O;

        if (!cursorWnd){
            printf("NO WINDOW!");
//            (*key) = '\0';
//            (*shown) = NULL;
//            hide();

            HMONITOR monitor = MonitorFromWindow(cursorParentWnd, MONITOR_DEFAULTTONEAREST);

            MONITORINFO info2;

            GetMonitorInfo(monitor, &info2);

            topLeft.x = info2.rcMonitor.left;
            topLeft.y = info2.rcMonitor.top;

//            topLeft.x = rectWnd.left;
//            topLeft.y = rectWnd.top;

            //return;
        }
        else{
            //SetForegroundWindow(cursorWnd);
            SetForegroundWindow(cursorParentWnd);

            under = (cursor.y < 2*BUTTON_H);

            topLeft.x = cursor.x - width/2;
            topLeft.y = cursor.y + (under)*(h + DISTANCE) - (!under)*(h + BUTTON_H + 2*BUTTON_O + DISTANCE);
        }

        ShowWindow(hwnd, SW_SHOW);

        SetWindowPos(hwnd, HWND_TOPMOST, topLeft.x, topLeft.y,  width, BUTTON_H + 2*BUTTON_O, SWP_SHOWWINDOW);

        setButtons();
        //setRegion();

        //RedrawWindow(hwnd, NULL, region, RDW_INVALIDATE);
        UpdateWindow(hwnd);

        for (int i = 0; i < 8; i++){
            if (i < num){   ShowWindow(B[i], SW_SHOW); }
            else{           ShowWindow(B[i], SW_HIDE); }
            UpdateWindow(B[i]);
        }
    }
}
void WINDOW::hide(){
    ShowWindow(hwnd, SW_HIDE);
    SetWindowPos(hwnd, HWND_TOPMOST, GetSystemMetrics( SM_CXVIRTUALSCREEN ), 0, 20, 20, 0);
    UpdateWindow(hwnd);
}
//void WINDOW::sayString(char* str, char* title){
//    iconData.cbSize = NOTIFYICONDATA_V2_SIZE;
//
//    iconData.uFlags = NIF_INFO;
//    strcpy(iconData.szInfo, str);
//    strcpy(iconData.szInfoTitle, title);
//    iconData.uTimeout = 5000;
//    iconData.dwInfoFlags = NIF_INFO;
//    Shell_NotifyIcon( NIM_MODIFY, &iconData );
//
//    iconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
//}
