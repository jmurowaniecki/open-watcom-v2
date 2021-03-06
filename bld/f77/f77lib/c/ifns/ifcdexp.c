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


//
// IFCDEXP      : exponential function for COMPLEX*16 argument
//

#include "fmath.h"

#include "ftnstd.h"
#include "ifenv.h"


#if !defined( __alternate_if__ )

dcomplex        CDEXP( double rp, double ip ) {
//=============================================

// Return the exponential of arg.
// exp( x + iy ) = exp( x ) * cis( y )

    dcomplex    result;

    result.realpart = exp( rp );
    result.imagpart = result.realpart * sin( ip );
    result.realpart *= cos( ip );
    return( result );
}

#endif


dcomplex        XCDEXP( dcomplex *arg ) {
//=======================================

    return( CDEXP( arg->realpart, arg->imagpart ) );
}
