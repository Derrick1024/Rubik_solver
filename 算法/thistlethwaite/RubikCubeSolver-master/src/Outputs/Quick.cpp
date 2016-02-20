#include "Global.h"
#include "Quick.h"
#include "Rubik.h"
#include <fstream>

// implicit constructor - empty
QuickHandler::QuickHandler()
{
    //
}

// initialize everything needed
bool QuickHandler::Init(std::string &infile, std::string &outfile)
{
    // build cube with no renderers
    sCube->BuildCube(nullptr, nullptr);

    // input file is needed
    if (infile.length() == 0)
    {
        cout << "No input file specified, cannot continue." << endl;
        return false;
    }

    // store output filename
    m_outFile = std::string(outfile);

    return true;
}

void QuickHandler::Run()
{
    cout << "Solving cube from input file..." << endl;

    // solve the cube
    std::list<CubeFlip> flist;
    sCube->Solve(&flist);
    if (!flist.empty())
    {
        // if output file specified, write output there
        if (m_outFile.length() > 0)
        {
            // output file
            ofstream f;
            f.open(m_outFile);
            // may indicate some rights failure, etc.
            if (f.fail() || !f.is_open())
            {
                cerr << "Could not open file " << m_outFile << " for writing!" << endl;
                return;
            }

            // save flips to file
            for (std::list<CubeFlip>::iterator itr = flist.begin(); itr != flist.end(); ++itr)
            {
                f << getStrForFlip(*itr) << endl;
            }

            f.close();
        }
        else
        {
            // output flips to console if no output file specified
            cout << "No output file specified, printing to console" << endl;
            cout << "Found solution:" << endl;
            for (std::list<CubeFlip>::iterator itr = flist.begin(); itr != flist.end(); ++itr)
            {
                cout << getStrForFlip(*itr) << ", ";
            }
            cout << "\b\b  " << endl;
        }
    }
    else
    {
        cout << "No solution found, or cube already solved!" << endl;
    }
}
