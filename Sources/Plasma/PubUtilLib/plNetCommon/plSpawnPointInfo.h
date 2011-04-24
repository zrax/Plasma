/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef plSpawnPointInfo_h_inc
#define plSpawnPointInfo_h_inc

#include "hsConfig.h"
#include "hsStlUtils.h"

///////////////////////////////////////////////////////////////////

#define kDefaultSpawnPtTitle        "Default"
#define kDefaultSpawnPtName         "LinkInPointDefault"

///////////////////////////////////////////////////////////////////

class hsStream;

struct plSpawnPointInfo
{
    plString fTitle;         // friendly title for GUIs
    plString fSpawnPt;       // name of spawn point in dataset
    plString fCameraStack;
    plSpawnPointInfo(){}
    plSpawnPointInfo( const plSpawnPointInfo & other ) { (*this)=other; }
    plSpawnPointInfo( const char * title, const char * spawnPt )
        : fTitle( title ), fSpawnPt( spawnPt ) {}
    plString GetTitle() const { return fTitle; }
    void    SetTitle( const plString & v ) { fTitle=v; }
    plString GetName() const { return fSpawnPt; }
    void    SetName( const plString & v ) { fSpawnPt = v; }
    plString GetCameraStack() const { return fCameraStack; }
    void    SetCameraStack( const plString & v ) { fCameraStack=v; }
    void    Reset();
    void    Read( hsStream * s );
    void    ReadOld( hsStream * s );
    void    Write( hsStream * s ) const;
    plString AsString() const;
};
typedef std::vector<plSpawnPointInfo>   plSpawnPointVec;

extern const plSpawnPointInfo kDefaultSpawnPoint;



#endif // plSpawnPointInfo_h_inc
