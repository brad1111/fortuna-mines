/* COMP2215 Task 5---SKELETON */

#include "os.h"
#include "mines.h"
#include <stdlib.h>


int blink(int);
int update_dial(int);
int collect_delta(int);
int check_switches(int);
int freeRam();
int freeram_output(int);


int position = 0;


// Minesweeper stuff

volatile uint16_t rng_state;
volatile rectangle selection_position;
volatile uint8_t position_states[BOARD_ITEMS];
volatile uint8_t scroll_direction = 0;
volatile uint8_t game_started = 0;

//XORSHIFT16
uint16_t rng() {
    uint16_t x = rng_state;
    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 8;
    rng_state = x;
	return x;
}

/*
** disallowed is a length 9 array which contains the elements surrounding the players position
** all disallowed values must be one more than they represent
*/
uint16_t* generate_mines(uint16_t* disallowed){


	// clear position_states
	for(int i = 0; i < BOARD_ITEMS; i++){
		position_states[i] = 0;
	}

	uint16_t* rngValues = malloc(MAX_MINES * sizeof(uint16_t));

	for(int i = 0; i < MAX_MINES; i++){
		uint16_t rngValue = 0;
		do{
			rngValue = rng() % BOARD_ITEMS;
			//check to see if value is in disallowed
			for(int j = 0; j < MAX_MINES; j++){
				if(rngValue == rngValues[j] + 1 //|| //mine already placed
//				   (j < 9 && rngValue == disallowed[j])//mine is diallowed
				){
					rngValue = 0;
					break;
				}
			}

			//Use 0 , because our RNG cant generate 0
		} while(rngValue == 0);

		position_states[rngValue - 1] = 4;
		rngValues[i] = rngValue - 1;
	}
	return rngValues;
}

uint8_t can_discover(int i){
	return (position_states[i]|4) == 4; //if we can discover 000 | 100 = 100
}

uint8_t is_discovered(int i){
	return (position_states[i]|4) == 5;  //001 | 100 == 101 will tell if discovered
}

uint8_t is_tagged(int i){
	return (position_states[i]|4) == 6; //010 | 100 == 110 will tell if we have a tag
}

uint8_t is_questioned(int i){
	return (position_states[i]|4) == 7; //011 | 100 == 111 will tell if we have a question
}

uint8_t is_mine(int i){
	return (position_states[i]|3) == 7; //100 | 011 == 111 will tell if we have a mine
}


uint8_t adjacent_mines(int i, uint8_t ree){
	uint16_t cellsToCheck[8];
	uint8_t counter = 0;

	int above_left = i-(BOARD_SIZE_X+1); int above = i-BOARD_SIZE_X; int above_right = i-(BOARD_SIZE_X-1);
	int left = i-1; int right = i + 1;
	int below_left = i+(BOARD_SIZE_X-1); int below = i+BOARD_SIZE_X; int below_right = i+(BOARD_SIZE_X+1);

	if(i==0){
		//TOP-LEFT
		counter = 3;
		cellsToCheck[0] = right;
		cellsToCheck[1] = below_right;
		cellsToCheck[2] = below;
	} else if (i==BOARD_SIZE_X - 1){
		//TOP-RIGHT
		counter = 3;
		cellsToCheck[0] = left;
		cellsToCheck[1] = below_left;
		cellsToCheck[2] = below;
	} else if (i==(BOARD_SIZE_Y -1) * BOARD_SIZE_X){
		//BOTTOM-LEFT
		counter = 3;
		cellsToCheck[0] = above;
		cellsToCheck[1] = above_right;
		cellsToCheck[2] = right;
	} else if (i==((BOARD_SIZE_Y * BOARD_SIZE_X) - 1)){
		//BOTTOM-RIGHT
		counter = 3;
		cellsToCheck[0] = left;
		cellsToCheck[1] = above_left;
		cellsToCheck[2] = above;
	} else if (i < BOARD_SIZE_X){
		//TOP row
		counter = 5;
		cellsToCheck[0] = left;
		cellsToCheck[1] = below_left;
		cellsToCheck[2] = below;
		cellsToCheck[3] = below_right;
		cellsToCheck[4] = right;
	} else if (i >= (BOARD_SIZE_Y - 1) * BOARD_SIZE_X){
		//Bottom ROw
		counter = 5;
		cellsToCheck[0] = left;
		cellsToCheck[1] = above_left;
		cellsToCheck[2] = above;
		cellsToCheck[3] = above_right;
		cellsToCheck[4] = right;
	} else if (i % BOARD_SIZE_X == 0){
		//Left column
		counter = 5;
		cellsToCheck[0] = above;
		cellsToCheck[1] = above_right;
		cellsToCheck[2] = right;
		cellsToCheck[3] = below_right;
		cellsToCheck[4] = below;
	} else if (i % BOARD_SIZE_X == BOARD_SIZE_X - 1){
		//Right column
		counter = 5;
		cellsToCheck[0] = above;
		cellsToCheck[1] = above_left;
		cellsToCheck[2] = left;
		cellsToCheck[3] = below_left;
		cellsToCheck[4] = below;
	} else {
		//Other cases
		counter = 8;
		cellsToCheck[0] = left;
		cellsToCheck[1] = above_left;
		cellsToCheck[2] = above;
		cellsToCheck[3] = above_right;
		cellsToCheck[4] = right;
		cellsToCheck[5] = below_right;
		cellsToCheck[6] = below;
		cellsToCheck[7] = below_left;
	}

	uint8_t adjacentCounter = 0;
	uint16_t cellId;
	for(uint8_t j = 0; j < counter; j++){
		cellId = cellsToCheck[j];
		if(is_mine(cellId)){
			adjacentCounter++;
		}
	}

	return adjacentCounter;
}

