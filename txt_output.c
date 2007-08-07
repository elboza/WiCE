#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<ctype.h>
#include<ncurses.h>
#include<math.h>
#include<gtk/gtk.h>
#include"main.h"
#include"list_utils.h"
//#include"parse2.h"
#include"pack.h"
//#include"init_game.h"
//#include"scheduler.h"
#include"execute.h"
#include"debug_output.h"
#include"txt_output.h"
#include"x11_output.h"

void curses_cell_refresh(int addr,struct process_thread *pt)
{
	int x,y,col;
	char car;
	addr2coords(addr,&x,&y);
	get_p_attr(pt->ptask->ID,&car,&col);
	mvaddch(y+yd,x+xd,car);
	refresh();
}
void init_txt()
{
	int count,curs_x,curs_y,col;
	char car;
	struct unpacked_op_mem mem;
	xd=1;		/*x offset from beginnig of screen*/
	yd=1;		/*y offest from beginning of screen*/
	xl=25;		/*x space from end of screen*/
	yl=1;		/*y space from end of screen*/
	initscr();
	cbreak();
	noecho();
	sc_x=COLS-xd-xl;
	max_sc_y=LINES-yd-yl;
	sc_y=(int)ceil((double)size_arena/(double)sc_x);
	if(output_mode>=OUTPUT_DEBUG)
	{
		sprintf(out_str,"sc_x=%d sc_y=%d COLS=%d ROWS=%d(max %d)\n",sc_x,sc_y,COLS,LINES,max_sc_y);
		fputs(out_str,fpout);
	}
	if(sc_y>(LINES-yd-yl))
	{
		deinit_txt();
		sprintf(out_str,"arena y (%d) excedes max video lines (%d)...switching to VO_NONE mode\n",sc_y,LINES);
		fputs(out_str,fpout);
		vo_mode=VO_NONE;
		return;
	}
	curs_y=0;
	for(count=0;count<size_arena;count++)
	{
		unpack(count,&mem);
		get_p_attr(mem.processID,&car,&col);
		addr2coords(count,&curs_x,&curs_y);
		mvaddch(curs_y+yd,curs_x+xd,car);
	}
	// make box
	mvaddch(0,0,'+');
	for(count=0;count<sc_x;count++) addch('=');
	addch('+');
	mvaddch(sc_y+1,0,'+');
	for(count=0;count<sc_x;count++) addch('=');
	addch('+');
	for(count=0;count<sc_y;count++)
	{
		mvaddch(count+yd,0,'|');
		mvaddch(count+yd,sc_x+1,'|');
	}
	refresh();
}
void deinit_txt()
{
	printw("leaving...");
	sprintf(out_str,"...and the winner is process #%d (%c)                            \n",primo_task->ID,primo_task->out_symbol);
	printw(out_str);
	getch();
	endwin();
}
void curses_print_arena_snap(void)
{

}
