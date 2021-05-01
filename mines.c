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

		position_states[rngValue - 1] = 8;
		rngValues[i] = rngValue - 1;
	}
	return rngValues;
}

void printMines(){
	rectangle sqr = {0,10,0,10};
	clear_screen();
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
		if(position_states[i] % 16 == 0){ // NOT A MINE
//			display_char('-');
			fill_rectangle(sqr, WHITE);
		} else { //IS A MINE
//			display_char('x');
			fill_rectangle(sqr, RED);
		}
		//increment for next time
		sqr.left += 10;
		sqr.right += 10;
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
	int oldPos = position;
	if(scroll_direction == SCROLL_HORIZONTAL){
		position += os_enc_delta();
	} else {
		position += BOARD_SIZE_X * os_enc_delta();
	}



////		int left = position * 10;
////		rectangle a = {left,left+10,0,10} ;
////		clear_screen();
////		fill_rectangle(a,GREEN);

	uint16_t oldPosColour;
	if(position_states[oldPos] % 16 == 0){ //Not a mine
		oldPosColour = WHITE;
	} else { //is a mine
		oldPosColour = RED;
	}
	update_selection_position(oldPos, oldPosColour);
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
		uint16_t* mines = generate_mines(NULL);
		printMines(mines);
		free(mines);

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


