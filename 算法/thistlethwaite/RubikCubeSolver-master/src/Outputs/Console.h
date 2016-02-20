#ifndef RUBIK_CONSOLE_H
#define RUBIK_CONSOLE_H

#include "Singleton.h"

class ConsoleHandler
{
    friend class Singleton<ConsoleHandler>;
    public:

        bool Init();
        void Run();

    private:
        ConsoleHandler();

        bool m_printOn;

        bool ProcessCommand(std::string &cmd);
};

#define sConsole Singleton<ConsoleHandler>::instance()

#endif
