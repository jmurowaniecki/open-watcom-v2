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
* Description:  Extend segment size
*               (16-bit code only)
*
****************************************************************************/


#include "dll.h"        // needs to be first
#include "variety.h"
#include <stddef.h>
#include <stdlib.h>
#include <malloc.h>
#if defined(__OS2__)
    #include <dos.h>
    #define INCL_DOSMEMMGR
    #include <wos2.h>
#elif defined(__QNX__)
    #include <sys/types.h>
    #include <sys/seginfo.h>
#elif defined(__WINDOWS__)
    #include <dos.h>
    #include <windows.h>
#else
    #include <dos.h>
#endif
#include "rtdata.h"
#if defined(__OS2__)
#elif defined(__QNX__)
#elif defined(__WINDOWS__)
#else
    #include "tinyio.h"
#endif
#include "heap.h"


#define HEAP(s)             ((XBPTR(heapblkp, s))0)
#define FRLPTR(s)           XBPTR(freelistp, s)
#define SET_HEAP_END(s,p)   ((FRLPTR(s))(p))->len = END_TAG; ((FRLPTR(s))(p))->prev = 0

int __GrowSeg( __segment seg, unsigned int amount )
{
    unsigned        num_of_paras;   /* number of paragraphs desired   */
    unsigned        new_heaplen;
    unsigned int    old_heaplen;
    unsigned int    old_heap_paras;
    FRLPTR( seg )   pfree;
    FRLPTR( seg )   pnew;

    if( !__heap_enabled )
        return( 0 );
    old_heaplen = HEAP( seg )->heaplen;
    if( old_heaplen != 0 ) {                /* if not already 64K */
        amount += TAG_SIZE;
        if( amount < TAG_SIZE )
            amount = /*0x....ffff*/ ~0U;
        if( amount < _amblksiz )
            amount = _amblksiz;
        num_of_paras = __ROUND_UP_SIZE_TO_PARA( amount );
        if( num_of_paras == 0 )
            num_of_paras = PARAS_IN_64K;
        old_heap_paras = __ROUND_DOWN_SIZE_TO_PARA( old_heaplen );
        num_of_paras += old_heap_paras;
        /*
            We shouldn't extend segments to 64k if we are not going to
            use the space for this allocation.  In protected-mode
            environments, it should be possible to extend segments
            later on when we know we can use the space.
        */
#if defined(__QNX__)
        if( num_of_paras > PARAS_IN_64K )
            return( 0 );
#elif defined(__OS2__) || defined(__WINDOWS__)
        if( num_of_paras > PARAS_IN_64K ) {
            if( _RWD_osmode != DOS_MODE ) {
                /* protected-mode */
                return( 0 );
            } else {
                num_of_paras = PARAS_IN_64K;
            }
        }
#else
        if( num_of_paras > PARAS_IN_64K )
            num_of_paras = PARAS_IN_64K;
#endif
#if defined(__OS2__)
        if( DosReallocSeg( num_of_paras << 4, seg ) != 0 )
            return( 0 );
#elif defined(__QNX__)
        if( qnx_segment_realloc( seg, ((long)num_of_paras) << 4 ) == -1 )
            return( 0 );
#elif defined(__WINDOWS__)
        if( old_heap_paras < ( PARAS_IN_64K - 2 ) && num_of_paras == PARAS_IN_64K ) {
            num_of_paras = PARAS_IN_64K - 2;
        } else if( num_of_paras > ( PARAS_IN_64K - 2 ) ) {
            /*
              in Standard mode, GlobalRealloc may change selectors
              if size > 65519 (64k-17)! (p. 4-246 Windows Ref) AFS 23-apr-91
            */
            return( 0 );
        }
        {
            HANDLE hmem;

            hmem = (HANDLE)GlobalHandle( seg );
            if( hmem == NULL ) {
                return( 0 );
            }
            if( GlobalReAlloc( hmem, ((long)num_of_paras) << 4, __win_realloc_flags) == NULL ){
                return( 0 );
            }
        }
#else
        if( TINY_ERROR( TinySetBlock( num_of_paras, seg ) ) )
            return( 0 );
#endif
        new_heaplen = num_of_paras << 4;
        HEAP( seg )->heaplen = new_heaplen;        /* put in new heap length */
        pfree = HEAP( seg )->freehead.prev;
        if( NEXT_BLK( pfree ) != old_heaplen - TAG_SIZE * 2 ) {
            /* last free entry not at end of the heap */
            /* add a new free entry to end of list */
            pnew = (FRLPTR( seg ))( old_heaplen - TAG_SIZE * 2 );
            pnew->prev = pfree;
            pnew->next = pfree->next;
            pfree->next = pnew;
            HEAP( seg )->freehead.prev = pnew;
            HEAP( seg )->numfree++;
            pfree = pnew;
        }
        pfree->len = new_heaplen - FP_OFF( pfree ) - TAG_SIZE * 2;
        if( HEAP( seg )->largest_blk < pfree->len )
            HEAP( seg )->largest_blk = pfree->len;
        SET_HEAP_END( seg, new_heaplen - 2 * TAG_SIZE );
        return( 1 );                /* indicate segment was grown */
    }
    return( 0 );    /* indicate failed to grow the segment */
}
