/* --------------------------------------------------------------------------------------------- */
/* Vector Doubles                                                                                */
/*                                                                                               */
/* Atari 8-bit XL/XE matching game, built using the cc65-2.19 toolchain.                         */
/*                                                                                               */
/* BSD-3-Clause License                                                                          */
/* Copyright 2024 Supurloop Software LLC                                                         */
/*                                                                                               */
/* Redistribution and use in source and binary forms, with or without modification, are          */
/* permitted provided that the following conditions are met:                                     */
/*                                                                                               */
/* 1. Redistributions of source code must retain the above copyright notice, this list of        */
/* conditions and the following disclaimer.                                                      */
/* 2. Redistributions in binary form must reproduce the above copyright notice, this list of     */
/* conditions and the following disclaimer in the documentation and/or other materials provided  */
/* with the distribution.                                                                        */
/* 3. Neither the name of the copyright holder nor the names of its contributors may be used to  */
/* endorse or promote products derived from this software without specific prior written         */
/* permission.                                                                                   */
/*                                                                                               */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   */
/* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    */
/* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL      */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE */
/* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    */
/* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  */
/* OF THE POSSIBILITY OF SUCH DAMAGE.                                                            */
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/* Buils                                                                                         */
/* cl65 --target atari -C .\atari-xex.cfg -O -o Vector-Doubles.xex main.c                        */
/* --------------------------------------------------------------------------------------------- */
// To Build:

/* --------------------------------------------------------------------------------------------- */
/* Includes                                                                                      */
/* --------------------------------------------------------------------------------------------- */
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atari.h>
#include <peekpoke.h>

/* --------------------------------------------------------------------------------------------- */
/* Macros                                                                                        */
/* --------------------------------------------------------------------------------------------- */
#define FILTER_MACRO(column) \
    tmp = rows[line][column]; \
    if (tmp == HEART) \
    { \
      if ((hposp0row == column) && (hposp1row == column)) \
      { \
          note = 16; \
          POKEY_WRITE.audf1 = note; \
          volume = 0xAC; \
          POKEY_WRITE.audc1 = volume; \
          total += 10; \
      } \
      else if ((hposp0row == column) || (hposp1row == column)) \
      { \
          note = 16; \
          POKEY_WRITE.audf1 = note; \
          volume = 0xAA; \
          POKEY_WRITE.audc1 = volume; \
          total += 5; \
      } \
      else \
      { \
        note = 204; \
        POKEY_WRITE.audf1 = note; \
        volume = 0x8C; \
        POKEY_WRITE.audc1 = volume; \
        if (total >= 5) total -= 5; \
      } \
      rows[line][column] = 0; \
    } \
    else if (filter[column] == tmp) \
    { \
          rows[line][column] = 0; \
          filter[column] = FILTER_IDLE; \
          note = 16; \
          POKEY_WRITE.audf1 = note; \
          volume = 0xA8; \
          POKEY_WRITE.audc1 = volume; \
          total++; \
    } \
    else if (tmp != 0) \
    { \
        rows[line][column] = 0; \
        filter[column] = FILTER_IDLE; \
        note = 204; \
        POKEY_WRITE.audf1 = note; \
        volume = 0x88; \
        POKEY_WRITE.audc1 = volume; \
        if (total > 0) total--; \
    }

