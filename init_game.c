#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<errno.h>
#include<ctype.h>
#include<math.h>
#include<gtk/gtk.h>
#include"main.h"
#include"list_utils.h"
#include"parse2.h"
#include"pack.h"
#include"init_game.h"
#include"txt_output.h"
#include"x11_output.h"

extern GdkColormap *execute_colormap;

void putcode(int pos,int processID,void *code)
{
	struct array_mem_small *msmall,*small_arena;
	struct array_mem_mid *mmid,*mid_arena;
	struct array_mem_norm *mlarge,*large_arena;
	if(arena_mem_type==MEM_TYPE_ONE)
	{
		msmall=(struct array_mem_small*)code;
		small_arena=(struct array_mem_small*)arena;
		small_arena[pos].mem=msmall->mem;
	}
	if(arena_mem_type==MEM_TYPE_TWO)
	{
		mmid=(struct array_mem_mid*)code;
		mid_arena=(struct array_mem_mid*)arena;
		mid_arena[pos].processID_opcode=mmid->processID_opcode;
		mid_arena[pos].arg1_arg2=mmid->arg1_arg2;
	}
	if(arena_mem_type==MEM_TYPE_FOUR)
	{
		mlarge=(struct array_mem_norm*)code;
		large_arena=(struct array_mem_norm*)arena;
		large_arena[pos].processID=mlarge->processID;
		large_arena[pos].opcode=mlarge->opcode;
		large_arena[pos].arg1=mlarge->arg1;
		large_arena[pos].arg2=mlarge->arg2;
	}
}
void get_symbols(char *out_symbol,int *out_color)
{
	static color=1;
	static letter='A';
	
	*out_symbol=letter++;
	*out_color=color++;
}
void free_expr(struct expr_node *expr)
{
	if(expr->left) free_expr(expr->left);
	if(expr->right) free_expr(expr->right);
	free(expr);
}
void init_game()
{
	struct array_mem_small *msmall;
	struct array_mem_mid *mmid;
	struct array_mem_norm *mlarge;
	struct Process *proc;
	struct instruction_node *in;
	struct process_thread *pthread;
	struct process_task *ptask;
	int next_offs,proc_offs,pos;
	double xx,lato_x,lato_y;
	if(arena_mem_type==MEM_TYPE_ONE)
	{
		msmall=(struct array_mem_small*)malloc(sizeof(struct array_mem_small)*size_arena);
		if(msmall==NULL) die("error malloking small array mem");
		bzero(msmall,sizeof(struct array_mem_small)*size_arena);
		arena=msmall;
	}
	if(arena_mem_type==MEM_TYPE_TWO)
	{
		mmid=(struct array_mem_mid*)malloc(sizeof(struct array_mem_mid)*size_arena);
		if(mmid==NULL) die("error malloking mid array mem");
		bzero(mmid,sizeof(struct array_mem_mid)*size_arena);
		arena=mmid;
	}
	if(arena_mem_type==MEM_TYPE_FOUR)
	{
		mlarge=(struct array_mem_norm*)malloc(sizeof(struct array_mem_norm)*size_arena);
		if(mlarge==NULL) die("error malloking large array mem");
		bzero(mlarge,sizeof(struct array_mem_norm)*size_arena);
		arena=mlarge;
	}
	next_offs=0;
	for(proc=proc_primo;proc;proc=proc->next)
	{
		//calc offset in mem
		proc_offs=(next_offs+(rand()%min_distance))%size_arena;
		//put in mem
		pos=proc_offs;
		for(in=proc->pc->first;in;in=in->next)
		{
			putcode(pos++,proc->processID,in->code);
		}
		//create pt
		pthread=(struct process_thread*)malloc(sizeof(struct process_thread));
		if(pthread==NULL) die("error alloking new thread");
		pthread->IP=proc_offs+(atoi(proc->pc->org));
		pthread->communication_in=0;
		pthread->communication_out=0;
		pthread->prev=NULL;
		pthread->next=NULL;
		pthread->ptask=NULL;
		ptask=(struct process_task*)malloc(sizeof(struct process_task));
		if(ptask==NULL) die("error alloking new task");
		ptask->ID=proc->processID;
		ptask->n_threads=1;
		ptask->prev=NULL;
		ptask->next=NULL;
		ptask->primo_thread=NULL;
		ptask->ultimo_thread=NULL;
		ptask->cur_thread=pthread;
		ptask->communication_in=0;
		ptask->communication_out=0;
		get_symbols(&ptask->out_symbol,&ptask->out_color);
		//add pt
		add_thread(pthread,ptask);
		add_task(ptask);
		//recalc next_offs
		next_offs+=proc_offs+proc->pc->len;
		//free proc&pc
		in=proc->pc->first;
		do{
			if(in->left) free_expr(in->left);
			if(in->right) free_expr(in->right);
			if(in->code) free(in->code);
			if(in->next) {in=in->next;free(in->prev);}
			else {free(in);in=NULL;}
		}while(in!=NULL);
	}
	xx=sqrt(size_arena);
	lato_y=rint(xx);
	lato_x=ceil(xx);
	max_x=(int)lato_x;
	max_y=(int)lato_y;
	if(output_mode>=OUTPUT_DEBUG)
	{
		sprintf(out_str,"max_x=%d max_y=%d\n",max_x,max_y);
		fputs(out_str,fpout);
	}
	//init_graph
	if(vo_mode==VO_FRAMEBUFFER) init_txt();
	if(vo_mode==VO_X11) init_x11();
}

void deinit_game()
{
	if(vo_mode==VO_FRAMEBUFFER) deinit_txt();
	if(vo_mode==VO_X11) deinit_x11();
	if(g_actual_CPU>=CPU_cicle)
	{
		sprintf(out_str,"The match is a Draw !!\n");
	}
	else
	{
		sprintf(out_str,"...and the winner is process #%d        ,at %d CPU cicles                       \n",primo_task->ID,g_actual_CPU);
		if(output_mode>=OUTPUT_DEBUG2) sprintf(out_str,"...and the winner is process #%d (%c)                       , at %d CPU cicles     \n",primo_task->ID,primo_task->out_symbol,g_actual_CPU);
	}
	fputs(out_str,fpout);
}
