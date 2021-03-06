/* Loosly Based on COMP2215 Task 5---SKELETON */
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


#include "mines.h"


int update_dial(int);
int collect_delta(int);
int check_switches(int);
int freeRam();
#ifdef DEBUG
int freeram_output(int);
#endif //DEBUG
adjCells adjacent_cells(int);
uint8_t is_mine(int);
void printMenu(char*,char*,uint8_t);
void setup_game();
void discover_pos(int);
void printCell(int);
void update_selection_position(int,uint16_t);
void update_selection_text(int);
void printMinesLeft();
void printMenuSelected(uint8_t,uint8_t);
void main_menu();

// position stuff
int position = 0;
uint8_t menuPosition = 0;
uint8_t menuEntries = 0;
uint8_t menuStatus = MENU_NONE;



// Minesweeper stuff

volatile uint16_t rng_state = 0;
static uint16_t EEMEM RNG_STATE;
volatile rectangle selection_position;
volatile uint8_t position_states[30*16];
volatile uint8_t scroll_direction = 0;
volatile uint8_t game_state = GAME_STATE_NONE;
volatile uint8_t mines_untagged= 0;
volatile uint8_t cells_tagged = 0;
volatile uint16_t cells_discovered = 0;
volatile uint16_t rng_count = 0;


//board sizing
uint8_t BOARD_SIZE_X = 30;
uint8_t BOARD_SIZE_Y = 16;
uint8_t MAX_MINES = 99;
uint8_t OFFSET_X = 1;
uint8_t OFFSET_Y = 1;

//XORSHIFT16
uint16_t rng() {
    uint16_t x = rng_state;
    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 8;
    rng_state = x;
	//writes to EEPROM in generate mines to decrease wear on EEPROm
	return x;
}


/*
** disallowed is a length 9 array which contains the elements surrounding the players position
** all disallowed values must be one more than they represent
*/
void generate_mines(uint16_t pos){
	// get states adjacent to pos
	adjCells disallowedAdjacent = adjacent_cells(pos);

	// clear position_states
	for(int i = 0; i < BOARD_ITEMS; i++){
		position_states[i] = 0;
	}

	for(int i = 0; i < MAX_MINES; i++){
		uint16_t rngValue = 0;
		do{
			rngValue = rng() % BOARD_ITEMS;
			//check to see if value is in disallowed
			if(rngValue == pos + 1){//don't put mine on initial place
				rngValue = 0;
			}
			for(int j = 0; j < MAX_MINES; j++){
				if(is_mine(rngValue - 1) //|| //mine already placed
//				   (j < 9 && rngValue == diVsallowed[j])//mine is diallowed
				){
					rngValue = 0;
					break;
				}
			}

			for(int j = 0; j < disallowedAdjacent.counter; j++){ //leave gap around inital placement
				if(rngValue == disallowedAdjacent.cells[j] + 1){
					rngValue = 0;
					break;
				}
			}
			//Use 0 , because our RNG cant generate 0
		} while(rngValue == 0);

		position_states[rngValue - 1] = 4;
	}
	//update eeprom RNG
	eeprom_update_word(&RNG_STATE, rng_state);
	rng_count++;
}

