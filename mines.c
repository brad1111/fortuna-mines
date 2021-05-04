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
adjCells adjacent_cells(int);
uint8_t is_mine(int);


int position = 0;


// Minesweeper stuff

volatile uint16_t rng_state;
volatile rectangle selection_position;
volatile uint8_t position_states[BOARD_ITEMS];
volatile uint8_t scroll_direction = 0;
volatile uint8_t game_started = 0;
volatile uint8_t mines_untagged= 0;
volatile uint8_t cells_tagged = 0;
volatile uint16_t cells_discovered = 0;

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
void generate_mines(int pos){
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
	uint8_t adjacentCounter = 0;
	uint16_t cellId;
	for(uint8_t j=0; j < adjacentCells.counter; j++){
		cellId = adjacentCells.cells[j];
		discover_pos(cellId);
		printCell(cellId);
	}
}

void printCell(int pos){
	if(is_discovered(pos) && is_mine(pos)){
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

/**
 * Prints no of mines left to sweep
 */
void printMinesLeft(){
	char out[10];
	sprintf(out, "mines:%2d", mines_untagged);
	display_string_xy(out,160,200);
	sprintf(out, "tags:%2d", cells_tagged);
	display_string_xy(out,160,210);
	sprintf(out, "clr:%2d", cells_discovered);
	display_string_xy(out,160,220);


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

void win(){
		game_started = 0;
		display_string_xy("Win", 160, 120);
}

void discover_pos(int pos){

	if (can_discover(pos)){
		if(is_mine(pos)){
			//explode
			game_started = 0;
			display_string_xy("Game Over!", 160, 120);
		} else {
			position_states[pos] |= 1;
			uint8_t value = adjacent_mines(pos,0);
			if(value == 0){
				//actually clear, yes this is not ideal
				adjacent_mines(pos,1);
			}
			cells_discovered++;
			if(cells_discovered == BOARD_ITEMS - MAX_MINES){
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
	} else if(can_discover(pos) || is_tagged(pos)){
		//make pos
		untag(pos);
		position_states[pos] |= 3; //question is one bit higher than tag
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


char selection_text[2];
void update_selection_text(int position){
	update_selection_position(position, WHITE);
	uint8_t val = adjacent_mines(position,0);
	if(val != 0){
		sprintf(selection_text, "%d", adjacent_mines(position,0));
		display_string_xy(selection_text, selection_position.left, selection_position.top);
	}
}


void main(void) {
    os_init();

//    os_add_task( blink,            25, 1);
    os_add_task( collect_delta,   200, 1);
    os_add_task( check_switches,  100, 1);
	os_add_task( freeram_output,   20, 1);

	rng_state = 3;

//setup seclection position
	update_selection_position(0,BLUE);

	printGrid();

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
	} else if (position > BOARD_ITEMS){
		position = position % BOARD_ITEMS;
	}


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
		question();
	}

	if (get_switch_press(_BV(SWS))) {
		printMines();
	}

	if (get_switch_press(_BV(SWW))) {
		tag();
	}


	if (get_switch_press(_BV(SWC))) {
		if(!game_started){
			generate_mines(position);
		    printGrid();
			game_started = 1;
			mines_untagged = MAX_MINES;
			cells_tagged = 0;
			discover();
		} else {
			discover();
		}

	}

	if (get_switch_rpt(_BV(SWC))) {
		//Sweep
		sweep_adjacent(position);
	}

//	if (get_switch_rpt(_BV(SWN))) {
//			display_string("[R] North\n");
//	}
//
//	if (get_switch_rpt(_BV(SWE))) {
//			display_string("[R] East\n");
//	}
//
//	if (get_switch_rpt(_BV(SWS))) {
//			display_string("[R] South\n");
//	}
//
//	if (get_switch_rpt(_BV(SWW))) {
//			display_string("[R] West\n");
//	}
//
//	if (get_switch_rpt(SWN)) {
//			display_string("[R] North\n");
//	}


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


