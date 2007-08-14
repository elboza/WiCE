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


void get_I_field(struct unpacked_op_mem *IR,int addr_mode,int val, int IP,int ID,int *RRPA)
{
	int RPA,PIP;
	struct unpacked_op_mem TMP;
	if(addr_mode==op_IMM) {RPA=IP;*RRPA=RPA;unpack(RPA,IR);}
	else
	{
		RPA=(IP+val)%size_arena;
		if(addr_mode==op_B_IND_PREDEC)
		{
			unpack(RPA,&TMP);
			(--TMP.b_val)%size_arena;
			TMP.processID=ID;
			pack2mem(RPA,&TMP);
			RPA=(RPA+TMP.b_val)%size_arena;
		}
		if(addr_mode==op_A_IND_PREDEC)
		{
			unpack(RPA,&TMP);
			(--TMP.a_val)%size_arena;
			TMP.processID=ID;
			pack2mem(RPA,&TMP);
			RPA=(RPA+TMP.a_val)%size_arena;
		}
		if(addr_mode==op_B_IND)
		{
			unpack(RPA,&TMP);
			RPA=(RPA+TMP.b_val)%size_arena;
		}
		if(addr_mode==op_A_IND)
		{
			unpack(RPA,&TMP);
			RPA=(RPA+TMP.a_val)%size_arena;
		}
		if(addr_mode==op_B_IND_POSTINC)
		{
			unpack(RPA,&TMP);
			PIP=RPA;
			RPA=(RPA+TMP.b_val)%size_arena;
		}
		if(addr_mode==op_A_IND_POSTINC)
		{
			unpack(RPA,&TMP);
			PIP=RPA;
			RPA=(RPA+TMP.a_val)%size_arena;
		}
		unpack(RPA,IR);
		*RRPA=RPA;
		if(addr_mode==op_B_IND_POSTINC)
		{
			TMP.processID=ID;
			(++TMP.b_val)%size_arena;
			pack2mem(PIP,&TMP);
		}
		if(addr_mode==op_A_IND_POSTINC)
		{
			TMP.processID=ID;
			(++TMP.a_val)%size_arena;
			pack2mem(PIP,&TMP);
		}
	}
	
}
int execute (struct unpacked_op_mem *code,struct process_thread *pt)
{
	int alive,RPA,RPB;
	struct unpacked_op_mem IRA,IRB;
	struct process_task *ptask;
	struct process_thread *new_thread;
	alive=ALIVE;
	code->processID=pt->ptask->ID;
	get_I_field(&IRA,code->a_pref,code->a_val,pt->IP,code->processID,&RPA);
	get_I_field(&IRB,code->b_pref,code->b_val,pt->IP,code->processID,&RPB);
	print_ex_data(code,&IRA,&IRB,RPA,RPB,pt);
	if(vo_mode>=VO_FRAMEBUFFER)
	{
		cell_refresh(pt->IP,pt);
	}
	if(output_mode==OUTPUT_DEBUG2 && (vo_mode==VO_NONE || b_log))
	{
		print_debug2();
	}
	switch(code->opcode)
	{
	case op_NOP:
		pt->IP=(pt->IP+1)%size_arena;
		break;
	case op_DAT:
		alive=DEAD;
		break;
	case op_MOV:
		switch(code->mod)
		{
		case op_A:
			IRB.a_val=IRA.a_val;
			IRB.processID=pt->ptask->ID;
			pack2mem(RPB,&IRB);
			break;
		case op_B:
			IRB.b_val=IRA.b_val;
			IRB.processID=pt->ptask->ID;
			pack2mem(RPB,&IRB);
			break;
		case op_AB:
			IRB.b_val=IRA.a_val;
			IRB.processID=pt->ptask->ID;
			pack2mem(RPB,&IRB);
			break;
		case op_BA:
			IRB.a_val=IRA.b_val;
			IRB.processID=pt->ptask->ID;
			pack2mem(RPB,&IRB);
			break;
		case op_F:
			IRB.a_val=IRA.a_val;
			IRB.b_val=IRA.b_val;
			IRB.processID=pt->ptask->ID;
			pack2mem(RPB,&IRB);
			break;
		case op_X:
			IRB.a_val=IRA.b_val;
			IRB.b_val=IRA.a_val;
			IRB.processID=pt->ptask->ID;
			pack2mem(RPB,&IRB);
			break;
		case op_I:
			IRA.processID=pt->ptask->ID;
			pack2mem(RPB,&IRA);
			break;
		default:
			break;
		}
		pt->IP=(pt->IP+1)%size_arena;
		if(vo_mode>=VO_FRAMEBUFFER)
		{
			cell_refresh(RPB,pt);
		}
		break;
	case op_ADD:
		IRB.processID=pt->ptask->ID;
		switch(code->mod)
		{
		case op_A:
			IRB.a_val=(IRA.a_val+IRB.a_val)%size_arena;
			break;
		case op_B:
			IRB.b_val=(IRA.b_val+IRB.b_val)%size_arena;
			break;
		case op_AB:
			IRB.b_val=(IRA.a_val+IRB.b_val)%size_arena;
			break;
		case op_BA:
			IRB.a_val=(IRA.b_val+IRB.a_val)%size_arena;
			break;
		case op_I:
		case op_F:
			IRB.a_val=(IRA.a_val+IRB.a_val)%size_arena;
			IRB.b_val=(IRA.b_val+IRB.b_val)%size_arena;
			break;
		case op_X:
			IRB.a_val=(IRA.b_val+IRB.b_val)%size_arena;
			IRB.b_val=(IRA.a_val+IRB.a_val)%size_arena;
			break;
		default:
			break;
		}
		pack2mem(RPB,&IRB);
		pt->IP=(pt->IP+1)%size_arena;
		if(vo_mode>=VO_FRAMEBUFFER)
		{
			cell_refresh(RPB,pt);
		}
		break;
	case op_SUB:
		IRB.processID=pt->ptask->ID;
		switch(code->mod)
		{
		case op_A:
			IRB.a_val=(IRB.a_val-IRA.a_val)%size_arena; //or maybe IRA-IRB ?!?!?to check
			break;
		case op_B:
			IRB.b_val=(IRB.b_val-IRA.b_val)%size_arena;
			break;
		case op_AB:
			IRB.b_val=(IRB.b_val-IRA.a_val)%size_arena;
			break;
		case op_BA:
			IRB.a_val=(IRB.a_val-IRA.b_val)%size_arena;
			break;
		case op_I:
		case op_F:
			IRB.a_val=(IRB.a_val-IRA.a_val)%size_arena;
			IRB.b_val=(IRB.b_val-IRA.b_val)%size_arena;
			break;
		case op_X:
			IRB.a_val=(IRB.b_val-IRA.b_val)%size_arena;
			IRB.b_val=(IRB.a_val-IRA.a_val)&size_arena;
			break;
		default:
			break;
		}
		pack2mem(RPB,&IRB);
		pt->IP=(pt->IP+1)%size_arena;
		if(vo_mode>=VO_FRAMEBUFFER)
		{
			cell_refresh(RPB,pt);
		}
		break;
	case op_MUL:
		IRB.processID=pt->ptask->ID;
		switch(code->mod)
		{
		case op_A:
			IRB.a_val=(IRA.a_val*IRB.a_val)%size_arena;
			break;
		case op_B:
			IRB.b_val=(IRA.b_val*IRB.b_val)%size_arena;
			break;
		case op_AB:
			IRB.b_val=(IRA.a_val*IRB.b_val)%size_arena;
			break;
		case op_BA:
			IRB.a_val=(IRA.b_val*IRB.a_val)%size_arena;
			break;
		case op_I:
		case op_F:
			IRB.a_val=(IRA.a_val*IRB.a_val)%size_arena;
			IRB.b_val=(IRA.b_val*IRB.b_val)%size_arena;
			break;
		case op_X:
			IRB.a_val=(IRA.b_val*IRB.b_val)%size_arena;
			IRB.b_val=(IRA.a_val*IRB.a_val)%size_arena;
			break;
		default:
			break;
		}
		pack2mem(RPB,&IRB);
		pt->IP=(pt->IP+1)%size_arena;
		if(vo_mode>=VO_FRAMEBUFFER)
		{
			cell_refresh(RPB,pt);
		}
		break;
	case op_DIV:
		IRB.processID=pt->ptask->ID;
		switch(code->mod)
		{
		case op_A:
			if(IRA.a_val==0) {alive=DEAD;} else
			{IRB.a_val=(IRB.a_val/IRA.a_val)%size_arena;}
			break;
		case op_B:
			if(IRA.b_val==0) {alive=DEAD;} else
			{IRB.b_val=(IRB.b_val/IRA.b_val)%size_arena;}
			break;
		case op_AB:
			if(IRA.a_val==0) {alive=DEAD;} else
			{IRB.b_val=(IRB.b_val/IRA.a_val)%size_arena;}
			break;
		case op_BA:
			if(IRA.b_val==0) {alive=DEAD;} else
			{IRB.a_val=(IRB.a_val/IRA.b_val)%size_arena;}
			break;
		case op_F:
		case op_I:
			if((IRA.a_val==0)||(IRA.b_val==0)) {alive=DEAD;} else
			{
				IRB.a_val=(IRB.a_val/IRA.a_val)%size_arena;
				IRB.b_val=(IRB.b_val/IRA.b_val)%size_arena;
			}
			break;
		case op_X:
			if((IRA.a_val==0)||(IRA.b_val==0)) {alive=DEAD;} else
			{
				IRB.a_val=(IRB.b_val/IRA.b_val)%size_arena;
				IRB.b_val=(IRB.a_val/IRA.a_val)%size_arena;
			}
			break;
		default:
			break;
		}
		pack2mem(RPB,&IRB);
		pt->IP=(pt->IP+1)%size_arena;
		if(vo_mode>=VO_FRAMEBUFFER)
		{
			cell_refresh(RPB,pt);
		}
		break;
	case op_MOD:
		IRB.processID=pt->ptask->ID;
		switch(code->mod)
		{
		case op_A:
			if(IRA.a_val==0) {alive=DEAD;} else
			{IRB.a_val=(IRB.a_val%IRA.a_val)%size_arena;}
			break;
		case op_B:
			if(IRA.b_val==0) {alive=DEAD;} else
			{IRB.b_val=(IRB.b_val%IRA.b_val)%size_arena;}
			break;
		case op_AB:
			if(IRA.a_val==0) {alive=DEAD;} else
			{IRB.b_val=(IRB.b_val%IRA.a_val)%size_arena;}
			break;
		case op_BA:
			if(IRA.b_val==0) {alive=DEAD;} else
			{IRB.a_val=(IRB.a_val%IRA.b_val)%size_arena;}
			break;
		case op_F:
		case op_I:
			if((IRA.a_val==0)||(IRA.b_val==0)) {alive=DEAD;} else
			{
				IRB.a_val=(IRB.a_val%IRA.a_val)%size_arena;
				IRB.b_val=(IRB.b_val%IRA.b_val)%size_arena;
			}
			break;
		case op_X:
			if((IRA.a_val==0)||(IRA.b_val==0)) {alive=DEAD;} else
			{
				IRB.a_val=(IRB.b_val%IRA.b_val)%size_arena;
				IRB.b_val=(IRB.a_val%IRA.a_val)%size_arena;
			}
			break;
		default:
			break;
		}
		pack2mem(RPB,&IRB);
		pt->IP=(pt->IP+1)%size_arena;
		if(vo_mode>=VO_FRAMEBUFFER)
		{
			cell_refresh(RPB,pt);
		}
		break;
	case op_JMP:
		pt->IP=RPA;
		break;
	case op_JMZ:
		switch(code->mod)
		{
		case op_A:
		case op_BA:
			if(IRB.a_val==0) {pt->IP=RPA;} else {pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_B:
		case op_AB:
			if(IRB.b_val==0) {pt->IP=RPA;} else {pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_F:
		case op_X:
		case op_I:
			if((IRB.a_val==0)&&(IRB.b_val==0))
			{ pt->IP=RPA;} else {pt->IP=(pt->IP+1)%size_arena;}
			break;
		default:
			break;
		}
		break;
	case op_JMN:
		switch(code->mod)
		{
		case op_A:
		case op_BA:
			if(IRB.a_val!=0) {pt->IP=RPA;} else {pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_B:
		case op_AB:
			if(IRB.b_val!=0) {pt->IP=RPA;} else {pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_F:
		case op_X:
		case op_I:
			if((IRB.a_val!=0)&&(IRB.b_val!=0))
			{ pt->IP=RPA;} else {(pt->IP++)%size_arena;}
			break;
		default:
			break;
		}
		break;
	case op_DJN:
		IRB.processID=pt->ptask->ID;
		switch(code->mod)
		{
		case op_A:
		case op_BA:
			(IRB.a_val--)%size_arena;
			if(IRB.a_val!=0) {pt->IP=RPA;}else{pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_B:
		case op_AB:
			(IRB.b_val--)%size_arena;
			if(IRB.b_val!=0) {pt->IP=RPA;}else{pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_F:
		case op_I:
		case op_X:
			(IRB.a_val--)%size_arena;
			(IRB.b_val--)%size_arena;
			if((IRB.a_val!=0)||(IRB.b_val!=0))
			{pt->IP=RPA;}else{(pt->IP++)%size_arena;}
			break;
		default:
			break;
		}
		pack2mem(RPB,&IRB);
		break;
	case op_CMP:
		switch(code->mod)
		{
		case op_A:
			if(IRA.a_val==IRB.a_val)
			{pt->IP=(pt->IP+2)%size_arena;}
			else{pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_B:
			if(IRA.a_val==IRB.a_val)
			{pt->IP=(pt->IP+2)%size_arena;}
			else{pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_AB:
			if(IRA.a_val==IRB.b_val)
			{pt->IP=(pt->IP+2)%size_arena;}
			else{pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_BA:
			if(IRA.b_val==IRB.a_val)
			{pt->IP=(pt->IP+2)%size_arena;}
			else{pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_F:
			if((IRA.a_val==IRB.a_val)&&(IRA.b_val==IRB.b_val))
			{pt->IP=(pt->IP+2)%size_arena;}
			else{pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_X:
			if((IRA.a_val==IRB.b_val)&&(IRA.b_val==IRB.a_val))
			{pt->IP=(pt->IP+2)%size_arena;}
			else{pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_I:
			if((IRA.opcode==IRB.opcode)&&
			(IRA.mod==IRB.mod)&&
			(IRA.a_pref==IRB.b_pref)&&
			(IRA.a_val==IRB.a_val)&&
			(IRA.b_pref==IRB.b_pref)&&
			(IRA.b_val==IRB.b_val))
			{pt->IP=(pt->IP+2)%size_arena;}
			else{pt->IP=(pt->IP+1)%size_arena;}
			break;
		default:
			break;
		}
		break;
	case op_SLT:
		switch(code->mod)
		{
		case op_A:
			if(IRA.a_val<IRB.a_val)
			{pt->IP=(pt->IP+2)%size_arena;}
			else {pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_B:
			if(IRA.b_val<IRB.b_val)
			{pt->IP=(pt->IP+2)%size_arena;}
			else {pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_AB:
			if(IRA.a_val<IRB.b_val)
			{pt->IP=(pt->IP+2)%size_arena;}
			else {pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_BA:
			if(IRA.b_val<IRB.a_val)
			{pt->IP=(pt->IP+2)%size_arena;}
			else {pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_F:
		case op_I:
			if((IRA.a_val<IRB.a_val)&&(IRA.b_val<IRB.b_val))
			{pt->IP=(pt->IP+2)%size_arena;}
			else {pt->IP=(pt->IP+1)%size_arena;}
			break;
		case op_X:
			if((IRA.a_val<IRB.b_val)&&(IRA.b_val<IRB.a_val))
			{pt->IP=(pt->IP+2)%size_arena;}
			else {pt->IP=(pt->IP+1)%size_arena;}
			break;
		default:
			break;
		}
		break;
	case op_SPL:
		//create new thread
		ptask=pt->ptask;
		if(ptask->n_threads<maxprocesses)
		{
			new_thread=(struct process_thread*)malloc(sizeof(struct process_thread));
			if(new_thread==NULL) die("error creating the new thread");
			new_thread->ptask=ptask;
			new_thread->prev=NULL;
			new_thread->next=NULL;
			new_thread->communication_in_a=0;
			new_thread->communication_out_a=0;
			new_thread->communication_in_b=0;
			new_thread->communication_out_b=0;
			new_thread->IP=RPA;
			// and resume to next instruction
			pt->IP=(pt->IP+1)%size_arena;
			//set new process to go last in process task's queue (repeat the father first)
			add_thread_rev(new_thread,ptask);
			ptask->cur_thread=ptask->cur_thread->prev;
			if(ptask->cur_thread==NULL) ptask->cur_thread=ptask->ultimo_thread;
		}
		break;
	case op_CTOUT:
		switch(code->mod)
		{
		case op_A:
			pt->communication_out_a=IRA.a_val;
			pt->communication_out_b=IRB.a_val;
			break;
		case op_B:
			pt->communication_out_a=IRA.b_val;
			pt->communication_out_b=IRB.b_val;
			break;
		case op_AB:
		case op_F:
		case op_I:
			pt->communication_out_a=IRA.a_val;
			pt->communication_out_b=IRB.b_val;
			break;
		case op_BA:
		case op_X:
			pt->communication_out_a=IRA.b_val;
			pt->communication_out_b=IRB.a_val;
			break;
		
		default:
			break;
		}
		ctout(pt);
		pt->IP=(pt->IP+1)%size_arena;
		break;
	case op_CTIN:
		switch(code->mod)
		{
		case op_A:
			IRA.a_val=pt->communication_out_a;
			IRB.a_val=pt->communication_out_b;
			break;
		case op_B:
			IRA.b_val=pt->communication_out_a;
			IRB.b_val=pt->communication_out_b;
			break;
		case op_AB:
		case op_F:
		case op_I:
			IRA.a_val=pt->communication_out_a;
			IRB.b_val=pt->communication_out_b;
			break;
		case op_BA:
		case op_X:
			IRA.b_val=pt->communication_out_a;
			IRB.a_val=pt->communication_out_b;
			break;
		
		default:
			break;
		}
		pt->IP=(pt->IP+1)%size_arena;
		if(vo_mode>=VO_FRAMEBUFFER)
		{
			cell_refresh(RPB,pt);
			cell_refresh(RPA,pt);
		}
		break;
	case op_CPOUT:
		switch(code->mod)
		{
		case op_A:
			pt->ptask->communication_out_a=IRA.a_val;
			pt->ptask->communication_out_b=IRB.a_val;
			break;
		case op_B:
			pt->ptask->communication_out_a=IRA.b_val;
			pt->ptask->communication_out_b=IRB.b_val;
			break;
		case op_AB:
		case op_F:
		case op_I:
			pt->ptask->communication_out_a=IRA.a_val;
			pt->ptask->communication_out_b=IRB.b_val;
			break;
		case op_BA:
		case op_X:
			pt->ptask->communication_out_a=IRA.b_val;
			pt->ptask->communication_out_b=IRB.a_val;
			break;
		
		default:
			break;
		}
		cpout(pt->ptask);
		pt->IP=(pt->IP+1)%size_arena;
		break;
	case op_CPIN:
		switch(code->mod)
		{
		case op_A:
			IRA.a_val=pt->ptask->communication_out_a;
			IRB.a_val=pt->ptask->communication_out_b;
			break;
		case op_B:
			IRA.b_val=pt->ptask->communication_out_a;
			IRB.b_val=pt->ptask->communication_out_b;
			break;
		case op_AB:
		case op_F:
		case op_I:
			IRA.a_val=pt->ptask->communication_out_a;
			IRB.b_val=pt->ptask->communication_out_b;
			break;
		case op_BA:
		case op_X:
			IRA.b_val=pt->ptask->communication_out_a;
			IRB.a_val=pt->ptask->communication_out_b;
			break;
		
		default:
			break;
		}
		pt->IP=(pt->IP+1)%size_arena;
		if(vo_mode>=VO_FRAMEBUFFER)
		{
			cell_refresh(RPB,pt);
			cell_refresh(RPA,pt);
		}
		break;
	default:
		break;
	}
	if(output_mode>=OUTPUT_DEBUG)
	{
		sprintf(out_str,"=>newIP=%d\n",pt->IP);
		if(vo_mode==VO_NONE && log_mode) fputs(out_str,fpout);
		//if(vo_mode==VO_FRAMEBUFFER);
	}
	return alive;
}
