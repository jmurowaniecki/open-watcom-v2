/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#include "variety.h"
#include "cover.h"
#include <stdlib.h>

struct wndprocs *WndProcList = 0;

LONG PASCAL _Cover_GetWindowLong( HWND wnd, short what )
{
    LONG        value;
    struct wndprocs *list;

    value = GetWindowLong( wnd, what );
    if( what == GWL_WNDPROC ) {
        for( list = WndProcList; ; list = list->next ) {
            if( list == NULL ) {
                list = malloc( sizeof( struct wndprocs ) );
                if( list != NULL ) {
                    list->next = WndProcList;
                    list->hwnd = wnd;
                    list->procaddr = value;
                    list->proctype = PROC_16;
                    WndProcList = list;
                }
                break;
            }
            if( list->hwnd == wnd && list->procaddr == value ) {
                break;
            }
        }
    }
    return( value );
}

LONG PASCAL _Cover_SetWindowLong( HWND wnd, short what, LONG value )
{
    struct wndprocs *list;

    if( what == GWL_WNDPROC ) {
        for( list = WndProcList; ; list = list->next ) {
            if( list == NULL ) {
                list = malloc( sizeof( struct wndprocs ) );
                if( list == NULL )
                    return( 0 );
                list->next = WndProcList;
                list->hwnd = wnd;
                list->procaddr = value;
                list->proctype = PROC_32;
                WndProcList = list;
            }
            if( list->hwnd == wnd && list->procaddr == value ) {
                if( list->proctype == PROC_32 ) {
                    value = (LONG)SetProc( (FARPROC)value, GETPROC_CALLBACK );
                }
                break;
            }
        }
    }
    return( SetWindowLong( wnd, what, value ) );
}
