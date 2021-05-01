#ifndef MINES_H_
#define MINES_H_

//
// Cell
//
// 8 bit value
//
// 0000000x - refers to whether uncovered
// 000000x0 - refers to whether tagged
// 00000x00 - refers to whether questioned
// 0000x000 - refers to whether a mine

#define BOARD_SIZE_X 30
#define BOARD_SIZE_Y 16
#define BOARD_ITEMS (BOARD_SIZE_X * BOARD_SIZE_Y)
#define MAX_MINES 99

#define SCROLL_HORIZONTAL 0
#define SCROLL_VERTICAL 1

#endif // MINES_H_
