#ifndef RUBIK_QUICK_H
#define RUBIK_QUICK_H

#include "Singleton.h"

class QuickHandler
{
    friend class Singleton<QuickHandler>;
    public:

        bool Init(std::string &infile, std::string &outfile);
        void Run();

    private:
        QuickHandler();

        std::string m_outFile;
};

#define sQuickHandler Singleton<QuickHandler>::instance()

#endif
