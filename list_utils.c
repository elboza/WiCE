#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<gtk/gtk.h>
#include"main.h"

void die(char *s)
{
	//char s[MAXCHAR];
	sprintf(out_str,"%s\n",s);
	fputs(out_str,fpout);
	exit(1);
}
void add_vt(struct Process *proc,struct var_table *vt)
{
	if(proc->pc->vt_first==NULL)
	{
		proc->pc->vt_first=vt;
		proc->pc->vt_last=vt;
		return;
	}
	vt->prev=proc->pc->vt_last;
	proc->pc->vt_last->next=vt;
	proc->pc->vt_last=vt;
}
void add_proc(struct Process *proc)
{
	if(proc_primo==NULL)
	{
		proc_primo=proc;
		proc_ultimo=proc;
	}
	else
	{
		proc->prev=proc_ultimo;
		proc_ultimo->next=proc;
		proc_ultimo=proc;
	}
}
void add_node(struct instruction_node *node,struct Process *proc)
{
	if(proc->pc->first==NULL)
	{
		proc->pc->first=node;
		proc->pc->last=node;
		return;
	}
	proc->pc->last->next=node;
	node->prev=proc->pc->last;
	proc->pc->last=node;
}
void add_thread(struct process_thread *thread,struct process_task *task)
{
	thread->ptask=task;
	if(task->primo_thread==NULL)
	{
		task->primo_thread=thread;
		task->ultimo_thread=thread;
		thread->prev=NULL;
		thread->next=NULL;
		return;
	}
	task->ultimo_thread->next=thread;
	thread->prev=task->ultimo_thread;
	thread->next=NULL;
	task->ultimo_thread=thread;
}
void add_thread_rev(struct process_thread *thread,struct process_task *task)
{
	thread->ptask=task;
	if(task->primo_thread==NULL)
	{
		task->primo_thread=thread;
		task->ultimo_thread=thread;
		thread->prev=NULL;
		thread->next=NULL;
		return;
	}
	task->primo_thread->prev=thread;
	thread->prev=NULL;
	thread->next=task->primo_thread;
	task->primo_thread=thread;
}
void add_task(struct process_task *task)
{
	if(primo_task==NULL)
	{
		primo_task=task;
		ultimo_task=task;
		task->prev=NULL;
		task->next=NULL;
		return;
	}
	ultimo_task->next=task;
	task->prev=ultimo_task;
	task->next=NULL;
	ultimo_task=task;
}
void del_task(struct process_task *task)
{
	struct process_thread *pt,*opt;
	if(primo_task==ultimo_task)
	{
		primo_task=NULL;
		ultimo_task=NULL;
	}
	else
	{
		if(task->prev) {task->prev->next=task->next;}else{primo_task=task->next;primo_task->prev=NULL;}
		if(task->next) {task->next->prev=task->prev;}else{ultimo_task=task->prev;ultimo_task->next=NULL;}
	}
	pt=task->primo_thread;
	//delete all related threads
	while(pt){
		opt=pt;
		pt=pt->next;
		free(opt);
	};
	free(task);
}
void del_thread(struct process_thread *pt)
{
	pt->ptask->n_threads--;
	if(pt==pt->ptask->cur_thread)
	{
		pt->ptask->cur_thread=pt->next;
		if(pt->next==NULL) pt->ptask->cur_thread=pt->ptask->primo_thread;
	}
	if(pt->ptask->primo_thread==pt->ptask->ultimo_thread)
	{
		pt->ptask->primo_thread=NULL;
		pt->ptask->ultimo_thread=NULL;
		pt->ptask->cur_thread=NULL;
	}
	else
	{
		if(pt->prev) {pt->prev->next=pt->next;}else{pt->ptask->primo_thread=pt->next;pt->ptask->primo_thread->prev=NULL;}
		if(pt->next) {pt->next->prev=pt->prev;}else{pt->ptask->ultimo_thread=pt->prev;pt->ptask->ultimo_thread->next=NULL;}
	}
	free(pt);
}
void add_killed_task(struct process_task *task)
{
	if(first_killed_task==NULL)
	{
		first_killed_task=task;
		last_killed_task=task;
		task->prev=NULL;
		task->next=NULL;
		return;
	}
	last_killed_task->next=task;
	task->prev=last_killed_task;
	task->next=NULL;
	last_killed_task=task;
}
void addr2coords(int addr,int *x,int *y)
{
	*x=addr % sc_x;
	*y=(int)(((double)addr)/((double)sc_x));
}
void get_p_attr(int ID,char *car,int *col)
{
	struct process_task *proc;
	*car=63;
	*col=-1;
	if(ID==0) {*car='.';*col=0;return;}
	for(proc=primo_task;proc;proc=proc->next)
	{
		if(proc->ID==ID)
		{
			*car=proc->out_symbol;
			*col=proc->out_color;
			return;
		}
	}
}
