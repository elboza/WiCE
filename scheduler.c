#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<ctype.h>
#include<gtk/gtk.h>
#include"main.h"
#include"list_utils.h"
#include"parse2.h"
#include"pack.h"
#include"init_game.h"
#include"scheduler.h"


struct process_task* get_first()
{
	return primo_task;
}
int step(struct process_thread *pt)
{
	struct unpacked_op_mem *code;
	int alive,old_ip;
	code=(struct unpacked_op_mem*)malloc(sizeof(struct unpacked_op_mem));
	if(code==NULL) die("error alloking getting opcode from mem_array");
	unpack(pt->IP,code);
	old_ip=pt->IP;
	alive=execute(code,pt);
	if(alive)
	{
		code->processID=pt->ptask->ID;
		pack2mem(old_ip,code);
	}
	return alive;
}
void play_game()
{
	struct process_task *ptask,*task_to_kill;
	int actual_CPU,alive;
	ptask=get_first();
	//init_graph()
	actual_CPU=0;
	while(actual_CPU++<CPU_cicle)
	{
		//control if there is a winner
		if(primo_task==ultimo_task) break;
		if(sleeptime) sleep(sleeptime);
		alive=step(ptask->cur_thread);
		if(alive)
		{
			ptask->cur_thread=ptask->cur_thread->next;
			if(ptask->cur_thread==NULL) ptask->cur_thread=ptask->primo_thread;
		}
		else
		{
			del_thread(ptask->cur_thread);
			if(output_mode>=OUTPUT_DEBUG)
			{
				sprintf(out_str,"proc #%d: thread killed.                              \n",ptask->ID);
				if(vo_mode==VO_NONE && log_mode) fputs(out_str,fpout);
				if(vo_mode==VO_FRAMEBUFFER) mvaddstr(sc_y+2+yd,0,out_str);
			}
		}
		task_to_kill=ptask;
		ptask=ptask->next; if(ptask==NULL) ptask=primo_task;
		if(task_to_kill->cur_thread==NULL)
		{
			if(output_mode>=OUTPUT_DEBUG)
			{
				sprintf(out_str,"process killed.                                               \n");
				if(vo_mode==VO_NONE && log_mode) fputs(out_str,fpout);
				if(vo_mode==VO_FRAMEBUFFER) mvaddstr(sc_y+2+yd,0,out_str);
			}
			if(vo_mode==VO_FRAMEBUFFER)
			{
				mvaddch(task_to_kill->ID,txt_y_warrior_list,'X');
				sprintf(out_str,"process killed.                                               \n");
				mvaddstr(sc_y+2+yd,0,out_str);
			}
			del_task(task_to_kill);
		}
	}
	g_actual_CPU=actual_CPU;
}
