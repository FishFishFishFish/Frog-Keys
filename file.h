#ifndef FILE_H
#define FILE_H

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

int loadFile(char* name, std::vector<wchar_t*>* out, wchar_t** command);

struct KEYS{
    std::vector<wchar_t*> lower;
    std::vector<wchar_t*> upper;

    wchar_t* command;

    int load(char* lowerf, char* upperf);
    void search(bool isUpper, int key, wchar_t** shown);
};

#endif // FILE_H
