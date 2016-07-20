/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * TestAppWinMM.cpp
 *      Entry point and other implementation for a simple console application
 *      for testing the functionality of this library via WinMM.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ImportApiWinMM.h"
#include "TestApp.h"
#include "Mapper/Base.h"


using namespace Xidi;


// -------- FUNCTIONS ------------------------------------------------------ //

// Runs the test application. Effectively acts as its entry point.
int RunTestApp(int argc, char* argv[])
{
    MMRESULT result;


    ////////////////////////////////////
    ////////   Initialization

    // Initialize the imported DirectInput8 API.
    if (MMSYSERR_NOERROR != ImportApiWinMM::Initialize())
    {
        terr << _T("Unable to initialize WinMM API.") << endl;
        return -1;
    }

    


    ////////////////////////////////////
    ////////   Cleanup and Exit

    tout << _T("Exiting.") << endl;
    return 0;
}


// -------- ENTRY POINT ---------------------------------------------------- //

int main(int argc, char* argv[])
{
    int result = RunTestApp(argc, argv);

    system("pause");
    return result;
}
