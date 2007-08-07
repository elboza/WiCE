#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<ctype.h>
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

extern void gtk_display_curr_instr(char *ss);

void get_str(struct unpacked_op_mem *code,char *m_op,char *m_apref,char *m_bpref,char *m_mod)
{
	static char *op_table[]={
	"nop","1","dat","0","mov","2","add","3","sub","4","mul","5","div","6","mod","7","jmp","8",
	"jmz","9","jmn","10","djn","11","spl","12","cmp","13","seq","14","sne","15","slt","16","ldp","17",
	"stp","18","NULL","-1"
	};
	static char *pref_table[]={
	"#","0","$","2","*","3","@","4","{","5","<","6","}","7",">","1","NULL","-1"
	};
	static char *mod_table[]={
	"a","0","b","1","ab","2","ba","3","f","4","x","5","i","6","NULL","-1"
	};
	
	int n,rv;
	for(n=1;;n+=2)
	{
		rv=atoi(op_table[n]);
		if(rv==code->opcode) {strcpy(m_op,op_table[n-1]);break;}
	}
	for(n=1;;n+=2)
	{
		rv=atoi(pref_table[n]);
		if(rv==code->a_pref) {strcpy(m_apref,pref_table[n-1]);break;}
	}
	for(n=1;;n+=2)
	{
		rv=atoi(pref_table[n]);
		if(rv==code->b_pref) {strcpy(m_bpref,pref_table[n-1]);break;}
	}
	for(n=1;;n+=2)
	{
		rv=atoi(mod_table[n]);
		if(rv==code->mod) {strcpy(m_mod,mod_table[n-1]);break;}
	}
}
void print_ex_data(struct unpacked_op_mem *code,struct unpacked_op_mem *IRA,struct unpacked_op_mem *IRB,int RPA,int RPB,struct process_thread *pt)
{
	char m_op[8],m_apref[5],m_bpref[5],m_mod[5];
	int m_aval=0,m_bval=0;
	if(output_mode>=OUTPUT_DEBUG)
	{
		get_str(code,&m_op[0],&m_apref[0],&m_bpref[0],&m_mod[0]);
		sprintf(out_str,"code:{proc=%d| %s %s %d,%s %d |IP=%d}                                                                ",code->processID,m_op,m_apref,code->a_val,m_bpref,code->b_val,pt->IP);
		if(vo_mode==VO_NONE && log_mode)	fputs(out_str,fpout);
		if(vo_mode==VO_FRAMEBUFFER) mvaddstr(sc_y+1+yd,0,out_str);
	}
	if(vo_mode==VO_FRAMEBUFFER)
	{
		get_str(code,&m_op[0],&m_apref[0],&m_bpref[0],&m_mod[0]);
		sprintf(out_str,"code:{proc=%d| %s %s %d,%s %d |IP=%d}                                                                ",code->processID,m_op,m_apref,code->a_val,m_bpref,code->b_val,pt->IP);
		mvaddstr(sc_y+1+yd,0,out_str);
	}
	if(vo_mode==VO_X11)
	{
		get_str(code,&m_op[0],&m_apref[0],&m_bpref[0],&m_mod[0]);
		sprintf(out_str,"code:{proc=%d| %s %s %d,%s %d |IP=%d}                                                                ",code->processID,m_op,m_apref,code->a_val,m_bpref,code->b_val,pt->IP);
		gtk_display_curr_instr(out_str);
	}
}
void cell_refresh(int addr,struct process_thread *pt)
{
	if(vo_mode==VO_FRAMEBUFFER) curses_cell_refresh(addr,pt);
	if(vo_mode==VO_X11) x11_cell_refresh(addr,pt);
}
void print_debug2()
{
	int count,x_pos,y_pos,col;
	char car;
	struct unpacked_op_mem mem;
	for(count=0;count<size_arena;count++)
	{
		for(x_pos=0;x_pos<max_x;x_pos++)
		{
			if(++count>size_arena) break;
			unpack(count,&mem);
			get_p_attr(mem.processID,&car,&col);
			sprintf(out_str,"%c",car);
			fputs(out_str,fpout);
		}
		sprintf(out_str,"\n");
		fputs(out_str,fpout);
	}
	sprintf(out_str,"\n");
	fputs(out_str,fpout);
}
