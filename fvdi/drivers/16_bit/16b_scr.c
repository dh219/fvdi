/* 
 * 16 bit pixel set/get routines, by Johan Klockars.
 *
 * This file is an example of how to write an
 * fVDI device driver routine in C.
 *
 * You are encouraged to use this file as a starting point
 * for other accelerated features, or even for supporting
 * other graphics modes. This file is therefore put in the
 * public domain. It's not copyrighted or under any sort
 * of license.
 */

#include "fvdi.h"
#include "driver.h"
#include "../bitplane/bitplane.h"
#include "relocate.h"

#define PIXEL       unsigned char
#define PIXEL_SIZE  sizeof(PIXEL)

int rand(void)
{
    static unsigned long _next;
    _next = (_next * 1103515245) + 12345;
    return ((_next >> 16) & 0x7fff);
}

/* destination MFDB (odd address marks table operation)
 * x or table address
 * y or table length (high) and type (0 - coordinates)
 */
long CDECL
c_write_pixel(Virtual *vwk, MFDB *dst, long x, long y, long colour)
{
    Workstation *wk;
    long offset;

    if ((long)vwk & 1)
        return 0;
    
    wk = vwk->real_address;



    // still don't quite get what the difference between these branches is
    if (!dst || !dst->address || (dst->address == wk->screen.mfdb.address)) {
        offset = wk->screen.wrap * y + x * PIXEL_SIZE;
#ifdef BOTH
        if (wk->screen.shadow.address) {
            *(PIXEL *)((long)wk->screen.shadow.address + offset) = colour;
        }
#endif
        *(PIXEL *)((long)wk->screen.mfdb.address + offset) = colour;
    } else {
        //PRINTF(("[%d] B\n", vwk->standard_handle ));
        offset = (dst->wdwidth * 2 * dst->bitplanes) * y + x * PIXEL_SIZE;
        *(PIXEL *)((long)dst->address + offset) = colour;
    }

/*    
    if( ( rand() % 10000 ) == 0 )
        PRINTF(("%c[%d] %ld,%ld = %lx (%p)\n", a ? 'A' : 'B', vwk->standard_handle, x, y, colour, vwk->palette ));
  */  
    return 1;
}


long CDECL
c_read_pixel(Virtual *vwk, MFDB *src, long x, long y)
{
    Workstation *wk;
    long offset;
    PIXEL colour;

    wk = vwk->real_address;
    if (!src || !src->address || (src->address == wk->screen.mfdb.address)) {
        offset = wk->screen.wrap * y + x * PIXEL_SIZE;
#ifdef BOTH
        if (wk->screen.shadow.address)
        {
            colour = *(PIXEL *)((long)wk->screen.shadow.address + offset);
        } else
#endif
        {
            colour = *(PIXEL *)((long)wk->screen.mfdb.address + offset);
        }
    } else {
        offset = (src->wdwidth * 2 * src->bitplanes) * y + x * PIXEL_SIZE;
        colour = *(PIXEL *)((long)src->address + offset);
    }
//        PRINTF(("c_read_pixel( %d, %p, %ld, %ld ) == %x\n", vwk->standard_handle, src, x, y, colour ));
    return colour;
}
