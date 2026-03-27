
#include "MainApp.h"

#ifdef WIN32
#include <windows.h>
#include <stringapiset.h>
#endif


// Main code
int main(int argc, char** argv)
{
#ifdef WIN32
    system("chcp 65001");
    SetConsoleOutputCP(CP_UTF8);
    setbuf(stdout, nullptr);
#endif
    MainApp::run();
    return 0;
}
