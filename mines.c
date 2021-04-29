/* COMP2215 Task 5---SKELETON */

#include "os.h"
#include <stdlib.h>


int blink(int);
int update_dial(int);
int collect_delta(int);
int check_switches(int);


int position = 0;


void main(void) {
    os_init();

    os_add_task( blink,            30, 1);
    os_add_task( collect_delta,   500, 1);
    os_add_task( check_switches,  100, 1);
//	os_add_task( freeram_output,   20, 1);
//	os_add_task( lose_mem,        250, 1);

    sei();
    for(;;){}

}


int collect_delta(int state) {
	position += os_enc_delta();
	return state;
}


int check_switches(int state) {

	if (get_switch_press(_BV(SWN))) {
			display_string("North\n");
	}

	if (get_switch_press(_BV(SWE))) {
			display_string("East\n");
	}

	if (get_switch_short(_BV(SWS))) {
			display_string("South\n");
	}

	if (get_switch_long(_BV(SWS))){
		f_mount(&FatFs, "", 0);
		if (f_open(&File, "myfile.txt", FA_READ) == FR_OK) {
			uint16_t index = f_size(&File);
			f_lseek(&File, index);
			uint8_t arrayIndex = 0;
			uint16_t* last25Newlines = calloc(25,25*sizeof(uint16_t));

			char* character = malloc(sizeof(char));
			UINT* br = malloc(sizeof(UINT));
			display_string("\nLast 25 lines of saved data:\n");
			for(;index > 0 && arrayIndex < 25; ){
				index--;
				f_read(&File, character,1,br);
				if(*character == '\n'){
					last25Newlines[arrayIndex] = index;
					arrayIndex++;
				}
				f_lseek(&File,index);
			}

			for(arrayIndex = 24; arrayIndex > 0; arrayIndex--){
				uint8_t lineLength = last25Newlines[arrayIndex-1] - last25Newlines[arrayIndex];
				if(lineLength == 0){
					continue;
				}
				char* line = calloc(lineLength, lineLength*sizeof(char));

				f_read(&File,line,lineLength,br);
				sprintf(line,line);
				display_string(line);
				free(line);
			}

			free(character);
			free(br);
			free(last25Newlines);
			f_close(&File);
		} else {
			display_string("Can't read file! \n");
		}
	}

	if (get_switch_press(_BV(SWW))) {
			display_string("West\n");
	}

	if (get_switch_long(_BV(SWC))) {
		f_mount(&FatFs, "", 0);
		if (f_open(&File, "myfile.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {
			f_lseek(&File, f_size(&File));
			f_printf(&File, "Encoder position is: %d \r\n", position);
			f_close(&File);
			display_string("Wrote position\n");
		} else {
			display_string("Can't write file! \n");
		}

	}

	if (get_switch_short(_BV(SWC))) {
			display_string("[S] Centre\n");
	}

	if (get_switch_rpt(_BV(SWN))) {
			display_string("[R] North\n");
	}

	if (get_switch_rpt(_BV(SWE))) {
			display_string("[R] East\n");
	}

//	if (get_switch_rpt(_BV(SWS))) {
//			display_string("[R] South\n");
//	}

	if (get_switch_rpt(_BV(SWW))) {
			display_string("[R] West\n");
	}

	if (get_switch_rpt(SWN)) {
			display_string("[R] North\n");
	}


	if (get_switch_long(_BV(OS_CD))) {
		display_string("Detected SD card.\n");
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
	display_string_xy(out, 290, 000);
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


// Minesweeper stuff

// xorshift32 taken from wikipedia example
struct xorshift32_state {
    uint32_t a;
}

uint32_t xorshift32(struct xorshift32_state *state) {
    uint32_t x = state->a;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return state->a = x;
}
