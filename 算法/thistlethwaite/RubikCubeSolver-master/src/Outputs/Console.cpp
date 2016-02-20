#include "Global.h"
#include "Console.h"
#include "Rubik.h"

#include <fstream>

// implicit constructor
ConsoleHandler::ConsoleHandler()
{
    m_printOn = true;
}

// initialize console interface
bool ConsoleHandler::Init()
{
    // init empty cube (without using any renderers, etc.)
    sCube->BuildCube(nullptr, nullptr);

    return true;
}

// console command processing method
bool ConsoleHandler::ProcessCommand(std::string &cmd)
{
    // help message - displays all available commands
    if (cmd.compare("help") == 0)
    {
        cout << "List of commands:" << endl;
        cout << "help               - prints this message" << endl;
        cout << "load <file>        - loads cube state from file" << endl;
        cout << "mixup              - randomly mixes current cube" << endl;
        cout << "flip <flip>        - performs specified flip" << endl;
        cout << "solve              - solves current cube" << endl;
        cout << "solve save <file>  - saves solving sequence to file" << endl;
        cout << "print on           - the cube will be printed after eact flip" << endl;
        cout << "print off          - the cube won't be printed" << endl;
        cout << "print              - prints current state of cube" << endl;
        cout << "exit               - exits the application" << endl;
        cout << endl;
    }
    // load command with specified filename to load
    else if (cmd.length() > 4 && cmd.substr(0, 5).compare("load ") == 0)
    {
        // filename must be specified
        if (cmd.length() < 6)
        {
            cout << "No filename specified" << endl;
            return true;
        }

        std::string fname = cmd.substr(5);

        // load cube state from file
        cout << "Loading file: " << fname << endl;
        if (sCube->LoadFromFile((char*)fname.c_str()))
        {
            cout << "File loaded successfully" << endl;
            // if printing is on, print out cube state
            if (m_printOn)
            {
                sCube->PrintOut();
                cout << endl;
            }
        }
        else
            cout << "Could not load cube state from file" << endl;
    }
    // print on command - to turn on cube printing after each step
    else if (cmd.compare("print on") == 0)
    {
        m_printOn = true;
        cout << "Cube printing turned ON" << endl;
    }
    // print off command - to turn off cube printing after each step
    else if (cmd.compare("print off") == 0)
    {
        m_printOn = false;
        cout << "Cube printing turned OFF" << endl;
    }
    // print command - to print current cube state
    else if (cmd.compare("print") == 0)
    {
        sCube->PrintOut();
        cout << endl;
    }
    // mixup command - to randomly mix up the cube
    else if (cmd.compare("mixup") == 0)
    {
        // generate seqence of flips
        std::list<CubeFlip> flist;
        sCube->Scramble(&flist);

        // proceed them
        for (std::list<CubeFlip>::iterator itr = flist.begin(); itr != flist.end(); ++itr)
        {
            sCube->DoFlip(*itr, true);
            // print out, if printing is turned on
            if (m_printOn)
            {
                cout << "Flipping: " << getStrForFlip(*itr) << endl;
                sCube->PrintOut();
                cout << endl;
            }
        }

        cout << "Cube successfully mixed up in " << flist.size() << " flips" << endl;
    }
    // flip command - eighter print flip list, or performs flip
    else if (cmd.length() >= 4 && cmd.substr(0, 4).compare("flip") == 0)
    {
        bool printhelp = true;

        // flip is specified
        if (cmd.length() > 5)
        {
            // get flip (as string, and then retrieve appropriate enum value)
            std::string flipstr = cmd.substr(5);
            CubeFlip fl = getFlipForStr((char*)flipstr.c_str());

            // if this value exists..
            if (fl != FLIP_NONE)
            {
                printhelp = false;

                // perform flip and print out, if printing is on
                sCube->DoFlip(fl, true);
                cout << "Flipping: " << flipstr << endl;
                if (m_printOn)
                {
                    sCube->PrintOut();
                    cout << endl;
                }
            }
        }

        if (printhelp)
        {
            cout << "Syntax: flip <flip to proceed>" << endl;
            cout << "Existing flips: F+, F2, F-, L+, L2, L-, R+, R2, R-, U+, U2, U-, D+, D2, D-, B+, B2, B-" << endl << endl;
        }
    }
    // solve save command - solves cube and saves it to file specified
    else if (cmd.length() > 10 && cmd.substr(0, 11).compare("solve save ") == 0)
    {
        // filename must be specified
        if (cmd.length() <= 11)
        {
            cout << "No filename specified" << endl;
            return true;
        }

        std::string fname = cmd.substr(11);

        cout << "Finding solution..." << endl;

        // find solution (if any)
        std::list<CubeFlip> flist;
        sCube->Solve(&flist);

        // if there is some solution available, proceed
        if (!flist.empty())
        {
            // output file
            ofstream f;
            f.open(fname);
            // may indicate some rights failure, etc.
            if (f.fail() || !f.is_open())
            {
                cerr << "Could not open file " << fname << " for writing!" << endl;
                return true;
            }

            // save flips to file
            for (std::list<CubeFlip>::iterator itr = flist.begin(); itr != flist.end(); ++itr)
            {
                f << getStrForFlip(*itr) << endl;
            }

            f.close();

            cout << "Cube successfully solved in " << flist.size() << " flips" << endl;
        }
        else
        {
            cout << "No solution found for current cube state, or the cube is already solved!" << endl;
        }
    }
    // solve command - to solve 
    else if (cmd.compare("solve") == 0)
    {
        cout << "Finding solution..." << endl;

        std::list<CubeFlip> flist;
        sCube->Solve(&flist);

        if (!flist.empty())
        {
            for (std::list<CubeFlip>::iterator itr = flist.begin(); itr != flist.end(); ++itr)
            {
                sCube->DoFlip(*itr, true);
                if (m_printOn)
                {
                    cout << "Flipping: " << getStrForFlip(*itr) << endl;
                    sCube->PrintOut();
                    cout << endl;
                }
            }

            if (!m_printOn)
            {
                cout << "Flips to solve the cube:" << endl;

                for (std::list<CubeFlip>::iterator itr = flist.begin(); itr != flist.end(); ++itr)
                    cout << getStrForFlip(*itr) << ", ";

                // move back a bit - remove trailing characters ", "
                cout << "\b\b  " << endl;
            }

            cout << "Cube successfully solved in " << flist.size() << " flips" << endl;
        }
        else
        {
            cout << "No solution found for current cube state!" << endl;
        }
    }
    // just exit..
    else if (cmd.compare("exit") == 0)
    {
        return false;
    }
    else
    {
        cout << "Unknown command: " << cmd << endl;
    }

    // "do not exit program"
    return true;
}

// run method, called from main thread to start console interface (nogui option)
void ConsoleHandler::Run()
{
    cout << "Type \"help\" (without quotes) for quick help" << endl << endl;

    // main loop of console interface
    std::string command;
    while (true)
    {
        // prompt, load keyboard input and process
        cout << "> ";
        getline(std::cin, command);

        // if ProcessCommand returns false, exit program
        if (!ProcessCommand(command))
            break;
    }
}
