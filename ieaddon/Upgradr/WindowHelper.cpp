///////////////////////////////////////////////////////////////
//
// WindowHelper.cpp
//
// Created: 20/07/2003
// Copyright (c) 2003 Ralph Hare (ralph.hare@ysgyfarnog.co.uk)
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WindowHelper.h"
#include <vector>

#pragma comment( lib, "version" )

namespace
{
    const size_t    CLASS_NAME_SIZE = 128;
}

namespace WindowHelper
{
    tstring GetClassName( HWND hWnd )
    {
        TCHAR   className[ CLASS_NAME_SIZE ];
        ::GetClassName( hWnd, className, CLASS_NAME_SIZE );

        return className;
    }

    bool GetWindowModuleVersion( HWND hWnd, VersionInfo &versionInfo )
    {
        TCHAR   fileName[ _MAX_PATH ];
        if( ::GetWindowModuleFileName( hWnd, fileName, _MAX_PATH ) == 0 )
        {
            return false;
        }

        DWORD   dummy;
        DWORD   cbInfo = ::GetFileVersionInfoSize( fileName, &dummy );
        if( cbInfo == 0 )
        {
            return false;
        }

        std::vector< BYTE >     buffer( cbInfo );
        if( ::GetFileVersionInfo( fileName, 0, cbInfo, &buffer[ 0 ] ) == FALSE )
        {
            return false;
        }

        void    *pValue;
        size_t  cbValue;
        if( ::VerQueryValue( 
                        &buffer[ 0 ],
                        _T( "\\" ), 
                        &pValue, 
                        &cbValue 
                        ) == FALSE )
        {
            return false;
        }

        VS_FIXEDFILEINFO    *pInfo = reinterpret_cast< VS_FIXEDFILEINFO * >( pValue );

        memcpy( &versionInfo, &( pInfo->dwFileVersionMS ), sizeof( VersionInfo ) );

        return true;
    }

} // WindowHelper