void printCell(int pos){
	if(is_discovered(pos) && is_mine(pos)){
		//show red
		update_selection_position(pos,RED);
	} else if (is_discovered(pos)){
		//show white
		update_selection_position(pos,WHITE);
	} else if (is_tagged(pos)){
		//show magenta
		update_selection_position(pos,MAGENTA);
	} else if (is_questioned(pos)){
		//show yellow
		update_selection_position(pos,YELLOW);
	} else {
		//show grey
		update_selection_position(pos,GREY);
	}
}


//Prints the grid as is
void printGrid(){
	clear_screen();
	for(int i = 0; i < BOARD_ITEMS; i++){
		printCell(i);
	}
}

void printMines(){
	rectangle sqr = {0,10,0,10}; clear_screen();
	for(int i = 0; i < BOARD_ITEMS; i++){
		if(i != 0 && i % BOARD_SIZE_X == 0){
//			display_char('\n');
			sqr.left = 0;
			sqr.right = 10;
			sqr.top += 10;
			sqr.bottom += 10;
		}
	//	int m = 0;
//		for(; m < MAX_MINES; m++){
//			if(mines[m] == i){
//				break;
//			}
//		}


		//when m has reached the max, there isn't a mine in this pos
		if(!is_mine(i)){ // NOT A MINE
//			display_char('-');
			fill_rectangle(sqr, WHITE);
			char out[1];
			sprintf(out, "%d", adjacent_mines(i,0));
			display_string_xy(out, sqr.left, sqr.top);
		} else { //IS A MINE
//			display_char('x');
			fill_rectangle(sqr, RED);
		}
		//increment for next time
		sqr.left += 10;
		sqr.right += 10;
	}
}

void discover(){
	int pos = position;
	if(is_mine(pos)){
		//explode
		game_started = 0;
	} else if (can_discover(pos)){
		position_states[pos] |= 1;
	}
}

/*
** Pos is the position of the clickwheel
*/
void update_selection_position(int position, uint16_t colour){
	int x = position % BOARD_SIZE_X;
	int y = (position / BOARD_SIZE_X) % BOARD_SIZE_Y;

	selection_position.left   = x*10;
	selection_position.right  =	x*10+10;
	selection_position.top	  = y*10;
	selection_position.bottom = y*10+10;
	fill_rectangle(selection_position, colour);
}


void main(void) {
    os_init();

    os_add_task( blink,            25, 1);
    os_add_task( collect_delta,   200, 1);
    os_add_task( check_switches,  100, 1);
	os_add_task( freeram_output,   20, 1);
//	os_add_task( lose_mem,        250, 1);

	rng_state = 3;

//setup seclection position
	update_selection_position(0,BLUE);


    sei();
    for(;;){}

}


int collect_delta(int state) {
	int oldPos = position % BOARD_ITEMS;
	if(scroll_direction == SCROLL_HORIZONTAL){
		position += os_enc_delta();
	} else {
		position += BOARD_SIZE_X * os_enc_delta();
	}

	if (position < 0){
		position = BOARD_ITEMS - position;
	}


////		int left = position * 10;
////		rectangle a = {left,left+10,0,10} ;
////		clear_screen();
////		fill_rectangle(a,GREEN);

	uint16_t oldPosColour;
	char output[30];
	sprintf(output,"stat:%d\npos:%d\n", position_states[position], position);
	display_string_xy(output, 0, 210);
	printCell(oldPos);
	update_selection_position(position, BLUE);
	return state;
}


int check_switches(int state) {

	if (get_switch_press(_BV(SWN))) {
		scroll_direction = !scroll_direction;
	}

	if (get_switch_press(_BV(SWE))) {
			display_string("East\n");
	}

	if (get_switch_press(_BV(SWS))) {
		printMines();
	}

	if (get_switch_press(_BV(SWW))) {
			display_string("West\n");
	}


	if (get_switch_press(_BV(SWC))) {
		if(!game_started){
			generate_mines(NULL);
		    printGrid();
			game_started = 1;
		} else {
			discover();
		}

	}

	if (get_switch_rpt(_BV(SWN))) {
			display_string("[R] North\n");
	}

	if (get_switch_rpt(_BV(SWE))) {
			display_string("[R] East\n");
	}

	if (get_switch_rpt(_BV(SWS))) {
			display_string("[R] South\n");
	}

	if (get_switch_rpt(_BV(SWW))) {
			display_string("[R] West\n");
	}

	if (get_switch_rpt(SWN)) {
			display_string("[R] North\n");
	}


	return state;
}




int blink(int state) {
	static int light = 0;
	uint8_t level;

	if (light < -120) {
		state = 1;
	} else if (light > 254) {
		state = -20;
	}


	/* Compensate somewhat for nonlinear LED
           output and eye sensitivity:
        */
	if (state > 0) {
		if (light > 40) {
			state = 2;
		}
		if (light > 100) {
			state = 5;
		}
	} else {
		if (light < 180) {
			state = -10;
		}
		if (light < 30) {
			state = -5;
		}
	}
	light += state;

	if (light < 0) {
		level = 0;
	} else if (light > 255) {
		level = 255;
	} else {
		level = light;
	}

	os_led_brightness(level);
	return state;
}


int freeram_output (int state){
	char out[10];
	sprintf(out, "%d\n", freeRam());
	display_string_xy(out, 290, 200);
	if(state == 10){
		return 0;
	} else {
		return ++state;
	}
}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


