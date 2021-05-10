# Fortuna mines
Minesweeper for the LaFortuna. 
It has 3 board sizes based on Microsofts Minesweeper:
- Beginner: 9x9, 10 mines
- Intermediate: 16x16, 20 mines
- Expert: 30x16, 99 mines

![Main menu](/images/fortuna_mines_/main_menu.jpg)
![Intermediate board](/images/fortuna_mines_/medium_board.jpg)
![Expert board](/images/fortuna_mines_/expert_board.jpg)
![Failed expert board, on mines screen](/images/fortuna_mines_/mines_screen.jpg)

## Installing
You can either download the `mines.hex` file from releases and upload using:
```shell
dfu-programmer at90usb1286 erase
dfu-programmer at90usb1286 flash mines.hex 
```
or compile and install using the provided makefile.

## Playing

Grey cells are undiscovered, white cells are empty, cells with numbers have n adjacent mines. Avoid detonating the mines whilst attempting to tag all mines or clear the board.
Pink cells are tagged, yellow are questioned.

### Movement
- Uses rotary encoder to move either left/right or up/down
- Use up to toggle between left/right or up/down movement
- Left to tag, right button to question
- Down pauses the game
- Centre discovers and undiscovered cell, or sweeps if held down

## License breakdown
- mines.c, mines.h - BSD 3-Clause
- lcd/\* - CC-BY
- rios - BSD 3-Clause
- ruota - (C) Peter Dannegger  
