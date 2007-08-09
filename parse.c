#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<ctype.h>
#include<gtk/gtk.h>
#include"main.h"
#include"list_utils.h"


char my_token[MAXSTR];

char* skip_space(char *p)
{
	while ((*p==' ')||(*p=='\t')) p++;
	return p;
}
char* get_word(char *p)
{
	int n=0;
	while(isalnum(*p))
	{
		my_token[n++]=*p;
		p++;
	}
	my_token[n]='\0';
	return p;
}
char* get_num(char *p)
{
	int n=0;
	if(*p=='-') {my_token[n++]=*p;p++;p=skip_space(p);}
	while(isdigit(*p))
	{
		my_token[n++]=*p;
		p++;
	}
	my_token[n]='\0';
	return p;
}
char* get_sym(char *p)
{
	int n=0;
	my_token[n]=*p;
	p++;n++;
	if(my_token[0]=='=')
	{
		if(*p=='=') {my_token[n]=*p;p++;n++;}
	}
	if(my_token[0]=='!')
	{
		if(*p=='=') {my_token[n]=*p;p++;n++;}
	}
	my_token[n]='\0';
	return p;
}
char* get_token(char *p)
{
	if(isalpha(*p)) {p=get_word(p);} else
	if((isdigit(*p))||((*p=='-')&&(isdigit(*(p+1))))) {p=get_num(p);}
	else {p=get_sym(p);}
	return p;
}

