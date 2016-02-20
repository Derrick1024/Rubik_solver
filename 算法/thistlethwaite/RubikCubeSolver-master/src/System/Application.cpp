#include "Global.h"
#include "Application.h"
#include "Drawing.h"
#include "Console.h"
#include "Quick.h"
#include "Rubik.h"

#include <ctime>

// set implicit values in this constructor
Application::Application()
{
    m_graphicMode = true;
}

Application::~Application()
{
    //
}

// initialize the application with command line args
bool Application::Init(int argc, char** argv)
{
    /*
        Args:
                -i file, --input file       - uses file as input (scrambled cube)
                -o file, --output file      - outputs solution of input cube to this file
                -ng, --nogui                - runs application without gui
                -q, --quick                 - if -i and -o are specified, just processes them and exits
    */

    // some nice info
    cout << "Rubik's Cube solver" << endl;
    cout << "Semestral work for KIV/UIR subject of University of West Bohemia" << endl;
    cout << "Author: Martin Ubl (A13B0453P), 2015" << endl;
    cout << endl;

    std::string infile, outfile;
    bool nogui = false, quick = false;

    // parse arguments...
    if (argc > 1)
    {
        int cur = 1;
        while (cur < argc)
        {
            if (std::string("-i") == argv[cur] || std::string("--input") == argv[cur])
            {
                // load cube configuration
                if (argc > cur + 1)
                {
                    cur++;
                    infile = argv[cur];
                }
            }
            else if (std::string("-o") == argv[cur] || std::string("--output") == argv[cur])
            {
                // store solution to file
                if (argc > cur + 1)
                {
                    cur++;
                    outfile = argv[cur];
                }
            }
            else if (std::string("-ng") == argv[cur] || std::string("--nogui") == argv[cur])
            {
                nogui = true;
            }
            else if (std::string("-q") == argv[cur] || std::string("--quick") == argv[cur])
            {
                quick = true;
            }
            else
            {
                cerr << "Unrecognized input parameter: " << argv[cur] << endl;
            }

            cur++;
        }
    }

    cout << "Using settings: " << endl;
    if (infile.length() > 0)
        cout << "- Input file:  " << infile << endl;
    if (outfile.length() > 0)
        cout << "- Output file: " << outfile << endl;

    cout << "- GUI:         " << (nogui ? "no" : "yes") << endl;
    cout << "- Quick:       " << (quick ? "yes" : "no") << endl;

    if (!nogui && quick)
        cout << "Running without GUI due to -q (--quick) parameter" << endl;

    cout << endl;

    m_graphicMode = !nogui && !quick;
    m_quickMode = quick;

    srand((unsigned int)time(NULL));

    if (m_graphicMode)
    {
        // init Irrlicht rendering engine, and init GUI
        if (!sDrawing->Init())
            return false;
    }
    else if (!m_quickMode)
    {
        // init console gui
        if (!sConsole->Init())
            return false;
    }
    else
    {
        // init quicksolver
        if (!sQuickHandler->Init(infile, outfile))
            return false;
    }

    // load cube if specified input file
    if (infile.length() > 0)
        sCube->LoadFromFile((char*)infile.c_str());

    return true;
}

int Application::Run()
{
    int frames = 99;

    // this stage depends on what type of application flow we chosed
    if (m_graphicMode)
    {
        while (true)
        {
            // render everything
            if (!sDrawing->Render())
                break;

            // each 100 frames, update FPS in window title
            if (++frames == 100)
            {
                core::stringw str = L"Rubik's Cube - KIV/UIR [ ";
                str += (s32)sDrawing->getDriver()->getFPS();
                str += L" FPS ]";

                sDrawing->getDevice()->setWindowCaption(str.c_str());
                frames = 0;
            }
        }
    }
    else if (!m_quickMode)
    {
        // does not run in a loop - retains commands from stdin, etc.
        sConsole->Run();
    }
    else
    {
        // also does not run in a loop - just solves input, puts results into file and closes
        sQuickHandler->Run();
    }

    return 0;
}