uint16_t noOfMines(){
	uint16_t mineCount = 0;
	for(uint16_t i = 0; i < BOARD_ITEMS; i++){
		if(is_mine(i)){
			mineCount++;
		}
	}
	return mineCount;
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



/**
 * pos - position you want to get adjacent of
 * returns - adjCells, cells[8] array of cells, counter (the number of elements in array used)
 */
adjCells adjacent_cells(int pos){

	adjCells adjacentCells;

	int above_left = pos-(BOARD_SIZE_X+1); int above = pos-BOARD_SIZE_X; int above_right = pos-(BOARD_SIZE_X-1);
	int left = pos-1; int right = pos + 1;
	int below_left = pos+(BOARD_SIZE_X-1); int below = pos+BOARD_SIZE_X; int below_right = pos+(BOARD_SIZE_X+1);

	if(pos==0){
		//TOP-LEFT
		adjacentCells.counter = 3;
		adjacentCells.cells[0] = right;
		adjacentCells.cells[1] = below_right;
		adjacentCells.cells[2] = below;
	} else if (pos==BOARD_SIZE_X - 1){
		//TOP-RIGHT
		adjacentCells.counter = 3;
		adjacentCells.cells[0] = left;
		adjacentCells.cells[1] = below_left;
		adjacentCells.cells[2] = below;
	} else if (pos==(BOARD_SIZE_Y -1) * BOARD_SIZE_X){
		//BOTTOM-LEFT
		adjacentCells.counter = 3;
		adjacentCells.cells[0] = above;
		adjacentCells.cells[1] = above_right;
		adjacentCells.cells[2] = right;
	} else if (pos==((BOARD_SIZE_Y * BOARD_SIZE_X) - 1)){
		//BOTTOM-RIGHT
		adjacentCells.counter = 3;
		adjacentCells.cells[0] = left;
		adjacentCells.cells[1] = above_left;
		adjacentCells.cells[2] = above;
	} else if (pos < BOARD_SIZE_X){
		//TOP row
		adjacentCells.counter = 5;
		adjacentCells.cells[0] = left;
		adjacentCells.cells[1] = below_left;
		adjacentCells.cells[2] = below;
		adjacentCells.cells[3] = below_right;
		adjacentCells.cells[4] = right;
	} else if (pos >= (BOARD_SIZE_Y - 1) * BOARD_SIZE_X){
		//Bottom ROw
		adjacentCells.counter = 5;
		adjacentCells.cells[0] = left;
		adjacentCells.cells[1] = above_left;
		adjacentCells.cells[2] = above;
		adjacentCells.cells[3] = above_right;
		adjacentCells.cells[4] = right;
	} else if (pos % BOARD_SIZE_X == 0){
		//Left column
		adjacentCells.counter = 5;
		adjacentCells.cells[0] = above;
		adjacentCells.cells[1] = above_right;
		adjacentCells.cells[2] = right;
		adjacentCells.cells[3] = below_right;
		adjacentCells.cells[4] = below;
	} else if (pos % BOARD_SIZE_X == BOARD_SIZE_X - 1){
		//Right column
		adjacentCells.counter = 5;
		adjacentCells.cells[0] = above;
		adjacentCells.cells[1] = above_left;
		adjacentCells.cells[2] = left;
		adjacentCells.cells[3] = below_left;
		adjacentCells.cells[4] = below;
	} else {
		//Other cases
		adjacentCells.counter = 8;
		adjacentCells.cells[0] = left;
		adjacentCells.cells[1] = above_left;
		adjacentCells.cells[2] = above;
		adjacentCells.cells[3] = above_right;
		adjacentCells.cells[4] = right;
		adjacentCells.cells[5] = below_right;
		adjacentCells.cells[6] = below;
		adjacentCells.cells[7] = below_left;
	}

	return adjacentCells;
}

/**
 * Gets the value of a cell based on how many adjacent mines
 */
uint8_t adjacent_mines(int pos, uint8_t clear){
	adjCells adjacentCells = adjacent_cells(pos);

	uint8_t adjacentCounter = 0;
	uint16_t cellId;
	for(uint8_t j = 0; j < adjacentCells.counter; j++){
		cellId = adjacentCells.cells[j];
		if(is_mine(cellId)){
			adjacentCounter++;
		} else if (clear){
			//will be something we want to update and clear
			discover_pos(cellId);
			printCell(cellId);
		}
	}

	return adjacentCounter;
}

/**
 * If the no of adjacent mines tagged equals adjacent mines then sweep
 */
void sweep_adjacent(int pos){
	adjCells adjacentCells = adjacent_cells(pos);
	uint8_t mine_counter = 0;
	uint8_t tag_counter = 0;
	uint16_t cellId;

	for(uint8_t j = 0; j < adjacentCells.counter; j++){
		cellId = adjacentCells.cells[j];
		if(is_mine(cellId)){
			mine_counter++;
		}
		if(is_tagged(cellId)){
			tag_counter++;
		}
	}

	if(mine_counter == tag_counter){
		for(uint8_t j = 0; j < adjacentCells.counter; j++){
			cellId = adjacentCells.cells[j];
			discover_pos(cellId);
			printCell(cellId);
		}
	}
}


/*
** Clears adjacent cells that are 0s
 */
void clear_adjacent(int pos){
	adjCells adjacentCells = adjacent_cells(pos);
	uint16_t cellId;
	for(uint8_t j=0; j < adjacentCells.counter; j++){
		cellId = adjacentCells.cells[j];
		discover_pos(cellId);
		printCell(cellId);
	}
}

void printCell(int pos){
	if(game_state == GAME_STATE_STARTING){
		//show grey
		update_selection_position(pos,GREY);
	}
	else if(game_state == GAME_STATE_ENDED && is_mine(pos)){
		//show red
		update_selection_position(pos,RED);
	} else if (is_discovered(pos)){
		//show white
		update_selection_text(pos);
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
	printMinesLeft();
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
			char out[4];
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

/**
 * Prints no of mines left to sweep
 */
void printMinesLeft(){

	char out[10];
#ifdef DEBUG
	sprintf(out, "mines:%2d", mines_untagged);
	display_string_xy(out,140,200);
	sprintf(out, "tags:%2d", cells_tagged);
	display_string_xy(out,140,210);
	sprintf(out, "clr:%2d", cells_discovered);
	display_string_xy(out,140,220);
#else
	uint8_t value;
	if(cells_tagged > MAX_MINES){
		value = 0;
	} else {
		value = MAX_MINES - cells_tagged;
	}
	sprintf(out, "mines:%2d", value);
	display_string_xy(out,140,200);
#endif //DEBUG

}

void untag(int pos){
	if(is_mine(pos)){
		mines_untagged++;
	}
	cells_tagged--;
	position_states[pos] &= (255-3); //Just get rid of the tag
	printMinesLeft();
}

void discover(){
	discover_pos(position);
}

#define GAME_OVER_MENU_ITEMS_COUNT 3
void gameOverCommon(char* title){
	game_state = GAME_STATE_ENDED;
	menuStatus = MENU_GAMEOVER;
	char items[GAME_OVER_MENU_ITEMS_COUNT][MAX_LETTERS_IN_MENU] = {
	"New Game",
	"View Board",
	"Main Menu"
    };
	clear_screen();
	printMenu(title, (char*) items, GAME_OVER_MENU_ITEMS_COUNT);
}

void win(){
		gameOverCommon("You won!");
}

void discover_pos(int pos){

	if (can_discover(pos)){
		if(is_mine(pos)){
			//explode
			gameOverCommon("Game over!");
		} else {
			position_states[pos] |= 1;
			uint8_t value = adjacent_mines(pos,0);
			if(value == 0){
				//actually clear, yes this is not ideal
				adjacent_mines(pos,1);
			}
			cells_discovered++;
			if(cells_discovered == ((uint16_t) BOARD_SIZE_X * (uint16_t) BOARD_SIZE_Y) - (uint16_t) MAX_MINES){
				win();
			}
		}
	}
}

void tag(){
	int pos = position;
	if(is_tagged(pos)){
		//untag
		untag(pos);
		return;
	} else if(is_questioned(pos)){
		//convert to tag
		position_states[pos] &= (255-1); //just get rid of final bit
	} else if (can_discover(pos)){
		//tag it
		position_states[pos] |= 2;
	}

	//check to see if we need to update tag counter
	if(is_mine(pos)){
		mines_untagged--;
	}
	cells_tagged++;
	printMinesLeft();

	//check to see if this will win the game
	if(mines_untagged == 0 && cells_tagged == MAX_MINES){
		win();
	}
}

void question(){
	int pos = position;
	if(is_questioned(pos)){
		//unquestion
		position_states[pos] &= (255-3); //get rid of question
	} else if(is_tagged(pos)){
		//make pos
		untag(pos);
		position_states[pos] |= 3; //question is one bit higher than tag
	} else if(can_discover(pos)){
		position_states[pos] |= 3;
	}
}

/*
** Pos is the position of the clickwheel
*/
void update_selection_position(int position, uint16_t colour){
	int x = position % BOARD_SIZE_X;
	int y = (position / BOARD_SIZE_X) % BOARD_SIZE_Y;

	selection_position.left   = (OFFSET_X + x)*10;
	selection_position.right  =	(OFFSET_X + x)*10+10;
	selection_position.top	  = (OFFSET_Y + y)*10;
	selection_position.bottom = (OFFSET_Y + y)*10+10;
	fill_rectangle(selection_position, colour);
}


char selection_text[4];
void update_selection_text(int position){
	update_selection_position(position, WHITE);
	uint8_t val = adjacent_mines(position,0);
	if(val != 0){
		sprintf(selection_text, "%d", adjacent_mines(position,0));
		display_string_xy(selection_text, selection_position.left, selection_position.top);
	}
}

void instructions(){
	menuStatus = MENU_INSTRUCTIONS;
	clear_screen();
	display_string_xy("Instructions:\n",140,0);
	display_string("Controls: \n\n");
	display_string("Move left/right or up/down:\n");
	display_string("- Spin wheel (depends on movement direction)\n");
	display_string("Discover selected cell:\n");
	display_string("- Centre button\n");
	display_string("Toggle movement direction: \n");
	display_string("- Up button\n");
	display_string("- Toggles between left/right or up/down movement\n");
	display_string("Tag cell: \n");
	display_string("- Left button\n");
	display_string("Question cell: \n");
	display_string("- Right button\n");
	display_string("Pause: \n");
	display_string("- Right button\n");
	display_string("\nScreen elements: \n\n");
	display_string("- Grey square: undiscovered\n");
	display_string("- Red square: exploded mine\n");
	display_string("- White or numbered square: discovered\n");
	display_string("- Pink square: tagged\n");
	display_string("- Yellow square: questioned\n");
	display_string("\nGoal: \n");
	display_string("Clear the board or tag all mines, without exploding any mines\n");
	display_string("\nPress centre button to go back...");

}

void main(void) {
    /* 8MHz clock, no prescaling (DS, p. 48) */
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    init_lcd();
    os_init_scheduler();
    os_init_ruota();

    os_add_task( collect_delta,   200, 1);
    os_add_task( check_switches,  100, 1);
	#ifdef DEBUG
	os_add_task( freeram_output,   20, 1);
	#endif


//setup seclection position
	update_selection_position(0,BLUE);

//	//setup clock prescaler for rng seed (CSN12 and CSN10 for /1024 prescaler)
//	TCCR1B |= 4;

	//setup watchdog interrupt so we can get a TRNG value to give as seed for PRNG

	//clear watchdog timer
	//MCUSR &= ~(1<<)

	//write logical 1 to both WDCE and WDE
//	WDTCSR |= (1<<WDCE) | (1<<WDE);

	//disable WD timer
//
//	WDTCSR = 0x00;
//
//	//enable WD timer with Change enable WDCE and WDIE (interrupt enable) and prescaler to ~1s
//	WDTCSR |= (1<<WDCE) | (1<<WDIE) | (1<<WDP2) | (1<<WDP1);

	//Enable ADC and start conversion
//	ADCSRA |= (1<<ADEN) | (1<<ADSC);

	//random seed inspired from https://www.avrfreaks.net/forum/random-number-generation-0

	rng_state = eeprom_read_word(&RNG_STATE);

//	printGrid();
	main_menu();

    sei();
    for(;;){}

}

void main_menu(){
	clear_screen();
	char arr[4][MAX_LETTERS_IN_MENU] = {
	"Play Beginner",
	"Play Intermediate",
	"Play Expert",
	"Instructions"
	};
	menuStatus = MENU_START;
	game_state = GAME_STATE_NONE;
	printMenu("Fortuna Mines",(char*) arr,4);
}


int collect_delta(int state) {
	if(!menuStatus){
		//in game
		int oldPos = position % BOARD_ITEMS;
		if(scroll_direction == SCROLL_HORIZONTAL){
			position += os_enc_delta();
		} else {
			position += BOARD_SIZE_X * os_enc_delta();
		}

		if (position < 0){
			position = BOARD_ITEMS - position;
		} else if (position > BOARD_ITEMS){
			position = position % BOARD_ITEMS;
		}


		#ifdef DEBUG
		char output[30];
		sprintf(output,"stat:%d\npos:%d\nrng:%d;%d", position_states[position], position, rng_state, rng_count);
		display_string_xy(output, 0, 210);
		#endif //DEBUG
		printCell(oldPos);
		update_selection_position(position, BLUE);
	} else {
		//in menu
		uint8_t oldPos = menuPosition;
		menuPosition = (menuPosition + os_enc_delta()) % menuEntries;
		if(oldPos != menuPosition){
			printMenuSelected(oldPos, menuPosition);
		}
	}
	return state;
}

/*
** Sets up the game to before start state
*/
void setup_game(){
	mines_untagged= 0;
	cells_tagged = 0;
	cells_discovered = 0;
	menuStatus = 0;
	game_state = GAME_STATE_STARTING;
	printGrid();
}

int check_switches(int state) {

	if (get_switch_press(_BV(SWN))) {
		if(!menuStatus){
			scroll_direction = !scroll_direction;
		}
	}

	if (get_switch_press(_BV(SWE))) {
		if(!menuStatus){
			question();
		}
	}

	if (get_switch_press(_BV(SWS))) {
		if(!menuStatus){
			menuStatus = MENU_PAUSE;
			char items[][MAX_LETTERS_IN_MENU] = {
			"Resume",
			#ifdef DEBUG
			"[DEBUG] cheat",
			#endif //DEBUG
			"Quit to menu"
		    };
			clear_screen();
			printMenu("Paused", (char*) items, sizeof(items)/(MAX_LETTERS_IN_MENU * sizeof(char)));
		}
	}

	if (get_switch_press(_BV(SWW))) {
		if(!menuStatus){
			tag();
		}
	}


	if (get_switch_press(_BV(SWC))) {
		switch (menuStatus) {
			case MENU_NONE:
				if(game_state == GAME_STATE_STARTING){
					//setup seed based on time to press the first button
					if(rng_state == 0){
						rng_state = 0x0bae;
					}

					generate_mines(position);
					game_state = GAME_STATE_STARTED;
					printGrid();
					mines_untagged = MAX_MINES;
					cells_tagged = 0;
				} else if (game_state == GAME_STATE_ENDED){
					menuStatus = MENU_GAMEOVER;
					gameOverCommon("What next?");
					break;
				}
				discover();
				break;
			case MENU_START:
				switch (menuPosition) {
					case 0:
						//10 mines
						BOARD_SIZE_X = 9;
						BOARD_SIZE_Y = 9;
						OFFSET_X = 11;
						OFFSET_Y = 5;
						MAX_MINES=10;
						setup_game();
						break;
					case 1:
						//20 mines
						BOARD_SIZE_X = 16;
						BOARD_SIZE_Y = 16;
						OFFSET_X = 8;
						OFFSET_Y = 1;
						MAX_MINES=20;
						setup_game();
						break;
					case 2:
						//99 mines
						BOARD_SIZE_X = 30;
						BOARD_SIZE_Y = 16;
						OFFSET_X = 1;
						OFFSET_Y = 1;
						MAX_MINES=99;
						setup_game();
						break;
					case 3:
						instructions();
						break;
					default:
						break;
				}
				break;
			case MENU_PAUSE:
				switch (menuPosition){
					case 0:
						//resume
						menuStatus = MENU_NONE;
						clear_screen();
						printGrid();
						break;
					case 1:
						//cheat
#ifdef DEBUG
						menuStatus = MENU_NONE;
						clear_screen();
						printMines();
						break;
					case 2:
#endif //DEBUG
						//quit to menu
						main_menu();
					default:
						break;
				}
				break;
			case MENU_GAMEOVER:
				switch (menuPosition) {
					case 0:
						//New game
						game_state = GAME_STATE_STARTING;
						menuStatus = MENU_NONE;
						printGrid();
						break;
					case 1:
						//View board
						clear_screen();
						menuStatus = MENU_NONE;
						printGrid();
						break;
					case 2:
						//Main menu
						main_menu();
						break;
					default:
						break;
				}
				break;
			case MENU_INSTRUCTIONS:
				main_menu();
				break;
			default:
				break;
		}


	}

	if (get_switch_rpt(_BV(SWC))) {
		//Sweep
		if(!menuStatus && cells_discovered){
			sweep_adjacent(position);
		}
	}

	return state;
}



/**
 * Prints a menu based on inputs
 */
void printMenu(char* title, char *lines, uint8_t menu_entries){
	display_string_xy(title, 140,0);

	menuEntries = menu_entries;
	uint8_t offset = (22 - menu_entries) / 2;
	for(uint8_t i = 0; i < menu_entries; i++){
		display_string_xy(lines+MAX_LETTERS_IN_MENU*i, 140, LETTER_HEIGHT_IN_MENU * (offset + i));
		if(menuPosition == i){
			printMenuSelected(i,i);
		}
	}

}


void printMenuSelected(uint8_t old_pos, uint8_t new_pos){
	uint8_t offset = (22 - menuEntries) / 2;
	display_string_xy(" ", 130, 10 * (offset + old_pos));
	display_string_xy("*", 130, 10 * (offset + new_pos));
}

#ifdef DEBUG

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

#endif // DEBUG