void take_assert(char *p)
{}
int take_instr1(char *token,char *p)
{}
void take_instr2(char *token,char *p)
{}
void take_equ(char *token,char *p)
{}
int is_instr(char *token,int line_count)
{
	static char *valid_instr[]={
	"mov","2","dat","2","nop","0","add","2","sub","2","mul","2","div","2","mod","2","jmp","1",
	"jmz","1","jmn","1","djn","1","spl","1","cmp","2","seq","2","sne","2","slt","2","ldp","2",
	"stp","2"
	,"NULL","0"
	};
	int n;
	char s1[MAXSTR];
	for(n=0;;n+=2)
	{
		if((strcasecmp(valid_instr[n],"NULL"))==0) break;
		if((strcasecmp(valid_instr[n],token))==0) return(atoi(valid_instr[n+1]));
	}
	//sprintf(s1,"invalid instruction at line %d .",line_count);
	//die(s1);
	return -1;
}
char* get_addr_mode(char *p)
{
	char ss[]="#$*@{}<>";
	int n;
	my_token[0]='$';my_token[1]='\0';
	for(n=0;n<=7;n++)
	if(*p==ss[n]){my_token[0]=*p;my_token[1]='\0';p++;break;}
	return p;
}
int get_processID()
{
	static int p=0;
	return (++p);
}
void is_modifier(char *token,int line_count)
{
	static char *valid_modifier[]={
	"a","b","ab","ba","f","x","i","NULL"
	};
	int n;
	char s1[MAXSTR];
	for(n=0;;n++)
	{
		if((strcasecmp(valid_modifier[n],"NULL"))==0) break;
		if((strcasecmp(valid_modifier[n],token))==0) return ;
	}
	sprintf(s1,"invalid modifier at line %d .",line_count);
	die(s1);
}
int is_op(int line_count)
{
	char s1[MAXSTR];
	if(my_token[0]=='+') return OP_ADD;
	if(my_token[0]=='-') return OP_ADD;
	if(my_token[0]=='*') return OP_MUL;
	if(my_token[0]=='/') return OP_MUL;
	if(my_token[0]=='%') return OP_MUL;
	sprintf(s1,"parse error at line %d. not a valid operation symbol",line_count);
	die(s1);
}
int is_bool_op(char *p,int line_cout)
{
	p=get_token(p);
	if((strcmp(my_token,"=="))==0) return 1;
	if((strcmp(my_token,"!="))==0) return 1;
	if((strcmp(my_token,"<="))==0) return 1;
	if((strcmp(my_token,">="))==0) return 1;
	if((strcmp(my_token,">"))==0) return 1;
	if((strcmp(my_token,"<"))==0) return 1;
	if((strcmp(my_token,"="))==0) return 1;
	
	return 0;
}
int is_item(int line_count)
{
	char s1[MAXSTR];
	if((isdigit(my_token[0]))||((my_token[0]=='-')&&(isdigit(my_token[1])))) return NUMBER;
	if(isalpha(my_token[0])) return VAR;
	sprintf(s1,"parse error at line %d. not a valid number or variable",line_count);
	die(s1);
}
char* get_arg(struct expr_node **expr,char *p,int line_count)
{
	int m_type;
	struct expr_node *new_expr,*new_expr2,*save_expr,*save_tree;
	char s1[MAXSTR];
	p=get_token(p);
	m_type=is_item(line_count);
	new_expr=(struct expr_node*)malloc(sizeof(struct expr_node));
	if(new_expr==NULL)
	{ sprintf(s1,"at line %d, error malloc exr_node",line_count);die(s1);}
	new_expr->type=m_type;
	strncpy(new_expr->str,my_token,MAXSTR);
	new_expr->left=NULL;
	new_expr->right=NULL;
	p=skip_space(p);
	if((*p=='\n')||(*p==',')|| (is_bool_op(p,line_count)))
	{
		*expr=new_expr;
		return p;
	}
	p=get_token(p);
	m_type=is_op(line_count);
	new_expr2=(struct expr_node*)malloc(sizeof(struct expr_node));
	if(new_expr2==NULL)
	{ sprintf(s1,"at line %d, error malloc exr_node",line_count);die(s1);}
	new_expr2->type=m_type;
	strncpy(new_expr2->str,my_token,MAXSTR);
	new_expr2->left=new_expr;
	new_expr2->right=NULL;
	*expr=new_expr2;
	p=skip_space(p);
	p=get_arg(&(new_expr2->right),p,line_count);
	if((new_expr2->type<new_expr2->right->type)&&(new_expr2->type<3)&&(new_expr2->right->type<3))
	{
		save_expr=new_expr2->right;
		save_tree=new_expr2->right->left;
		new_expr2->right->left=new_expr2;
		save_expr->left->right=save_tree;
		*expr=save_expr;
	}
	return p;
}
char* get_b_arg(struct expr_node **expr,char *p,int line_count)
{
	struct expr_node *new_expr,*new_expr2,*new_expr3;
	char s1[MAXSTR];
	p=get_arg(&new_expr,p,line_count);
	p=skip_space(p);
	if((*p=='\n')||(*p==','))
	{
		*expr=new_expr;
		return p;
	}
	//if is_bool_op(p)
	if(is_bool_op(p,line_count))
	{
		new_expr2=(struct expr_node*)malloc(sizeof(struct expr_node));
		if(new_expr2==NULL)
		{ sprintf(s1,"at line %d, error malloc exr_node",line_count);die(s1);}
		new_expr2->type=OP_BOOL;
		p=get_token(p);
		strncpy(new_expr2->str,my_token,MAXSTR);
		new_expr2->left=new_expr;
		new_expr2->right=NULL;
		p=skip_space(p);
		p=get_arg(&new_expr3,p,line_count);
		new_expr2->right=new_expr3;
		*expr=new_expr2;
	}
	return p;
}
void insert_label(char *label2,int label_val,int line_num,struct Process *proc)
{
	struct var_table *vt,*new_vt;
	struct expr_node *expr;
	char s1[MAXSTR],label[MAXSTR];
	sprintf(label,":%s",label2);
	for(vt=proc->pc->vt_first;vt;vt=vt->next)
	if((strcmp(vt->name,label))==0)
	{
		sprintf(s1,"sorry, the label at line %d already exists",line_num);
		die(s1);
	}
	new_vt=(struct var_table*)malloc(sizeof(struct var_table));
	if(new_vt==NULL) die("error allocating new_vt");
	expr=(struct expr_node*)malloc(sizeof(struct expr_node));
	if(expr==NULL) die("error allcating expr");
	strncpy(new_vt->name,label,MAXSTR);
	new_vt->val_first=expr;
	new_vt->val_last=expr;
	expr->left=NULL;
	expr->right=NULL;
	expr->type=NUMBER;
	sprintf(expr->str,"%d",label_val);
	add_vt(proc,new_vt);
}
void insert_in_vt(char *varname,struct expr_node *expr,int line_num,struct Process *proc)
{
	struct var_table *vt,*new_vt;
	//struct expr_node *expr;
	char s1[MAXSTR];
	if((strcmp(varname,ASSERT_STR))!=0)
	for(vt=proc->pc->vt_first;vt;vt=vt->next)
	if((strcmp(vt->name,varname))==0)
	{
		sprintf(s1,"sorry, the variable at line %d already exists",line_num);
		die(s1);
	}
	new_vt=(struct var_table*)malloc(sizeof(struct var_table));
	if(new_vt==NULL) die("error allocating new_vt");
	//expr=(struct expr_node*)malloc(sizeof(struct expr_node));
	//if(expr==NULL) die("error allcating expr");
	strncpy(new_vt->name,varname,MAXSTR);
	new_vt->val_first=expr;
	new_vt->val_last=expr;
	//expr->left=expr;
	//expr->right=NULL;
	//expr->type=NUMBER;
	//sprintf(expr->str,"%d",label_val);
	add_vt(proc,new_vt);
}
void add_environment_costant(char *env_name,int val,struct Process *proc)
{
	struct var_table *vt;
	struct expr_node *expr;
	vt=(struct var_table*)malloc(sizeof(struct var_table));
	if(vt==NULL) die("error allocating vt in add_environment_contants");
	expr=(struct expr_node*)malloc(sizeof(struct expr_node));
	if(expr==NULL) die("error on allocating exprin add_environment_contants");
	expr->left=NULL;
	expr->right=NULL;
	expr->type=NUMBER;
	sprintf(expr->str,"%d",val);
	strncpy(vt->name,env_name,MAXSTR);
	vt->val_first=expr;
	vt->val_last=expr;
	add_vt(proc,vt);
}
void add_environment_costants(struct Process *proc)
{
	add_environment_costant("CORESIZE",size_arena,proc);
	add_environment_costant("WARRIORS",warriors,proc);
	add_environment_costant("MAXPROCESSES",maxprocesses,proc);
	add_environment_costant("MAXCYCLES",CPU_cicle,proc);
	add_environment_costant("MAXLENGTH",max_prog_size,proc);
	add_environment_costant("MINDISTANCE",min_distance,proc);
	add_environment_costant("VERSION",version,proc);
}
void read_file(char *filename,struct Process **pproc)
{
	FILE *fp;
	char *endfile,line[MAXSTR],*p,save_token[MAXSTR];
	int label_bool=0,line_count=0,num_code=0,n_args;
	struct instruction_node *new_instr;
	struct Process *proc;
	struct expr_node *new_expr;
	proc=(struct Process*)malloc(sizeof(struct Process));
	if(proc==NULL) die("errore nell'allocare struct Process");
	proc->pt=NULL;
	proc->pc=NULL;
	proc->prev=NULL;
	proc->next=NULL;
	proc->processID=get_processID();
	fp=fopen(filename,"r");
	if(fp==NULL) die("error opening file");
	proc->pc=(struct process_construct*)malloc(sizeof(struct process_construct));
	if(proc->pc==NULL) die("errore nell'allocare process_construct");
	proc->pc->first=NULL;
	proc->pc->last=NULL;
	proc->pc->len=0;
	proc->pc->org[0]='\0';
	proc->pc->vt_first=NULL;
	proc->pc->vt_last=NULL;
	while((endfile=fgets(line,MAXSTR,fp))!=NULL)
	{
		line_count++;
		p=&line[0];
		if(*p==';') continue;
		p=skip_space(p);
		if(*p=='\n') continue;
		p=get_token(p);
		p=skip_space(p);
		if((strcmp(my_token,"org"))==0) 
		{
			p=get_word(p);strncpy(proc->pc->org,my_token,MAXSTR);
			if(*p!='\n') {sprintf(save_token,"parse error at line %d. not an end line after the org argument",line_count);die(save_token);}
			continue;
		}
		if((strcmp(my_token,"end"))==0) 
		{
			p=skip_space(p);
			if(*p!='\n') {sprintf(save_token,"parse error after 'end' at line %d.",line_count);die(save_token);}
			break;
		}
		if((strcmp(my_token,"assert"))==0) 
		{
			take_assert(p);
			new_expr=(struct expr_node*)malloc(sizeof(struct expr_node));
			if(new_expr==NULL)
			{printf("at line %d, ",line_count);die("error allocating new_expr");}
			p=skip_space(p);
			p=get_b_arg(&new_expr,p,line_count);
			insert_in_vt(ASSERT_STR,new_expr,line_count,proc);
			continue;
		}
		if(*p==':') 
		{
			insert_label(my_token,num_code,line_count,proc);p=skip_space(++p);
			if(*p=='\n') continue;
			p=get_token(p);
		}
		n_args=is_instr(my_token,line_count);
		if(n_args!=-1) 
		{
			new_instr=(struct instruction_node*)malloc(sizeof(struct instruction_node));
			if(new_instr==NULL) {printf("at line %d , ",line_count);die("error allocating new_instr");}
			strncpy(new_instr->instr,my_token,MAXSTR);
			new_instr->num_node=num_code++;
			new_instr->line_count=line_count;
			new_instr->prev=NULL;
			new_instr->next=NULL;
			new_instr->code=NULL;
			new_instr->left=NULL;
			new_instr->right=NULL;
			new_instr->laddr[0]='#';new_instr->laddr[1]='\0';
			new_instr->raddr[0]='#';new_instr->raddr[1]='\0';
			strcpy(new_instr->modifier,"NULL");
			if(*p=='.'){
				p=get_word(++p);is_modifier(my_token,line_count);
				strncpy(new_instr->modifier,my_token,MAXMOD);
			}
			p=skip_space(p);
			if(n_args>0)
			{
				p=get_addr_mode(p); //$ by default. the result is in my_token
				new_instr->laddr[0]=my_token[0];
				new_instr->laddr[1]='\0';
				//p=get_token(p);
				//new_instr->left=(struct expr_node*)malloc(sizeof(struct expr_node));
				//if(new_instr->left==NULL){
				//printf("at line %d , ",line_count);die("error alocating left expr");}
				p=get_arg(&new_instr->left,p,line_count);
				p=skip_space(p);
				if(n_args>1)
				{
					if(*p!=',')
					{printf("at line %d , ",line_count);die("a comma expected (,)");}
					p=skip_space(++p);
					p=get_addr_mode(p); //$ by default. the result is in my_token
					new_instr->raddr[0]=my_token[0];
					new_instr->raddr[1]='\0';
					//p=get_token(p);
					//new_instr->right=(struct expr_node*)malloc(sizeof(struct expr_node));
					//if(new_instr->right==NULL){
					//printf("at line %d , ",line_count);die("error allocating right expr");}
					p=get_arg(&new_instr->right,p,line_count);
					p=skip_space(p);
				}
			}
			if(*p!='\n')
			{printf("at line %d , ",line_count);die("not an ending line after command");}
			//add the node
			add_node(new_instr,proc);
			continue;
		}
		strncpy(save_token,my_token,MAXSTR);
		p=get_token(p);
		if((strcmp(my_token,"equ"))==0) 
		{
			new_expr=(struct expr_node*)malloc(sizeof(struct expr_node));
			if(new_expr==NULL)
			{printf("at line %d, ",line_count);die("error allocating new_expr");}
			p=skip_space(p);
			p=get_arg(&new_expr,p,line_count);
			insert_in_vt(save_token,new_expr,line_count,proc);
			continue;
		}
	}
	fclose(fp);
	proc->pc->len=num_code;
	add_proc(proc);
	*pproc=proc;
}
void visit_tree(struct expr_node *expr)
{
	if(expr)
	{
		sprintf(out_str,"%s",expr->str);fputs(out_str,fpout);
		if(expr->type>2) return;
		sprintf(out_str,"(");fputs(out_str,fpout);
		visit_tree(expr->left);
		sprintf(out_str,",");fputs(out_str,fpout);
		visit_tree(expr->right);
		sprintf(out_str,")");fputs(out_str,fpout);
		
	}
}
void print_data(struct Process *proc)
{
	struct process_construct *pc;
	struct instruction_node *in;
	struct expr_node *expr;
	struct var_table *vt;
	if(!DO_DEBUG) return;
	for(in=proc->pc->first;in;in=in->next)
	{
		sprintf(out_str,"{line(%d[%d]),%s",in->num_node,in->line_count,in->instr);fputs(out_str,fpout);
		if(in->left)
		{
			sprintf(out_str,"(%c ",*in->laddr);fputs(out_str,fpout);
			visit_tree(in->left);
			if(in->right)
			{
				sprintf(out_str,",%c ",*in->raddr);fputs(out_str,fpout);
				visit_tree(in->right);
			}
			sprintf(out_str,")");fputs(out_str,fpout);
		}
		sprintf(out_str,"};");fputs(out_str,fpout);
	}
	sprintf(out_str,"\n---var table---\n");fputs(out_str,fpout);
	for(vt=proc->pc->vt_first;vt;vt=vt->next)
	{
		sprintf(out_str,"%s %s",vt->name,vt->val_first->str);fputs(out_str,fpout);
		if(vt->val_first->left)
		{
			sprintf(out_str,"(");fputs(out_str,fpout);
			visit_tree(vt->val_first->left);
			if(vt->val_first->right)
			{
				sprintf(out_str,",");fputs(out_str,fpout);
				visit_tree(vt->val_first->right);
			}
			sprintf(out_str,")");fputs(out_str,fpout);
		}
		sprintf(out_str,"\n");fputs(out_str,fpout);
	}
}
void parse(char *filename)
{
	struct Process *proc;
	//proc=(struct Process*)malloc(sizeof(struct Process));
	//if(proc==NULL) die("errore nell'allocare struct Process");
	//proc->pt=NULL;
	//proc->pc=NULL;
	//proc->prev=NULL;
	//proc->next=NULL;
	proc=NULL;
	read_file(filename,&proc);
	add_environment_costants(proc);
	if(output_mode>=OUTPUT_DEBUG) print_data(proc);
	generate_code(proc);
}

