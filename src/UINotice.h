#ifndef UINOTICE_H
#define UINOTICE_H

#include "fddEMU.h" //str_error

class UINotice{
    public:
    void show_P(const char* header, const char *notice);
    void show(const char* header, const char *notice) {show_P(header, notice);}
    void error_P(const char *notice) {show_P(str_error, notice);}
    void error(const char *notice) {error_P(notice);}
};

extern class UINotice msg;

#endif //UINOTICE_H