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
#ifndef hsStlUtils_h_inc
#define hsStlUtils_h_inc

#include "hsUtils.h"
#include <xmemory>
#include <functional>
#include <algorithm>
#include <vector>
#include <map>
#include <list>
#include <set>


/*****************************************************************************
*
*   derived stl classes that use our heap manager
*
***/

        // TEMPLATE CLASS cyallocator
template<class _Ty>
    class cyallocator
        : public std::_Allocator_base<_Ty>
    {   // generic cyallocator for objects of class _Ty
public:
    typedef std::_Allocator_base<_Ty> _Mybase;
    typedef typename _Mybase::value_type value_type;


    typedef value_type _FARQ *pointer;
    typedef value_type _FARQ& reference;
    typedef const value_type _FARQ *const_pointer;
    typedef const value_type _FARQ& const_reference;

    typedef _SIZT size_type;
    typedef _PDFT difference_type;

    template<class _Other>
        struct rebind
        {   // convert an cyallocator<_Ty> to an cyallocator <_Other>
        typedef cyallocator<_Other> other;
        };

    pointer address(reference _Val) const
        {   // return address of mutable _Val
        return (&_Val);
        }

    const_pointer address(const_reference _Val) const
        {   // return address of nonmutable _Val
        return (&_Val);
        }

    cyallocator()
        {   // construct default cyallocator (do nothing)
        }

    cyallocator(const cyallocator<_Ty>&)
        {   // construct by copying (do nothing)
        }

    template<class _Other>
        cyallocator(const cyallocator<_Other>&)
        {   // construct from a related cyallocator (do nothing)
        }

    template<class _Other>
        cyallocator<_Ty>& operator=(const cyallocator<_Other>&)
        {   // assign from a related cyallocator (do nothing)
        return (*this);
        }

    void deallocate(pointer _Ptr, size_type)
        {   // deallocate object at _Ptr, ignore size
        FREE(_Ptr);
        }

    pointer allocate(size_type _Count)
        {   // allocate array of _Count elements
        return (pointer)ALLOC(_Count * sizeof(_Ty));
        }

    pointer allocate(size_type _Count, const void _FARQ *)
        {   // allocate array of _Count elements, ignore hint
        return (allocate(_Count));
        }

    void construct(pointer _Ptr, const _Ty& _Val)
        {   // construct object at _Ptr with value _Val
        std::_Construct(_Ptr, _Val);
        }

    void destroy(pointer _Ptr)
        {   // destroy object at _Ptr
        std::_Destroy(_Ptr);
        }

    _SIZT max_size() const
        {   // estimate maximum array size
        _SIZT _Count = (_SIZT)(-1) / sizeof (_Ty);
        return (0 < _Count ? _Count : 1);
        }
    };

        // cyallocator TEMPLATE OPERATORS
template<class _Ty,
    class _Other> inline
    bool operator==(const cyallocator<_Ty>&, const cyallocator<_Other>&)
    {   // test for cyallocator equality (always true)
    return (true);
    }

template<class _Ty,
    class _Other> inline
    bool operator!=(const cyallocator<_Ty>&, const cyallocator<_Other>&)
    {   // test for cyallocator inequality (always false)
    return (false);
    }

        // CLASS cyallocator<void>
template<> class _CRTIMP2 cyallocator<void>
    {   // generic cyallocator for type void
public:
    typedef void _Ty;
    typedef _Ty _FARQ *pointer;
    typedef const _Ty _FARQ *const_pointer;
    typedef _Ty value_type;

    template<class _Other>
        struct rebind
        {   // convert an cyallocator<void> to an cyallocator <_Other>
        typedef cyallocator<_Other> other;
        };

    cyallocator()
        {   // construct default cyallocator (do nothing)
        }

    cyallocator(const cyallocator<_Ty>&)
        {   // construct by copying (do nothing)
        }

    template<class _Other>
        cyallocator(const cyallocator<_Other>&)
        {   // construct from related cyallocator (do nothing)
        }

    template<class _Other>
        cyallocator<_Ty>& operator=(const cyallocator<_Other>&)
        {   // assign from a related cyallocator (do nothing)
        return (*this);
        }
    };


/*****************************************************************************
*
*   Drop-in replacements for stl classes. Uses our allocator instead of the default one.
*
***/

        // TEMPLATE CLASS cyvector
template<class _Ty>
class cyvector : public std::vector<_Ty, cyallocator<_Ty> > {
};

        // TEMPLATE CLASS cymap
template<class _Kty, class _Ty, class _Pr=std::less<_Kty> >
class cymap : public std::map<_Kty, _Ty, _Pr, cyallocator<std::pair<_Kty, _Ty > > > {
};

        // TEMPLATE CLASS cylist
template<class _Ty>
class cylist : public std::list<_Ty, cyallocator<_Ty> > {
};

        // TEMPLATE CLASS cyset
template<class _Kty, class _Pr = std::less<_Kty> >
class cyset : public std::set<_Kty, _Pr, cyallocator< _Kty > > {
};



/*****************************************************************************
*
*   stl extensions
*
***/
namespace xtl
{

// Why oh why doesn't stl have copy_if?
// See Effective STL [Meyers 2001] Item 36.
template< typename InIt, typename OutIt, typename Pred >
OutIt copy_if( InIt srcBegin, InIt srcEnd, OutIt dstBegin, Pred pred )
{
    while ( srcBegin!=srcEnd )
    {
        if ( pred( *srcBegin ) )
            *dstBegin++ = *srcBegin;
        ++srcBegin;
    }
    return dstBegin;
}



// useful when clearing a vector/list/set of pointers that need to be deleted.
// use like:
//  std::vector<foo*>   vec;
//  std::for_each(vec.begin(),vec.end(),xtl::delete_ptr());
//  vec.clear();

struct delete_ptr
{
    template< class T > void operator()( T * p ) const { delete p;}
};

// useful when clearing a map of pointers that need to be deleted.
// use like:
//  typedef std::map<int,foo*> foomap;
//  foomap  m;
//  std::for_each(m.begin(),m.end(),xtl::delete_map_ptr_T<foomap>());
//  m.clear();

template< class A >
struct delete_map_ptr_T
{
    void operator()( typename A::value_type & pair ) const { delete pair.second;}
};


template <typename T> bool GetStringGroup(const std::string& s, T& group, char sep = ',');
template <typename T> bool GetStringGroup(const std::wstring& s, T& group, wchar_t sep = L',');
template <typename T> bool GetStringGroupAsString(const T& group, std::string& s, char sep = ',');
template <typename T> bool GetStringGroupAsString(const T& group, std::wstring& s, wchar_t sep = L',');


} // namespace xtd



#endif // hsStlUtils_h_inc
