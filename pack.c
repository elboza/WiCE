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

int str_to_code(char *str)
{
	static char *conv_table[]={
	"nop","1","dat","0","mov","2","add","3","sub","4","mul","5","div","6","mod","7","jmp","8",
	"jmz","9","jmn","10","djn","11","spl","12","cmp","13","seq","14","sne","15","slt","16","ldp","17",
	"stp","18"
	,"#","0","$","2","*","3","@","4","{","5","<","6","}","7",">","1"
	,"a","0","b","1","ab","2","ba","3","f","4","x","5","i","6"
	,"NULL","-1"
	};
	int n,retval;
	for(n=0;;n+=2)
	{
		if((strcasecmp(conv_table[n],"NULL"))==0) break;
		if((strcasecmp(conv_table[n],str))==0)
		{
			retval=atoi(conv_table[n+1]);
			return retval;
		}
	}
	return -1;
	
}
void get_default_mod(struct unpacked_op_mem *op)
{
	switch(op->opcode)
	{
		case op_NOP:
		case op_DAT:
			op->mod=op_F;
			break;
		case op_MOV:
		case op_SEQ:
		case op_SNE:
		case op_CMP:
			op->mod=op_I;
			if(op->b_pref==op_IMM) op->mod=op_B;
			if(op->a_pref==op_IMM) op->mod=op_AB;
			break;
		case op_ADD:
		case op_SUB:
		case op_MUL:
		case op_DIV:
		case op_MOD:
			op->mod=op_F;
			if(op->b_pref==op_IMM) op->mod=op_B;
			if(op->a_pref==op_IMM) op->mod=op_AB;
			break;
		case op_SLT:
		case op_LDP:
		case op_STP:
			op->mod=op_B;
			if(op->a_pref==op_IMM) op->mod=op_AB;
			break;
		case op_JMP:
		case op_JMZ:
		case op_DJN:
		case op_SPL:
			op->mod=op_B;
			break;
		default:
			break;
	}
}
void pack(struct unpacked_op_mem *in,void *out)
{
	struct array_mem_small *msmall;
	struct array_mem_mid *mmid;
	struct array_mem_norm *mlarge;
	
	if(arena_mem_type==MEM_TYPE_ONE)
	{
		msmall=(struct arena_mem_small*)out;
		msmall->mem=(in->processID<<28)+(in->opcode<<23)+(in->mod<<19)+(in->a_pref<<16)+(in->b_pref<<13)+(in->a_val<<7)+(in->b_val<<1);
	}
	if(arena_mem_type==MEM_TYPE_TWO)
	{
		mmid=(struct arena_mem_mid*)out;
		mmid->processID_opcode=(in->processID<<16)+(in->opcode<<12)+(in->mod<<8)+(in->a_pref<<5)+(in->b_pref<<2);
		mmid->arg1_arg2=(in->a_val<<16)+(in->b_val);
	}
	if(arena_mem_type==MEM_TYPE_FOUR)
	{
		mlarge=(struct arena_mem_norm*)out;
		mlarge->processID=in->processID;
		mlarge->opcode=(in->mod<<28)+(in->a_pref<<25)+(in->b_pref<<22)+in->opcode;
		mlarge->arg1=in->a_val;
		mlarge->arg2=in->b_val;
	}
	
}
void unpack(int ip,struct unpacked_op_mem *out)
{
	struct array_mem_small *msmall;
	struct array_mem_mid *mmid;
	struct array_mem_norm *mlarge;
	
	if(arena_mem_type==MEM_TYPE_ONE)
	{
		msmall=(struct mem_type_small*)arena;
		out->b_val=(msmall[ip].mem>>1)&63;
		out->a_val=(msmall[ip].mem>>7)&63;
		out->b_pref=(msmall[ip].mem>>13)&7;
		out->a_pref=(msmall[ip].mem>>16)&7;
		out->mod=(msmall[ip].mem>>19)&15;
		out->opcode=(msmall[ip].mem>>23)&31;
		out->processID=(msmall[ip].mem>>28)&15;
	}
	if(arena_mem_type==MEM_TYPE_TWO)
	{
		mmid=(struct mem_type_mid*)arena;
		out->b_val=(mmid[ip].arg1_arg2)&65535;
		out->a_val=(mmid[ip].arg1_arg2>>16)&65535;
		out->b_pref=(mmid[ip].processID_opcode>>2)&7;
		out->a_pref=(mmid[ip].processID_opcode>>5)&7;
		out->mod=(mmid[ip].processID_opcode>>8)&15;
		out->opcode=(mmid[ip].processID_opcode>>12)&31;
		out->processID=(mmid[ip].processID_opcode>>16)&65535;
	}
	if(arena_mem_type==MEM_TYPE_FOUR)
	{
		mlarge=(struct mem_type_norm*)arena;
		out->a_val=mlarge[ip].arg1;
		out->b_val=mlarge[ip].arg2;
		out->processID=mlarge[ip].processID;
		out->opcode=(mlarge[ip].opcode)&31;
		out->b_pref=(mlarge[ip].opcode>>22)&7;
		out->a_pref=(mlarge[ip].opcode>>25)&7;
		out->mod=(mlarge[ip].opcode>>28)&15;
	}
}
void pack2mem(int ip,struct unpacked_op_mem *in)
{
	struct array_mem_small *msmall;
	struct array_mem_mid *mmid;
	struct array_mem_norm *mlarge;
	
	if(arena_mem_type==MEM_TYPE_ONE)
	{
		msmall=(struct arena_mem_small*)arena;
		msmall[ip].mem=(in->processID<<28)+(in->opcode<<23)+(in->mod<<19)+(in->a_pref<<16)+(in->b_pref<<13)+(in->a_val<<7)+(in->b_val<<1);
	}
	if(arena_mem_type==MEM_TYPE_TWO)
	{
		mmid=(struct arena_mem_mid*)arena;
		mmid[ip].processID_opcode=(in->processID<<16)+(in->opcode<<12)+(in->mod<<8)+(in->a_pref<<5)+(in->b_pref<<2);
		mmid[ip].arg1_arg2=(in->a_val<<16)+(in->b_val);
	}
	if(arena_mem_type==MEM_TYPE_FOUR)
	{
		mlarge=(struct arena_mem_norm*)arena;
		mlarge[ip].processID=in->processID;
		mlarge[ip].opcode=(in->mod<<28)+(in->a_pref<<25)+(in->b_pref<<22)+in->opcode;
		mlarge[ip].arg1=in->a_val;
		mlarge[ip].arg2=in->b_val;
	}
	
}
