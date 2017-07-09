/* 
 * ncurses program for spinning ascii bitcoin
 *
 * compile with `gcc coin.c -o coin -lncurses`
 *
 * by matt farrugia <matomatical@gmail.com>
 * adapted from ascii art by max lee
 */

#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>

#include "coin-frames.h"

int main(int argc, char **argv) {
	
	// curses init stuff
	initscr();
	int x, y;
	getmaxyx(stdscr, y, x); // note: this is a macro so & not needed


	// 											 +1 because windows are wrapped
	//											 so without this, the \n will
	// 											 end the next line!!
	WINDOW *coin = newwin(coin_height, coin_width+1, 2, (x - coin_width) / 2);

	for (unsigned int i = 0; ; i++) {
		// clear the screen and draw a coin
		clear();

		// also draw the frame number and reset the cursor
		// render!
		// refresh();
		mvprintw(0, 0, "%d", i % coin_n_frames);
		refresh();

		mvwprintw(coin, 0, 0, coin_frames[i % coin_n_frames]);
		wrefresh(coin);

		// move cursor out of the way
		move(y-1, 0);
		refresh();
		
		// sleep for a moment
		usleep(100000);
	}

	delwin(coin);
	endwin();

	return 0;
}
