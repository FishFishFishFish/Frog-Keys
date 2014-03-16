#undef __STRICT_ANSI__
#define WINVER 0x0501
#define WIN32_LEAN_AND_MEAN
//#define NOWINABLE

// Standard C Libraries
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <memory.h>
#include <string>
//#include <tchar.h>
#include <wchar.h>
#include <ctime>
#include <cstring>
#include <cctype>
#include <string>
#include <map>
#include <vector>
#include <deque>
#include <stack>

// Disgusting Windows Libraries
#include <windows.h>
#include <winuser.h>

WINUSERAPI BOOL WINAPI GetGUIThreadInfo(DWORD,LPGUITHREADINFO);

//#include <mmsystem.h>
//#include <Windowsx.h>
//#include <commctrl.h>
//#include <shellapi.h>
//#include <Shlwapi.h>
//#include <process.h>
//#include <Winuser.h>
//#include <direct.h>

// Internal dependencies
//#include "window.h"

#define BUTTON_W 18
#define BUTTON_H 40
#define BUTTON_P 02
#define DISTANCE 10

#define CLASSNAME       "KeyPress"
#define WINDOWTITLE     "KeyPress"

std::vector<wchar_t*> keys;
std::vector<wchar_t*> shiftkeys;

bool lshift;
bool rshift;
bool shift;

char key;
wchar_t* shown;

HHOOK kHook;
bool running;
MSG msg;

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

struct WINDOW{
    int num;
    HWND B[8];

    int width;
    int b;

    HWND hwnd;
    POINT cursor;
    HWND cursorWnd;
    POINT topLeft;
    bool under;
    //MSG msg;

    WNDCLASSEX windowclass;
    HINSTANCE modulehandle;
    HRGN region;

    void create(HINSTANCE thisinstance, WNDCLASSEX windowclass);
    void setRegion();
    void setButtons();
    void show();
    void hide();
};

//#include "window.h"

