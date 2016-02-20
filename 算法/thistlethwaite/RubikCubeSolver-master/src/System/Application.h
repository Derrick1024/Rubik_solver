#ifndef RUBIK_APPLICATION_H
#define RUBIK_APPLICATION_H

#include "Singleton.h"

class Application
{
    friend class Singleton<Application>;

    public:
        ~Application();

        bool Init(int argc, char** argv);
        int Run();

        bool IsGraphicMode() { return m_graphicMode; };

    private:
        Application();

        bool m_graphicMode;
        bool m_quickMode;
};

#define sApplication Singleton<Application>::instance()

#endif