#define MOVE_PLAYER(strig, stick, hposp, center) \
    if (PEEK(strig) == 1) \
    { \
      tmp = PEEK(stick); \
      if (tmp == 15) { center = 1; } \
      else if (tmp == 11) \
      { \
          if (center) \
          { \
            center = 0; \
            if (hposp == 48) { hposp = 200; hposp##row = 19; } \
            else { hposp -= 8; hposp##row--; } \
          } \
      } \
      else if (tmp == 7) \
      { \
          if (center) \
          { \
            center = 0; \
            if (hposp == 200) { hposp = 48; hposp##row = 0; } \
            else { hposp += 8; hposp##row++; } \
          } \
      } \
    } \
    else \
    { \
      tmp = (hposp >> 3) - 6; \
      switch(PEEK(stick)) \
      { \
        default: \
            break; \
        case 14: \
          filter[tmp] = ARROW_UP; \
          break; \
        case 11: \
          filter[tmp] = ARROW_LEFT; \
          break; \
        case 13: \
          filter[tmp] = ARROW_DOWN; \
          break; \
        case 7: \
          filter[tmp] = ARROW_RIGHT; \
          break; \
      } \
    }

#define STRIG0 0x0284  //Trigger button
#define STRIG1 0x0285  //Trigger button
#define STICK0 0x0278  //Joystick address
#define STICK1 0x0279  //Joystick address
#define NUM_COLUMNS 20
#define NUM_STATUS_COLUMNS 40
#define NUM_ROWS 12
#define ROW_ADDR_OFF 3

typedef enum
{
    BLANK,
    ARROW_UP,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    HEART,
    FILTER_IDLE,
    CHARS_MAX
} CHAR_t;

/* --------------------------------------------------------------------------------------------- */
/* Module Variables                                                                              */
/* --------------------------------------------------------------------------------------------- */
const char playerChars[CHARS_MAX][8] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x00, },
    { 0x00, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00 },
    { 0x00, 0x10, 0x30, 0x7E, 0x7E, 0x30, 0x10, 0x00 },
    { 0x00, 0x08, 0x0C, 0x7E, 0x7E, 0x0C, 0x08, 0x00 },
    { 0x1c, 0x36, 0x7f, 0x07, 0x7f, 0x3e, 0x1c, 0x00 },
    { 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00 },
};

/* 1KB Aligned for Antic */
#pragma bss-name ("ROW_SEGMENT")
char rows[NUM_ROWS][NUM_COLUMNS];
char filter[NUM_COLUMNS];
char status1[NUM_STATUS_COLUMNS] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00vector\000doubles\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
char status2[NUM_STATUS_COLUMNS];
#pragma bss-name ("BSS")

unsigned char colorbg = 0x97;
unsigned char *cr = (unsigned char *)0xD01A;
unsigned char loop;
unsigned char  x;
unsigned char line = 11;
unsigned char center0;
unsigned char center1;
unsigned char countdown;
unsigned char update;
unsigned char vblanks;
unsigned char isGameStarted;
unsigned char mainbgcolor = 15;
unsigned char filterbgcolor = 10;
unsigned char statusbgcolor; // same as main char color
unsigned char p0color = 24;
unsigned char p1color = 88;
unsigned highlight;
unsigned char reseed;
unsigned char currClockFrame = 0;
unsigned char hposp0 = 88;
unsigned char hposp1 = 160;
unsigned char hposp0row = 5;
unsigned char hposp1row = 14;
unsigned char tmp;
unsigned char once = 1;
unsigned int total;
unsigned int best;
unsigned char note;
unsigned char volume;
unsigned char note2;
unsigned char volume2;
unsigned char starting;
char totalStr[6];

const void dlist =
{
    DL_DLI(DL_VSCROL(DL_BLK8)),
    DL_VSCROL(DL_BLK8),
    DL_VSCROL(DL_LMS(DL_CHR20x16x2)),
    &rows[0],
    DL_VSCROL(DL_LMS(DL_CHR20x16x2)),
    &rows[1],
    DL_VSCROL(DL_LMS(DL_CHR20x16x2)),
    &rows[2],
    DL_VSCROL(DL_LMS(DL_CHR20x16x2)),
    &rows[3],
    DL_VSCROL(DL_LMS(DL_CHR20x16x2)),
    &rows[4],
    DL_VSCROL(DL_LMS(DL_CHR20x16x2)),
    &rows[5],
    DL_VSCROL(DL_LMS(DL_CHR20x16x2)),
    &rows[6],
    DL_VSCROL(DL_LMS(DL_CHR20x16x2)),
    &rows[7],
    DL_VSCROL(DL_LMS(DL_CHR20x16x2)),
    &rows[8],
    DL_VSCROL(DL_LMS(DL_CHR20x16x2)),
    &rows[9],
    DL_VSCROL(DL_LMS(DL_CHR20x16x2)),
    &rows[10],
    DL_DLI(DL_LMS(DL_CHR20x16x2)),
    &rows[11],
    DL_DLI(DL_LMS(DL_CHR20x16x2)),
    filter,
    DL_DLI(DL_LMS(DL_CHR40x8x1)),
    status1,
    (DL_LMS(DL_CHR40x8x1)),
    status2,
    DL_JVB,
    0x0600,
};

/* --------------------------------------------------------------------------------------------- */
/* Local Prototypes                                                                              */
/* --------------------------------------------------------------------------------------------- */
void dli_routine(void);

/* --------------------------------------------------------------------------------------------- */
/* DLI                                                                                           */
/* --------------------------------------------------------------------------------------------- */
void dli_routine2(void)
{
    asm("pha");
    asm("txa");
    asm("pha");

    ANTIC.chbase = 0xE0;
    ANTIC.wsync = 0;
    GTIA_WRITE.grafp0 = 0;
    GTIA_WRITE.grafp1 = 0;

    asm("pla");
    asm("tax");
    asm("pla");
    asm("rti");
}

/* --------------------------------------------------------------------------------------------- */
/* DLI                                                                                           */
/* --------------------------------------------------------------------------------------------- */
void dli_routinea(void)
{
    asm("pha");
    asm("txa");
    asm("pha");

    ANTIC.wsync = 0;
    *cr = filterbgcolor;
    OS.vdslst = &dli_routine2;
    ANTIC.wsync = 0;
    ANTIC.wsync = 0;
    ANTIC.wsync = 0;
    ANTIC.wsync = 0;
    ANTIC.wsync = 0;
    ANTIC.wsync = 0;
    ANTIC.wsync = 0;
    ANTIC.wsync = 0;
    ANTIC.wsync = 0;
    ANTIC.wsync = 0;
    ANTIC.wsync = 0;
    ANTIC.wsync = 0;
    ANTIC.wsync = 0;
    ANTIC.wsync = 0;

    GTIA_WRITE.grafp0 = 0x99;
    GTIA_WRITE.grafp1 = 0xA5;

    asm("pla");
    asm("tax");
    asm("pla");
    asm("rti");
}

/* --------------------------------------------------------------------------------------------- */
/* DLI                                                                                           */
/* --------------------------------------------------------------------------------------------- */
void dli_routine(void)
{
    asm("pha");
    asm("txa");
    asm("pha");

    OS.vdslst = &dli_routinea;
    ANTIC.wsync = 0;

    asm("pla");
    asm("tax");
    asm("pla");
    asm("rti");
}

/* --------------------------------------------------------------------------------------------- */
/* Waits for vblank.                                                                             */
/* --------------------------------------------------------------------------------------------- */
void waitForVBLANK(void)
{
    currClockFrame = OS.rtclok[2];
    while (OS.rtclok[2] == currClockFrame);
}

/* --------------------------------------------------------------------------------------------- */
/* Updates best score                                                                            */
/* --------------------------------------------------------------------------------------------- */
void UpdateBest(void)
{
    sprintf(totalStr, "%u", best);

    if (best < 10)
    {
        status1[35] = 0;
        status1[36] = 0;
        status1[37] = 0;
        status1[38] = 0;
        status1[39] = totalStr[0] - 0x20;
    } else if (best < 100)
    {
        status1[35] = 0;
        status1[36] = 0;
        status1[37] = 0;
        status1[38] = totalStr[0] - 0x20;
        status1[39] = totalStr[1] - 0x20;
    } else if (best < 1000)
    {
        status1[35] = 0;
        status1[36] = 0;
        status1[37] = totalStr[0] - 0x20;
        status1[38] = totalStr[1] - 0x20;
        status1[39] = totalStr[2] - 0x20;
    } else if (best < 10000)
    {
        status1[35] = 0;
        status1[36] = totalStr[0] - 0x20;
        status1[37] = totalStr[1] - 0x20;
        status1[38] = totalStr[2] - 0x20;
        status1[39] = totalStr[3] - 0x20;
    } else
    {
        status1[35] = totalStr[0] - 0x20;
        status1[36] = totalStr[1] - 0x20;
        status1[37] = totalStr[2] - 0x20;
        status1[38] = totalStr[3] - 0x20;
        status1[39] = totalStr[4] - 0x20;
    }
}

/* --------------------------------------------------------------------------------------------- */
/* Main loop                                                                                     */
/* --------------------------------------------------------------------------------------------- */
void main(void)
{
    char *p = (char *)0x8000;
    char *dl = (char *)0x600;

    memset(filter, FILTER_IDLE, sizeof(filter));

    GTIA_WRITE.hposp0 = hposp0;
    GTIA_WRITE.grafp0 = 0x81;
    GTIA_WRITE.sizep0 = 0;
    GTIA_WRITE.colpm0 = 46;
    GTIA_WRITE.colpf0 = 88;
    OS.pcolr0 = p0color;

    GTIA_WRITE.hposp1 = hposp1;
    GTIA_WRITE.grafp1 = 0x81;
    GTIA_WRITE.sizep1 = 0;
    GTIA_WRITE.colpm1 = 46;
    //GTIA_WRITE.colpf1 = 103;
    OS.pcolr1 = p1color;

    GTIA_WRITE.prior = PRIOR_PF03_P03;

    // Copy the display list to RAM
    memcpy((void *)0x0600, &dlist, sizeof(dlist));

    OS.color0 = 0;
    OS.color1 = 15; // Status test color
    OS.color2 = 0;
    OS.color3 = 0;
    OS.color4 = mainbgcolor;

    // ram char set at 0x8000
    OS.chbas = 0x80;
    memcpy(p, playerChars, sizeof(playerChars));
    UpdateBest();

    waitForVBLANK();

    /* Enable DLI */
    OS.vdslst = &dli_routine;
    ANTIC.nmien = 0xC0;
    OS.sdlst = (void *)0x600;

    POKEY_WRITE.audctl = 0;

    while (1)
    {
        waitForVBLANK();
        vblanks++;
        OS.vdslst = &dli_routine;

        GTIA_WRITE.hposp0 = hposp0;
        GTIA_WRITE.hposp1 = hposp1;
        GTIA_WRITE.grafp0 = 0x81;
        GTIA_WRITE.grafp1 = 0x81;

        /* Sound effects */
        if (volume & 0x0f)
        {
            volume--;
            POKEY_WRITE.audc1 = volume;
        }

        if (volume2 & 0x0f)
        {
            volume2--;
            POKEY_WRITE.audc2 = volume2;
        }

        x--;
        x &= 0x0f;
        ANTIC.vscrol = x;
        if (x == 0x0f)
        {
            line--;
            if (line == 255) line = NUM_ROWS - 1;

            /* Update display list */
            tmp = dl[ROW_ADDR_OFF + 33];
            dl[ROW_ADDR_OFF + 33] = dl[ROW_ADDR_OFF + 30];
            dl[ROW_ADDR_OFF + 30] = dl[ROW_ADDR_OFF + 27];
            dl[ROW_ADDR_OFF + 27] = dl[ROW_ADDR_OFF + 24];
            dl[ROW_ADDR_OFF + 24] = dl[ROW_ADDR_OFF + 21];
            dl[ROW_ADDR_OFF + 21] = dl[ROW_ADDR_OFF + 18];
            dl[ROW_ADDR_OFF + 18] = dl[ROW_ADDR_OFF + 15];
            dl[ROW_ADDR_OFF + 15] = dl[ROW_ADDR_OFF + 12];
            dl[ROW_ADDR_OFF + 12] = dl[ROW_ADDR_OFF + 9];
            dl[ROW_ADDR_OFF + 9] = dl[ROW_ADDR_OFF + 6];
            dl[ROW_ADDR_OFF + 6] = dl[ROW_ADDR_OFF + 3];
            dl[ROW_ADDR_OFF + 3] = dl[ROW_ADDR_OFF + 0];
            dl[ROW_ADDR_OFF + 0] = tmp;

            if (isGameStarted)
            {
                /* Check for collisions */
                FILTER_MACRO(0);
                FILTER_MACRO(1);
                FILTER_MACRO(2);
                FILTER_MACRO(3);
                FILTER_MACRO(4);
                FILTER_MACRO(5);
                FILTER_MACRO(6);
                FILTER_MACRO(7);
                FILTER_MACRO(8);
                FILTER_MACRO(9);
                FILTER_MACRO(10);
                FILTER_MACRO(11);
                FILTER_MACRO(12);
                FILTER_MACRO(13);
                FILTER_MACRO(14);
                FILTER_MACRO(15);
                FILTER_MACRO(16);
                FILTER_MACRO(17);
                FILTER_MACRO(18);
                FILTER_MACRO(19);
            } else
            {
                memset(&rows[line], 0, NUM_COLUMNS);
            }
        } else if (x == 0)
        {
            /* Reseed arrows every 4 lines */
            if (reseed == 0)
            {
                rows[line][rand() % 20] = ARROW_UP + (rand() & 0x03);
            }
            /* Heart */
            if ((rand() & 0x1f) == 0)
            {
                rows[line][rand() % 20] = HEART;
            }
            reseed++;
            if (reseed == 3) reseed = 0;
        } else
        {
            if (isGameStarted)
            {
                /* Middle of scroll, do other work */
                update++;
                update &= 0x01;
                if (update == 0)
                {
                    /* Print total score */
                    sprintf(totalStr, "%u", total);
                    if (total < 10)
                    {
                        status1[0] = totalStr[0] - 0x20;
                        status1[1] = 0;
                        status1[2] = 0;
                        status1[3] = 0;
                        status1[4] = 0;
                    } else if (total < 100)
                    {
                        status1[0] = totalStr[0] - 0x20;
                        status1[1] = totalStr[1] - 0x20;
                        status1[2] = 0;
                        status1[3] = 0;
                        status1[4] = 0;
                    } else if (total < 1000)
                    {
                        status1[0] = totalStr[0] - 0x20;
                        status1[1] = totalStr[1] - 0x20;
                        status1[2] = totalStr[2] - 0x20;
                        status1[3] = 0;
                        status1[4] = 0;
                    } else if (total < 10000)
                    {
                        status1[0] = totalStr[0] - 0x20;
                        status1[1] = totalStr[1] - 0x20;
                        status1[2] = totalStr[2] - 0x20;
                        status1[3] = totalStr[3] - 0x20;
                        status1[4] = 0;
                    } else
                    {
                        status1[0] = totalStr[0] - 0x20;
                        status1[1] = totalStr[1] - 0x20;
                        status1[2] = totalStr[2] - 0x20;
                        status1[3] = totalStr[3] - 0x20;
                        status1[4] = totalStr[4] - 0x20;
                    }
                } else if (update == 1)
                {
                    /* Print countdown */
                    if (vblanks >= 60)
                    {
                        vblanks -= 60;
                        countdown--;
                        if (countdown == 0)
                        {
                            note2 = 65;
                            POKEY_WRITE.audf2 = note2;
                            volume2 = 0xaf;
                            POKEY_WRITE.audc2 = volume2;

                            if (total > best)
                            {
                                best = total;
                            }
                            isGameStarted = 0;
                            UpdateBest();
                        } else if (countdown <= 10)
                        {
                            note2 = 216;
                            POKEY_WRITE.audf2 = note2;
                            volume2 = 0xa8;
                            POKEY_WRITE.audc2 = volume2;
                        }
                    }
                    sprintf(totalStr, "%u", countdown);
                    if (countdown < 10)
                    {
                        status2[18] = 0;
                        status2[19] = totalStr[0] - 0x20;
                        status2[20] = 0;
                    } else if (countdown < 100)
                    {
                        status2[18] = 0;
                        status2[19] = totalStr[0] - 0x20;
                        status2[20] = totalStr[1] - 0x20;
                    } else
                    {
                        status2[18] = totalStr[0] - 0x20;
                        status2[19] = totalStr[1] - 0x20;
                        status2[20] = totalStr[2] - 0x20;
                    }
                }
            } else
            {
                /* Check for game start */
                if (PEEK(STRIG0) == 1 && PEEK(STRIG1) == 1)
                {
                    /* No buttons pressed */
                    starting = 0;
                    memset(status2, 0, 16);
                    memset(&status2[24], 0, 16);

                    memcpy(&status2[36], "best", 4);

                    if (total == 0)
                    {} else if (total <= 10)
                    {
                        memcpy(status2, "really\x1f", 7);
                    } else if (total <= 25)
                    {
                        memcpy(status2, "try\x00harder", 10);
                    } else if (total <= 50)
                    {
                        memcpy(status2, "sore\x00yet\x1f", 9);
                    } else if (total <= 100)
                    {
                        memcpy(status2, "alright", 7);
                    } else if (total <= 200)
                    {
                        memcpy(status2, "wicked", 6);
                    } else
                    {
                        memcpy(status2, "pure\x00zen", 8);
                    }

                    /* Both sticks up? */
                    if (PEEK(STICK0) == 14 && PEEK(STICK1) == 14)
                    {
                        memcpy(&status1[13], "by\x00jim\x00higgins", 14);
                    } else
                    {
                        memcpy(&status1[13], "vector\000doubles", 14);
                    }
                } else if (starting == 0)
                {
                    countdown = 3;
                    vblanks = 0;
                    starting = 1;
                    note = 16;
                    POKEY_WRITE.audf1 = note;
                    volume = 0xa8;
                    POKEY_WRITE.audc1 = volume;
                    status2[19] = countdown + 16;
                    UpdateBest();
                }

                if (starting)
                {
                    if (PEEK(STRIG0) == 0 && PEEK(STRIG1) == 1)
                    {
                        memcpy(status2, "ready\x00player\x00one", 16);
                        memcpy(&status2[24], "alert\x00player\x00two", 16);
                    } else if (PEEK(STRIG0) == 1 && PEEK(STRIG1) == 0)
                    {
                        memcpy(status2, "alert\x00player\x00one", 16);
                        memcpy(&status2[24], "ready\x00player\x00two", 16);
                    } else
                    {
                        memcpy(status2, "ready\x00player\x00one", 16);
                        memcpy(&status2[24], "ready\x00player\x00two", 16);
                    }

                    if (vblanks >= 60)
                    {
                        vblanks -= 60;
                        countdown--;

                        if (countdown != 0)
                        {
                            note = 16;
                            POKEY_WRITE.audf1 = note;
                            volume = 0xa8;
                            POKEY_WRITE.audc1 = volume;
                            status2[19] = countdown + 16;
                        } else
                        {
                            note = 14;
                            POKEY_WRITE.audf1 = note;
                            volume = 0xaf;
                            POKEY_WRITE.audc1 = volume;
                            countdown = 120;
                            total = 0;
                            memset(status2, 0, sizeof(status2));
                            memcpy(status2, "score", 5);
                            memcpy(&status2[36], "best", 4);
                            memset(rows, 0, sizeof(rows));
                            memset(filter, FILTER_IDLE, sizeof(filter));
                            hposp0 = 88;
                            hposp0row = 5;
                            hposp1 = 160;
                            hposp1row = 14;

                            mainbgcolor += 16;
                            filterbgcolor += 16;
                            statusbgcolor += 16; // same as main char color
                            p0color += 16;
                            p1color += 16;
                            OS.color4 = mainbgcolor;
                            OS.pcolr0 = p0color;
                            OS.pcolr1 = p1color;
                            POKE(77, 0); // disable attract mode
                            isGameStarted = 1;
                        }
                    }
                } else
                {
                    status2[19] = 0;
                }
            }
        }

        if (isGameStarted)
        {
            /* Move the players */
            MOVE_PLAYER(STRIG0, STICK0, hposp0, center0);
            MOVE_PLAYER(STRIG1, STICK1, hposp1, center1);
        }
    };
}
