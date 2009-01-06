///////////////////////////////////////////////////////////////
//
// WindowHelper.h
//
// Created: 20/07/2003
// Copyright (c) 2003 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#ifndef __WINDOWHELPER_H_8360FA15_18D4_4399_8D81_A9F5D6A4F383_
#define __WINDOWHELPER_H_8360FA15_18D4_4399_8D81_A9F5D6A4F383_

#include "tstring.h"

namespace WindowHelper
{
/**
 * Retrieve the class name of the given window
 **/
    tstring GetClassName( HWND hWnd );

    struct VersionInfo
    {
        WORD    minorVersion;
        WORD    majorVersion;
        WORD    fileVersion;
        WORD    buildNumber;
    };

/**
 * Retrive the module version info of the application that
 * owns the passed window
 **/
    bool    GetWindowModuleVersion( HWND hWnd, VersionInfo &versionInfo );
}

#endif // __WINDOWHELPER_H_8360FA15_18D4_4399_8D81_A9F5D6A4F383_
