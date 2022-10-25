/*
 * 16 bit palette handling routines, by Johan Klockars.
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

#define NOVA 0		/* 1 - byte swap 16 bit colour value (NOVA etc) */

#define red_bits   3	/* 5 for all normal 16 bit hardware */
#define green_bits 3	/* 6 for Falcon TC and NOVA 16 bit, 5 for NOVA 15 bit */
/* (I think 15 bit Falcon TC disregards the green LSB) */
#define blue_bits  2	/* 5 for all normal 16 bit hardware */


long CDECL c_get_colour(Virtual *vwk, long colour)
{
    Colour *local_palette, *global_palette;
    Colour *fore_pal, *back_pal;
    unsigned short foreground, background;
    PIXEL *realp;
    unsigned long bgcomponent;
    unsigned long fgcomponent;

    local_palette = vwk->palette;
    if (local_palette && !((long)local_palette & 1)) {   /* Complete local palette? */
        fore_pal = back_pal = local_palette;
//        PRINTF(("local_palette\r\n"));
    }
    else {                      /* Global or only negative local */
        local_palette = (Colour *)((long)local_palette & 0xfffffffeL);
        global_palette = vwk->real_address->screen.palette.colours;
        if (local_palette && ((short)colour < 0))
            fore_pal = local_palette;
        else
            fore_pal = global_palette;
        if (local_palette && ((colour >> 16) < 0))
            back_pal = local_palette;
        else
            back_pal = global_palette;
//        PRINTF(("colours: %p: fore_pal/back_pal: %p/%p\r\n", global_palette, fore_pal, back_pal));
    }

    // palette format is concatenation of ( RGB vdi, RGB hw, long real )
    // where RGB is short r, short, g, short b
    // the RGB goes 0-1000 ( 0x3e8 )
    realp = (PIXEL*)&fore_pal[(short)colour].real;
    foreground = *realp;
//    PRINTF(( "pal: %p, pal[col]: %p, pal[col].real: %d\r\n", fore_pal, (unsigned short *)&fore_pal[(short)colour], foreground )) ;
    realp = (PIXEL*)&back_pal[colour >> 16].real;
    background = *realp;
    
    bgcomponent = ((unsigned long)background << 16);
    fgcomponent = (unsigned long)foreground;
   
//    PRINTF(("c_get_colour(%lx): %lx (%lx/%lx)\r\n", colour, bgcomponent | fgcomponent, bgcomponent, fgcomponent));
//    PRINTF(( "[%d] c_get_col(%d) %x/%x\r\n", vwk->standard_handle, (int)colour, foreground, background ));

    return bgcomponent | fgcomponent;
}


void CDECL c_get_colours(Virtual *vwk, long colour, unsigned long *foreground, unsigned long *background)
{
    Colour *local_palette, *global_palette;
    Colour *fore_pal, *back_pal;
    PIXEL *realp;

    local_palette = vwk->palette;
    if (local_palette && !((long)local_palette & 1))    /* Complete local palette? */
        fore_pal = back_pal = local_palette;
    else {                      /* Global or only negative local */
        local_palette = (Colour *)((long)local_palette & 0xfffffffeL);
        global_palette = vwk->real_address->screen.palette.colours;
        if (local_palette && ((short)colour < 0))
            fore_pal = local_palette;
        else
            fore_pal = global_palette;
        if (local_palette && ((colour >> 16) < 0))
            back_pal = local_palette;
        else
            back_pal = global_palette;
    }

    realp = (PIXEL*)&fore_pal[(short)colour].real;
    *foreground = *realp;
    realp = (PIXEL*)&back_pal[colour >> 16].real;
    *background = *realp;
}


void CDECL c_set_colours(Virtual *vwk, long start, long entries, unsigned short *requested, Colour palette[])
{
    unsigned short colour;
    unsigned short component;
    unsigned long tc_word;
    int i;
    //short *realp;
    PIXEL *realp;

//    PRINTF(( "c_set_colours( %d, %ld, %ld, %p, %p)\r\n", vwk->standard_handle, start, entries, requested, palette ));
    
    (void) vwk;
    if ((long)requested & 1) {          /* New entries? */
//        PRINTF(( "c_set_colours(): new entries\r\n"));
        requested = (unsigned short *)((long)requested & 0xfffffffeL);
        for(i = 0; i < entries; i++) {
            requested++;                /* First word is reserved */
            component = *requested++ >> 8;
            palette[start + i].vdi.red = (component * 1000L) / 255;
            palette[start + i].hw.red = component;  /* Not at all correct */
            colour = component >> (16 - red_bits);  /* (component + (1 << (14 - red_bits))) */
            tc_word = colour << green_bits;
            component = *requested++ >> 8;
            palette[start + i].vdi.green = (component * 1000L) / 255;
            palette[start + i].hw.green = component;    /* Not at all correct */
            colour = component >> (16 - green_bits);    /* (component + (1 << (14 - green_bits))) */
            tc_word |= colour;
            tc_word <<= blue_bits;
            component = *requested++ >> 8;
            palette[start + i].vdi.blue = (component * 1000L) / 255;
            palette[start + i].hw.blue = component; /* Not at all correct */
            colour = component >> (16 - blue_bits);     /* (component + (1 << (14 - blue_bits))) */
            tc_word |= colour;
#if NOVA
            tc_word = ((tc_word & 0x000000ff) << 24) | ((tc_word & 0x0000ff00) <<  8) |
                      ((tc_word & 0x00ff0000) >>  8) | ((tc_word & 0xff000000) >> 24);
#endif
            realp = (PIXEL*)&palette[start + i].real;
            *realp = tc_word;
        }
    } else {
//        PRINTF(( "c_set_colours(): not deemed new entries\r\n"));
        for(i = 0; i < entries; i++) {
            component = *requested++;
            palette[start + i].vdi.red = component;
            palette[start + i].hw.red = component;  /* Not at all correct */
            colour = (component * ((1L << red_bits) - 1) + 500L) / 1000;
            tc_word = colour << green_bits;
            component = *requested++;
            palette[start + i].vdi.green = component;
            palette[start + i].hw.green = component;    /* Not at all correct */
            colour = (component * ((1L << green_bits) - 1) + 500L) / 1000;
            tc_word |= colour;          /* Was (colour + colour) */
            tc_word <<= blue_bits;
            component = *requested++;
            palette[start + i].vdi.blue = component;
            palette[start + i].hw.blue = component; /* Not at all correct */
            colour = (component * ((1L << blue_bits) - 1) + 500L) / 1000;
            tc_word |= colour;
#if NOVA
            tc_word = (tc_word << 8) | (tc_word >> 8);
#endif
            realp = (PIXEL*)&palette[start + i].real;
            *realp = tc_word;

//            if( i < 2 ) {
//                PRINTF(( "%d: %d %d %d = %lx %x (%p)\r\n", i, palette[start + i].vdi.red, palette[start + i].vdi.green, palette[start + i].vdi.blue, tc_word, (*realp), realp ));
//            }
        }
    }
}
