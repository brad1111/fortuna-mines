#ifndef MINES_H_
#define MINES_H_

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

typedef struct{
    uint16_t cells[8]; //The cells that are adjacent (up to 8)
    uint8_t counter; //The number of cells actually adjacent
} adjCells;

#endif // MINES_H_
