#include "file.h"

int loadFile(char* name, std::vector<wchar_t*>* out, wchar_t** command=NULL){
    FILE* f = fopen(name, "rb");
    if (!f){ printf("FILE DOES NOT EXIST!"); return 0; }

    wchar_t* str = new wchar_t[16];
    fgetws(str, 16, f);

    if (!(*command)){
        (*command) = new wchar_t[16];
        fgetws(*command, 16, f);
    }
    else{
        fgetws(str, 16, f);
    }

    delete str;

    while (!feof(f)){
        wchar_t* str = new wchar_t[16];
        fgetws(str, 16, f);

        int j = 16;
        while (j){
            j--;
            if (str[j] == 13){ str[j] = '\0'; }
        }

        out->push_back(str);
    }

    fclose(f);

    if ((char)(*out)[0][0] != 'k'){ printf("INVALID FILE!"); return 0; }

    return 1;
}

int KEYS::load(char* lowerf, char* upperf){
    return loadFile(lowerf, &lower, &command) && loadFile(upperf, &upper);
}

void KEYS::search(bool isUpper, int key, wchar_t** shown){
    if (isUpper){
        for (int i = 0; i < upper.size(); i++){
            if ((int)upper[i][0] == key) { *shown = upper[i]; }
        }
    }
    else{
        for (int i = 0; i < lower.size(); i++){
            if ((int)lower[i][0] == key + 32) { *shown = lower[i]; }
        }
    }
}
