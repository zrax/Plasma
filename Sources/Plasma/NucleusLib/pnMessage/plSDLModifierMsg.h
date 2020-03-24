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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef plSDLModifierMsg_INC
#define plSDLModifierMsg_INC

#include "pnMessage/plMessage.h"


//
// A msg sent to an SDL modifier to tell it send or recv state.
//
class hsStream;
class hsResMgr;
class plStateDataRecord;
class plSDLModifierMsg : public plMessage
{
public:
    enum Action
    {
        kActionNone = 0,
        kRecv,
        kSendToServer,
        kSendToServerAndClients
    };

protected:
    ST::string fSDLName;            // the state descriptor name (ie. "physical")
    Action fAction;
    plStateDataRecord* fState;      // for recving state
    bool fManageStateMem;           // delete fState?
    uint32_t fPlayerID;
    uint32_t fFlags;

public:
    plSDLModifierMsg(const ST::string& sdlName=ST::null, Action a=kActionNone);
    ~plSDLModifierMsg();

    CLASSNAME_REGISTER(plSDLModifierMsg);
    GETINTERFACE_ANY(plSDLModifierMsg, plMessage);

    uint32_t GetFlags() const { return fFlags; }
    void SetFlags(uint32_t f) { fFlags = f; }

    Action GetAction() const { return fAction; }
    void SetAction(Action t) { fAction=t; }

    plStateDataRecord* GetState(bool unManageState=false) { if (unManageState) fManageStateMem=false; return fState; }
    void SetState(plStateDataRecord* s, bool manageState) { fState=s; fManageStateMem=manageState; }

    ST::string GetSDLName() const { return fSDLName; }
    void SetSDLName(const ST::string& s) { fSDLName=s; }

    uint32_t GetPlayerID() const { return fPlayerID;  }
    void SetPlayerID(uint32_t p) { fPlayerID=p;   }

    // IO
    void Read(hsStream* stream, hsResMgr* mgr) HS_OVERRIDE {
        hsAssert(false, "local only msg");
    }
    void Write(hsStream* stream, hsResMgr* mgr) HS_OVERRIDE {
        hsAssert(false, "local only msg");
    }
};

#endif  // plSDLModifierMsg_INC
