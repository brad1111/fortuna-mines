/*
BSD 3-Clause License

Copyright (c) 2021, Bradley Eaton
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "lcd.h"
#include "rios.h"
#include "ruota.h"

#ifndef MINES_H_
#define MINES_H_

// if debug mode define this here
#define DEBUG

//
// Cell
//
// 8 bit value
// 00000xxx - means state of object
// 00000y00 - means it is normal
// 00000y01 - means it is discovered, discovered + mine = exploded
// 00000y10 - means it is tagged
// 00000y11 - means it is questioned
// 00000x00 - refers to whether a mine (1 means it is )

#define BOARD_SIZE_X 30
#define BOARD_SIZE_Y 16
#define BOARD_ITEMS (BOARD_SIZE_X * BOARD_SIZE_Y)
#define MAX_MINES 99

#define SCROLL_HORIZONTAL 0
#define SCROLL_VERTICAL 1

#define MAX_LETTERS_IN_MENU 20
#define LETTER_HEIGHT_IN_MENU 10

#define MENU_NONE 0
#define MENU_START 1
#define MENU_PAUSE 2
#define MENU_GAMEOVER 3
#define MENU_INSTRUCTIONS 4

#define GAME_STATE_NONE 0
#define GAME_STATE_STARTING 1
#define GAME_STATE_STARTED 2
#define GAME_STATE_ENDED 3


typedef struct{
    uint16_t cells[8]; //The cells that are adjacent (up to 8)
    uint8_t counter; //The number of cells actually adjacent
} adjCells;

#endif // MINES_H_
