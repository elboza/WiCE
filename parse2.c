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

unsigned int solve_vt(char *name,struct Process *proc)
{
	unsigned int val;
	struct var_table *vt;
	for(vt=proc->pc->vt_first;vt;vt=vt->next)
	{
		if((strcmp(name,vt->name))==0)
		{
			val=solve_expr(vt->val_first,proc);
			return val;
		}
	}
	return -1;
}
unsigned int solve_expr(struct expr_node *expr,struct Process *proc)
{
	int val,lval,rval;
	if(expr->type==NUMBER)
	{
		val=atoi(expr->str);
		return (val%size_arena);
	}
	if(expr->type==VAR)
	{
		val=solve_vt(expr->str,proc);
		return (val%size_arena);
	}
	if(expr->type){
	
	if(expr->left)
	{
		lval=solve_expr(expr->left,proc);
	}
	if(expr->right)
	{
		rval=solve_expr(expr->right,proc);
	}
	if((strcmp("+",expr->str))==0)
	{
		val=(lval+rval)%size_arena;
		return val;
	}
	if((strcmp("-",expr->str))==0)
	{
		val=(lval-rval)%size_arena;
		return val;
	}
	if((strcmp("*",expr->str))==0)
	{
		val=(lval*rval)%size_arena;
		return val;
	}
	if((strcmp("/",expr->str))==0)
	{
		val=(lval/rval)%size_arena;
		return val;
	}
	if((strcmp("%",expr->str))==0)
	{
		val=(lval%rval)%size_arena;
		return val;
	}
	if((strcmp("==",expr->str))==0)
	{
		val=lval==rval;
		return val;
	}
	if((strcmp("!=",expr->str))==0)
	{
		val=lval!=rval;
		return val;
	}
	if((strcmp("<",expr->str))==0)
	{
		val=lval<rval;
		return val;
	}
	if((strcmp(">",expr->str))==0)
	{
		val=lval>rval;
		return val;
	}
	if((strcmp("<=",expr->str))==0)
	{
		val=lval<=rval;
		return val;
	}
	if((strcmp(">=",expr->str))==0)
	{
		val=lval>=rval;
		return val;
	}
	
	}
}
void compute_asserts(struct Process *proc)
{
	struct var_table *vt;
	struct expr_node *expr;
	unsigned int ret_val;
	for(vt=proc->pc->vt_first;vt;vt=vt->next)
	{
		if((strcmp(vt->name,ASSERT_STR))==0)
		{
			ret_val=solve_expr(vt->val_first,proc);
			if(!ret_val)
			{
				die("assert condition failed");
			}
		}
	}
}
void generate_code(struct Process *proc)
{
	struct instruction_node *in;
	struct unpacked_op_mem op_mem;
	void *new_code;
	char s1[MAXSTR];
	int x;
	compute_asserts(proc);
	for(in=proc->pc->first;in;in=in->next)
	{
		op_mem.processID=proc->processID;
		op_mem.opcode=str_to_code(in->instr);
		op_mem.a_pref=str_to_code(in->laddr);
		op_mem.b_pref=str_to_code(in->raddr);
		if((strcmp(in->modifier,"NULL"))==0) {get_default_mod(&op_mem);} else {op_mem.mod=str_to_code(in->modifier);}
		if(in->left) {op_mem.a_val=solve_expr(in->left,proc);} else {op_mem.a_val=0;}
		if(in->right) {op_mem.b_val=solve_expr(in->right,proc);} else {op_mem.b_val=0;}

		//malloc void/struct
		if(arena_mem_type==MEM_TYPE_ONE) new_code=(struct array_mem_small*)malloc(sizeof(struct array_mem_small));
		if(arena_mem_type==MEM_TYPE_TWO) new_code=(struct array_mem_mid*)malloc(sizeof(struct array_mem_mid));
		if(arena_mem_type==MEM_TYPE_FOUR) new_code=(struct array_mem_norm*)malloc(sizeof(struct array_mem_norm));
		if(new_code==NULL)
		{sprintf(s1,"at line %d ",in->line_count);die("error malloking new_code struct");}
		//jump adjust
		if((op_mem.opcode==op_JMP)||(op_mem.opcode==op_JMZ)||(op_mem.opcode==op_JMN)||(op_mem.opcode==op_DJN))
		{
			op_mem.a_val=(op_mem.a_val-in->num_node)%size_arena;
		}
		//pack
		pack(&op_mem,new_code);
		//add code_node
		in->code=new_code;
	}
	//solve_org
	if((strcmp(proc->pc->org,""))!=0)
	{
		x=solve_vt(proc->pc->org,proc);
		sprintf(proc->pc->org,"%d",x);
	}
}
