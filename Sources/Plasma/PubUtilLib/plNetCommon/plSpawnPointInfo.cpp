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
#include "plSpawnPointInfo.h"
#include "pnMessage/plMessage.h"
#include "hsStream.h"
#include "hsBitVector.h"


const plSpawnPointInfo kDefaultSpawnPoint( plString::FromUtf8(kDefaultSpawnPtTitle),
                                           plString::FromUtf8(kDefaultSpawnPtName) );


namespace SpawnPointInfoStreamFlags
{
    enum
    {
        kHasTitle,
        kHasName,
        kHasCameraStack,
    };
}

void plSpawnPointInfo::ReadOld( hsStream * s )
{
    s->LogSubStreamStart("push me");
    s->LogSubStreamPushDesc("Title");
    plMsgPlStringHelper::Peek( fTitle, s );
    s->LogSubStreamPushDesc("Name");
    plMsgPlStringHelper::Peek( fSpawnPt, s );
    fCameraStack = "";
    s->LogSubStreamEnd();
}

void plSpawnPointInfo::Read( hsStream * s )
{
    hsBitVector flags;
    flags.Read( s );

    s->LogSubStreamStart("push me");
    if ( flags.IsBitSet( SpawnPointInfoStreamFlags::kHasTitle ) )
    {
        s->LogSubStreamPushDesc("Title");
        plMsgPlStringHelper::Peek( fTitle, s );
    }
    if ( flags.IsBitSet( SpawnPointInfoStreamFlags::kHasName ) )
    {
        s->LogSubStreamPushDesc("Name");
        plMsgPlStringHelper::Peek( fSpawnPt, s );
    }
    if ( flags.IsBitSet( SpawnPointInfoStreamFlags::kHasCameraStack ) )
    {
        s->LogSubStreamPushDesc("CameraStack");
        plMsgPlStringHelper::Peek( fCameraStack, s );
    }
    s->LogSubStreamEnd();
}

void plSpawnPointInfo::Write( hsStream * s ) const
{
    hsBitVector flags;
    flags.SetBit( SpawnPointInfoStreamFlags::kHasTitle );
    flags.SetBit( SpawnPointInfoStreamFlags::kHasName );
    flags.SetBit( SpawnPointInfoStreamFlags::kHasCameraStack );
    flags.Write( s );

    if ( flags.IsBitSet( SpawnPointInfoStreamFlags::kHasTitle ) )
    {
        plMsgPlStringHelper::Poke( fTitle, s );
    }
    if ( flags.IsBitSet( SpawnPointInfoStreamFlags::kHasName ) )
    {
        plMsgPlStringHelper::Poke( fSpawnPt, s );
    }
    if ( flags.IsBitSet( SpawnPointInfoStreamFlags::kHasCameraStack ) )
    {
        plMsgPlStringHelper::Poke( fCameraStack, s );
    }
}

void plSpawnPointInfo::Reset()
{
    (*this)=kDefaultSpawnPoint;
}

plString plSpawnPointInfo::AsString() const
{
    return plString::Format( "t:%s,n:%s,c:%s",
        fTitle.IsEmpty()?"(nil)":fTitle.c_str(),
        fSpawnPt.IsEmpty()?"(nil)":fSpawnPt.c_str(),
        fCameraStack.IsEmpty()?"(nil)":fCameraStack.c_str() );
}
