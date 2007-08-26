#define OUTPUT_QUIET 1
#define OUTPUT_NORMAL 2
#define OUTPUT_DEBUG 3
#define OUTPUT_DEBUG2 4
#define OUTPUT_DEBUG3 5
#define VO_NONE 1
#define VO_FRAMEBUFFER 2
#define VO_X11 3

#define ARENA_SIZE 8000

#define COMM_NULL 0
#define COMM_PROC 1
#define COMM_THREAD 2

#define MAX_CPU_CICLE 10000
#define STAT_WAIT_TIME 1000

#define MEM_TYPE_ONE 1
#define MEM_TYPE_TWO 2
#define MEM_TYPE_FOUR 4

#define MAXSTR 255
#define MAXMOD 6
#define MAX_PROG_SIZE 200
#define MAXPROCESSES 1000
#define MINDISTANCE 200
#define VERSION 01
#define PSPACESIZE 500

#define VERBOSE 1
#define DO_DEBUG 1

#define OP_NULL 0
#define OP_MUL 1
#define OP_ADD 2
#define NUMBER 3
#define VAR 4
#define OP_BOOL 5
#define ASSERT_STR "ASSERT"
#define ALIVE 1
#define DEAD 0

struct array_mem_small{
	unsigned int mem;
};
struct array_mem_mid{
	unsigned int processID_opcode;
	unsigned int arg1_arg2;
};
struct array_mem_norm{
	unsigned int processID;
	unsigned int opcode;
	unsigned int arg1;
	unsigned int arg2;
};
struct expr_node{
	int type;
	char str[MAXSTR];
	struct expr_node *left,*right;
};
struct instruction_node{
	char instr [MAXSTR];
	char modifier[6];
	char laddr[4];
	char raddr[4];
	int num_node;
	int line_count;
	struct expr_node *left,*right;
	void *code;
	struct instruction_node *prev,*next;
};
struct process_task{
	unsigned int ID;
	unsigned int n_threads;
	struct process_thread *cur_thread,*primo_thread,*ultimo_thread;
	int communication_in_a,communication_in_b,communication_out_a,communication_out_b;
	char out_symbol;
	int out_color;
	GdkColor m_color;
	struct process_task *prev,*next;
};
struct process_thread{
	unsigned int IP;
	struct process_task *ptask;
	int communication_in_a,communication_in_b,communication_out_a,communication_out_b;
	struct process_thread *prev,*next;
};
struct var_table{
	char name[MAXSTR];
	struct expr_node *val_first,*val_last;
	struct var_table *prev,*next;
};
struct process_construct{
	struct instruction_node *first,*last;
	int len;
	char org[MAXSTR];
	struct var_table *vt_first,*vt_last;
};
struct Process{
	int processID;
	struct process_task *pt;
	struct process_construct *pc;
	struct Process *prev,*next;
};
struct unpacked_op_mem{
	unsigned int processID;
	int opcode;
	int mod;
	int a_pref;
	int b_pref;
	int a_val;
	int b_val;
};
void *arena;
int size_arena,warriors,version,min_distance,maxprocesses,max_prog_size;
int arena_mem_type;
int output_mode,vo_mode,log_mode,b_log,max_x,max_y,sc_x,sc_y;
int xd,yd,xl,yl,max_sc_y,sleeptime,b_multi_win,gtkwaittime,current_make_node,txt_y_warrior_list,g_actual_CPU;
int gtk_sc_x,gtk_sc_y;
int communication;
int CPU_cicle;
struct process_task *primo_task,*ultimo_task,*first_killed_task,*last_killed_task;
struct Process *proc_primo,*proc_ultimo;
char logfile[MAXSTR],out_str[MAXSTR];
FILE *fpout;
