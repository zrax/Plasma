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
#include "hsStlUtils.h"

// stl extensions
namespace xtl {

typedef std::vector<std::string> StringVector;
typedef std::vector<std::wstring> WStringVector;
typedef std::list<std::string> StringList;
typedef std::list<std::wstring> WStringList;
typedef std::set<std::string> StringSet;
typedef std::set<std::wstring> WStringSet;

template bool GetStringGroup<StringList>(const std::string& s, StringList& group, char sep);
template bool GetStringGroup<WStringList>(const std::wstring& s, WStringList& group, wchar_t sep);
template bool GetStringGroup<StringVector>(const std::string& s, StringVector& group, char sep);
template bool GetStringGroup<WStringVector>(const std::wstring& s, WStringVector& group, wchar_t sep);
template bool GetStringGroup<StringSet>(const std::string& s, StringSet& group, char sep);
template bool GetStringGroup<WStringSet>(const std::wstring& s, WStringSet& group, wchar_t sep);

template <typename T> bool GetStringGroup(const std::string& s, T& group, char sep)
{
    bool ret = false;
    std::string::size_type oldpos = 0, newpos = 0;
    
    if (!(s.empty()))
    {
        do 
        {
            newpos = s.find(',',oldpos);
            group.insert(group.end(),s.substr(oldpos,newpos));
            if (newpos != s.npos)
                oldpos = newpos+1;
        } 
        while(newpos != s.npos);
        ret = true;
    }

    return ret;
}

template <typename T> bool GetStringGroup(const std::wstring& s, T& group, wchar_t sep)
{
    bool ret = false;
    std::wstring::size_type oldpos = 0, newpos = 0;

    if (!(s.empty()))
    {
        do
        {
            newpos = s.find(L',',oldpos);
            group.insert(group.end(),s.substr(oldpos,newpos));
            if (newpos != s.npos)
                oldpos = newpos+1;
        } while(newpos != s.npos);
        ret = true;
    }

    return ret;
}


template bool GetStringGroupAsString<StringList>(const StringList& group, std::string& s, char sep);
template bool GetStringGroupAsString<WStringList>(const WStringList& group, std::wstring& s, wchar_t sep);
template bool GetStringGroupAsString<StringVector>(const StringVector& group, std::string& s, char sep);
template bool GetStringGroupAsString<WStringVector>(const WStringVector& group, std::wstring& s, wchar_t sep);
template bool GetStringGroupAsString<StringSet>(const StringSet& group, std::string& s, char sep);
template bool GetStringGroupAsString<WStringSet>(const WStringSet& group, std::wstring& s, wchar_t sep);

template <typename T> bool GetStringGroupAsString(const T& group, std::string& s, char sep)
{
    typename T::const_iterator it = group.begin();
    bool fst = true;
    while (it != group.end())
    {
        if (!fst)
            s += ",";
        else
            fst = false;
        s+= (*it).c_str();
        it++;
    }

    return true;
}

template <typename T> bool GetStringGroupAsString(const T& group, std::wstring& s, wchar_t sep)
{
    typename T::const_iterator it = group.begin();
    bool fst = true;
    while (it != group.end())
    {
        if (!fst)
            s += L",";
        else
            fst = false;
        s+= (*it).c_str();
        it++;
    }
    
    return true;
}


} // namespace std

