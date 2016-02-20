#ifndef RUBIK_GLOBAL_H
#define RUBIK_GLOBAL_H

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <irrlicht.h>

using namespace std;

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
//#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

#ifdef _DEBUG
#define DATA_DIR "../data/"
#else
#define DATA_DIR "./data/"
#endif

#ifdef _WIN32
#include <Windows.h>
inline unsigned int getMSTime() { return GetTickCount(); }
#else
inline uint32 getMSTime()
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
#endif

inline unsigned int getMSTimeDiff(unsigned int oldMSTime, unsigned int newMSTime)
{
    // getMSTime() have limited data range and this is case when it overflow in this tick
    if (oldMSTime > newMSTime)
        return (0xFFFFFFFF - oldMSTime) + newMSTime;
    else
        return newMSTime - oldMSTime;
}

#endif
