#include "Application.h"

int main(int argc, char** argv)
{
    // init application
    if (!sApplication->Init(argc, argv))
        return 1;

    // run !
    return sApplication->Run();
}