void WINDOW::create(HINSTANCE thisinstance, WNDCLASSEX windowclass){
    HWND fgwindow = GetForegroundWindow();

    hwnd = CreateWindowEx( WS_EX_TOOLWINDOW, CLASSNAME, WINDOWTITLE, WS_POPUP,
                           CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, HWND_DESKTOP, NULL,
                           thisinstance, NULL );

    if (!hwnd){ printf("ERROR2"); }

    hide();

    SetForegroundWindow( fgwindow );

    b = 0;

    modulehandle = GetModuleHandle( NULL );
}
void WINDOW::setRegion(){
    //region = CreateRoundRectRgn(0, 0, width, BUTTON_H + 2*BUTTON_P, BUTTON_P, BUTTON_P);
    HRGN rect = CreateRoundRectRgn(0, 0, width, BUTTON_H + 2*BUTTON_P, BUTTON_P, 4*BUTTON_P);
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
    if (shown){
        wchar_t str[8];
        for (int i = 0; i < 8; i++){
            if (i < num){
                swprintf(str, L"%lc\n%i", shown[i+1], i+1);
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
    if (!shown){ return; }

    num = 0;
    while (shown[num+1]){
        num++;
    }

    printf("num: %i", num);

    if (num){
        GUITHREADINFO info;
        info.cbSize = sizeof(GUITHREADINFO);

        GetGUIThreadInfo(NULL, &info);

        RECT rect = info.rcCaret;

        RECT rectWnd;

        GetWindowRect(info.hwndCaret, &rectWnd);

        cursor.x = rectWnd.left + (rect.left + rect.right)/2;
        cursor.y = rectWnd.top  + (rect.bottom + rect.top)/2;

        int h = (rect.bottom - rect.top)/2;

        printf("l=%i, r=%i, t=%i, b=%i\n", rect.left, rect.right, rect.top, rect.bottom);

        printf("x=%i, y=%i\n", cursor.x, cursor.y);

        printf("h=%i\n", h);

        cursorWnd = info.hwndCaret;

        under = (cursor.y < 200);

        width = num*(BUTTON_W + BUTTON_P) + BUTTON_P;

        topLeft.x = cursor.x - width/2;
        topLeft.y = cursor.y + (under)*(h + BUTTON_P + DISTANCE) - (!under)*(h + BUTTON_H + BUTTON_P + DISTANCE);

//        topLeft.x = 0;
//        topLeft.y = 0;

        ShowWindow(hwnd, SW_SHOW);

        SetWindowPos(hwnd, HWND_TOPMOST, topLeft.x, topLeft.y,  width, BUTTON_H + 2*BUTTON_P, SWP_SHOWWINDOW);

        setButtons();
        //setRegion();

        RedrawWindow(hwnd, NULL, region, RDW_INVALIDATE);
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

    printf("%i", (int)(char)unicode);

    SendInput( 1, &input, sizeof( INPUT ) );

    input.ki.dwFlags |= KEYEVENTF_KEYUP;
    SendInput( 1, &input, sizeof( INPUT ) );

    key = '\0';
    shown = NULL;

    window.hide();
}

LRESULT CALLBACK handlekeys( int code, WPARAM wp, LPARAM lp ) {
    if (code == HC_ACTION && ( (wp == WM_KEYUP) || (wp == WM_KEYDOWN) )){
        char tmp[0xFF] = {'\0'};
        DWORD msg = 1;
        KBDLLHOOKSTRUCT st_hook = *( ( KBDLLHOOKSTRUCT* )lp );

        msg += ( st_hook.scanCode << 16 );
        //msg += ( st_hook.flags << 24 );
        GetKeyNameText( msg, tmp, 0xFF );

        if (wp == WM_KEYUP){

            if      (!strcmp("Shift", tmp)) {       lshift = false; shift = lshift || rshift; }
            else if (!strcmp("Right Shift", tmp)) { rshift = false; shift = lshift || rshift; }
            else if (key == tmp[0] && tmp[1] == '\0') {
                printf( "%s up\n", tmp);
                if (!shown){ key = '\0'; }
            }
        }
        else if (wp == WM_KEYDOWN) {
            if      (!strcmp("Shift", tmp)) {       lshift = true; shift = lshift || rshift; }
            else if (!strcmp("Right Shift", tmp)) { rshift = true; shift = lshift || rshift; }
            else if (shown && tmp[1] == '\0'){
                //printf( "%s down\n", tmp);
                if (key != tmp[0]) {
                    int num = 0;

                    for (int i = 1; i < 9 && shown[i] && keys[0][i]; i++){
                        if ((char)keys[0][i] == tmp[0]){ num = i; printf("HERE!!!"); }
                    }

                    if (!num){
                        key = tmp[0];
                        shown = NULL;

                        window.hide();
                        printf("ENDED!");
                    }
                    else{
                        wprintf(L"%lc", shown[num]);

                        replaceChar(shown[num]);

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
                    if (shift){
                        for (int i = 1; i < shiftkeys.size(); i++){
                            if ((int)shiftkeys[i][0] == (int)key) { shown = shiftkeys[i]; }
                        }
                    }
                    else{
                        for (int i = 1; i < keys.size(); i++){
                            if ((int)keys[i][0] == (int)key + 32) { shown = keys[i]; }
                        }
                    }

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


/*
void sayString( std::string str, std::string title ) {
    if ( bubble ) {
        nid.cbSize = NOTIFYICONDATA_V2_SIZE;

        // Set Version 5 behaviour for balloon feature
        //nid.uVersion = NOTIFYICON_VERSION;
        //Shell_NotifyIcon(NIM_SETVERSION, &nid);

        nid.uFlags = NIF_INFO;
        strcpy( nid.szInfo, str.c_str() );
        strcpy( nid.szInfoTitle, title.c_str() );
        nid.uTimeout = 10000;
        nid.dwInfoFlags = NIF_INFO;
        Shell_NotifyIcon( NIM_MODIFY, &nid );
    }
}
*/

void GUIThread( void* ) {
    POINT p;
    while( running ) {
        if (GetCaretPos(&p)){
            printf("x=%i, y=%i\n", p.x, p.y);
        }
        else{
            printf("Carot not found...\n");
        }

        Sleep(500);

//        time_t current;
//        time(&current);
//
//        double seconds = difftime(current, time);
    }
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

    HWND old;

    int n;

    switch ( msg ) {
        case WM_ERASEBKGND:
            return (LRESULT)1;
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);

//            GetClientRect(hwnd, &rect);
//            region = CreateRectRgnIndirect(&rect);
//            hBrush = CreateSolidBrush(RGB(255,255,255));
//            FillRgn(hdc, region, hBrush);

            FillRgn(hdc, window.region, CreateSolidBrush(RGB(255,255,255)));
            FrameRgn(hdc, window.region, CreateSolidBrush(RGB(200,200,200)),2,2);

            EndPaint(hwnd, &ps);
            break;
        case WM_CLOSE:
            break;
        case WM_CREATE:
            for (int i = 0; i < 8; i++){
                window.B[i] = CreateWindowEx(0, "BUTTON", " ", BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE | BS_MULTILINE,
                                             BUTTON_P + (BUTTON_P + BUTTON_W)*i, BUTTON_P, BUTTON_W, BUTTON_H,
                                             hwnd,(HMENU)B_[i], GetModuleHandle(NULL), NULL);
            }
            break;
//        case WM_SETFOCUS:
//            SetForegroundWindow((HWND)wparam);
//            break;
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
            if (n){ replaceChar(shown[n-1]); }
            break;
        case WM_DESTROY:
            break;
        default:
            return DefWindowProc( hwnd, msg, wparam, lparam );
    };
    return DefWindowProc( hwnd, msg, wparam, lparam );
}

/*

std::vector<std::string> ParseCmdLine( const char* cmdline ) {
    char* buffer = ( char* ) malloc( strlen( cmdline ) + 1 );
    strcpy( buffer, cmdline );
    std::vector<std::string> args;
    int len = strlen( cmdline );
    int pos = 0;
    int isquote = 0;
    int isescaped = 0;
    int isreading = 0;
    for( int i = 0; i <= len; ++i ) {
        if( !isescaped ) {
            if( buffer[i] == '\\' ) {
                isescaped = 1;
                if( !isreading ) {
                    isreading = 1;
                    pos = i;
                }
                continue;
            } else if( buffer[i] == '\"' ) {
                if( isquote ) {
                    buffer[i] = 0;
                    args.push_back( buffer + pos );
                    buffer[i] = ' ';
                    pos = i + 1;
                    isquote = 0;
                    isreading = 0;
                    continue;
                } else {
                    if( isreading ) {
                        buffer[i] = 0;
                        args.push_back( buffer + pos );
                        buffer[i] = ' ';
                    }
                    pos = i + 1;
                    isquote = 1;
                    isreading = 0;
                    continue;
                }
            } else if( isspace( buffer[i] ) || buffer[i] == 0 ) {
                if( !isquote && isreading ) {
                    buffer[i] = 0;
                    args.push_back( buffer + pos );
                    buffer[i] = ' ';
                    isreading = 0;
                }
            } else {
                if( !isreading ) {
                    isreading = 1;
                    pos = i;
                }
            }
        } else {
            int replacement = ' ';
            int newpos = 1;
            switch( buffer[i] ) {
                case 'r': replacement = '\r'; break;
                case 'n': replacement = '\n'; break;
                case 't': replacement = '\t'; break;
                case '\\': replacement = '\\'; break;
                case 'o': sscanf( buffer + i, "o%o%n", &replacement, &newpos ); break;
                case 'x': sscanf( buffer + i, "x%x%n", &replacement, &newpos ); break;
                case ' ': replacement = ' '; break;
            }

            for( int j = 0; newpos + i + j - 1 < len; ++j ) {
                buffer[i + j] = buffer[newpos + i + j];
            }
            len -= newpos;
            buffer[i - 1] = replacement;
            isescaped = 0;
            i -= 1;
            continue;
        }
    }
    if( isreading ) {
        args.push_back( buffer + pos );
    }
    free( buffer );
    return args;
}

int HandleArgs( const char* cmdline ) {
    std::vector< std::string > args = ParseCmdLine( cmdline );
    if ( args.size() == 0 ) {
        return 0;
    }
    HWND parent = FindWindowEx( NULL, NULL, "FrogLies", NULL );
    if ( args[0] == "--window" ) {
        if( parent != NULL ) {
            PostMessage( parent, 59090, 0, 0 );
        } else {
            DoUpload( UPLOAD_WINDOW );
            SetClipboard( whff.GetLastUpload() );
        }
        return 1;
    }
    if ( args[0] == "--screen" ) {
        if( parent != NULL ) {
            PostMessage( parent, 59091, 0, 0 );
        } else {
            DoUpload( UPLOAD_SCREEN );
            SetClipboard( whff.GetLastUpload() );
        }
        return 1;
    }
    if ( args[0] == "--clip" ) {
        if( parent != NULL ) {
            PostMessage( parent, 59092, 0, 0 );
        } else {
            DoUpload( UPLOAD_CLIP );
            SetClipboard( whff.GetLastUpload() );
        }
        return 1;
    }
    if ( args[0] == "--crop" ) {
        int passargs[4];
        if ( args.size() == 5 ) {
            passargs[0] = strtol( args[1].c_str(), NULL, 10 );
            passargs[1] = strtol( args[2].c_str(), NULL, 10 );
            passargs[2] = strtol( args[3].c_str(), NULL, 10 );
            passargs[3] = strtol( args[4].c_str(), NULL, 10 );
            if ( passargs[0] == 0 || passargs[1] == 0 || passargs[2] == 0 || passargs[3] == 0 ) {
                sayString( "Non-int CLI for crop.", "Error" );
            }

            if( parent != NULL ) {
                PostMessage( parent, 59093, passargs[0], passargs[1] );
                PostMessage( parent, 59094, passargs[2], passargs[3] );
            } else {
                Bitmap mb = GetWindow( GetDesktopWindow() );
                mb.Crop( passargs[0], passargs[1], passargs[2], passargs[3] );
                void* data = mb.ReadPNG();
                if( data != 0 ) {
                    Upload( "png", data, mb.PNGLen() );
                    SetClipboard( whff.GetLastUpload() );
                }
            }
            return 1;
        } else {
            printf( "Invalid number of arguments for crop.\n" );
        }
        return 1;
    }
    return 1;
}

*/

int WINAPI WinMain( HINSTANCE thisinstance, HINSTANCE previnstance, LPSTR cmdline, int ncmdshow ) {
    WNDCLASSEX windowclass;

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
    windowclass.hbrBackground =  CreateSolidBrush( RGB( 0, 0, 255 ) );

    if (!RegisterClassEx(&windowclass)){ printf("ERROR1"); }

    window.create(thisinstance, windowclass);

    FILE* f = fopen("keys.txt", "rb");
    if (!f){ printf("FILE DOES NOT EXIST!"); return 0; }

    wchar_t* str = new wchar_t[16];
    fgetws(str, 16, f);

    while (!feof(f)){
        wchar_t* str = new wchar_t[16];
        fgetws(str, 16, f);

        int j = 16;
        while (j){ j--; if (str[j] == 13){ str[j] = '\0'; } }

        keys.push_back(str);
//        printf("\n"); wprintf(str); printf("\n");
//        for (int k = 0; k < 16 && str[k]; k++){ printf("%i, ", (int)str[k]);  }
//        printf("\n");
    }

    fclose(f);

    f = fopen("shift.txt", "rb");
    if (!f){ printf("FILE DOES NOT EXIST!"); return -1; }

    str = new wchar_t[16];
    fgetws(str, 16, f);

    while (!feof(f)){
        str = new wchar_t[16];
        fgetws(str, 16, f);

        int j = 16;
        while (j){ j--; if (str[j] == 13){ str[j] = '\0'; } }

        shiftkeys.push_back(str);
//        printf("\n"); wprintf(str); printf("\n");
//        for (int k = 0; k < 16 && str[k]; k++){ printf("%i, ", (int)str[k]);  }
//        printf("\n");
    }

    fclose(f);

    if ((char)keys[0][0] != 'k'){ printf("INVALID FILE!"); return 0; }
    if ((char)shiftkeys[0][0] != 'K'){ printf("INVALID FILE!"); return -1; }

//    wchar_t str1[256];
//    wchar_t* str2 = str1;
//
//    for (int i = 0; i < keys.size(); i++){
//        str2 += swprintf(str2, L"%ls\n", keys[i]);
//    }
//
//    wprintf(str1);
//
//    MessageBoxW(NULL, str1, L"Testing...", MB_OK);

    window.hide();

    kHook = SetWindowsHookEx( WH_KEYBOARD_LL, (HOOKPROC)handlekeys, window.modulehandle, 0 );

    running = true;

    //sayString( "started...", "Startup" );

    //_beginthread( GUIThread, 1000, NULL );
    //GUIThread(0);

    while ( running ) {
        if ( !GetMessage( &msg, NULL, 0, 0 ) ) {
            break;
        }
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    //Shell_NotifyIcon( NIM_DELETE, &nid );

    return 0;
}

#ifdef DEBUGMALLOCS
#undef malloc(a)
#undef free(a)
#undef realloc(a,b)

void* DMALLOC( size_t a, int line, const char* name ) {
    void *r = malloc( a );
    printf( "MALLOC %X (%d) on line %d of %s\n", r, a, line, name );
    return r;
}
void DFREE( void* a, int line, const char* name ) {
    free( a );
    printf( "FREE %X on line %d of %s\n", a, line, name );
}
void* DREALLOC( void*a, size_t b, int line, const char*name ) {
    void* ret = realloc( a, b );
    printf( "REALLOC from %X to %X (%d) on line %d of %s\n", a, ret, b, line, name );
    return ret;
}
#endif
