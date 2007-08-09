#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<getopt.h>
#include<string.h>
#include<errno.h>
#include<gtk/gtk.h>
#include"main.h"

void usage()
{
	printf("WiCE v0.1 by xnando\n");
	printf("\nUSAGE:\n");
	printf("wice [options] file1 file2 file3 ...\n");
	printf("\nvalid options:\n\n");
	printf("\t--file(-f) file\t\talternative log file\n");
	printf("\t--output(-o) output\t\toutput mode (quiet,normal,debug,debug2)\n");
	printf("\t--size(-m) size\t\tsize of the array\n");
	printf("\t--comm(-c) comm\t\tcommunication type(null,process,thread)\n");
	printf("\t--vo(-v) mode\t\tmodes are (none,txt,x11)\n");
	printf("\t--log(-l)\t\turn log on (off default)\n");
	printf("\t--sleeptime(-z)\t\tpause between step commands (in seconds)\n");
	printf("\t--gtkwaittime(-w)\t\tpause between step commands (in milliseconds)\n");
	printf("\t--multiwin(-e)\t\ttoggle multi or single window(s) (default is multi windows)\n");
	exit(1);
}
void get_defaults_arg()
{
	size_arena=ARENA_SIZE;
	CPU_cicle=MAX_CPU_CICLE;
	arena_mem_type=MEM_TYPE_FOUR;
	output_mode=OUTPUT_NORMAL;
	communication=COMM_NULL;
	warriors=0;
	version=VERSION;
	min_distance=MINDISTANCE;
	maxprocesses=MAXPROCESSES;
	max_prog_size=MAX_PROG_SIZE;
	strcpy(logfile,"wice.log");
	log_mode=0;
	vo_mode=VO_NONE;
	fpout=stdout;
	b_log=0;
	sleeptime=0;
	b_multi_win=1;
	gtkwaittime=50;
	gtk_sc_x=200;
	gtk_sc_y=200;
}
void parse_args(int argc,char **argv)
{
	int c;
	while (1)
	{
		static struct option long_options[] =
		{
			{"output",required_argument,0,'o'},
			{"memtype",required_argument,0,'m'},
			{"cpu",required_argument,0,'c'},
			{"comm",required_argument,0,'z'},
			{"file",required_argument,0,'f'},
			{"size",required_argument,0,'s'},
			{"vo",required_argument,0,'v'},
			{"sleeptime",required_argument,0,'z'},
			{"gtkwaittime",required_argument,0,'w'}
		};
		int option_index = 0;
		c = getopt_long (argc, argv, "hlev:o:m:c:z:f:k:w:",long_options, &option_index);
		if (c == -1) break;
		switch(c)
		{
			case 'o':
				if((strcmp(optarg,"quiet"))==0) output_mode=OUTPUT_QUIET;
				if((strcmp(optarg,"normal"))==0) output_mode=OUTPUT_NORMAL;
				if((strcmp(optarg,"debug"))==0) output_mode=OUTPUT_DEBUG;
				if((strcmp(optarg,"debug2"))==0) output_mode=OUTPUT_DEBUG2;
				break;
			case 'm':
				if((strcmp(optarg,"tiny"))==0) arena_mem_type=MEM_TYPE_ONE;
				if((strcmp(optarg,"medium"))==0) arena_mem_type=MEM_TYPE_TWO;
				if((strcmp(optarg,"large"))==0) arena_mem_type=MEM_TYPE_FOUR;
				break;
			case 'c':
				CPU_cicle=strtol(optarg,NULL,10);
				if(errno==EINVAL) usage();
				break;
			case 'k':
				if((strcmp(optarg,"null"))==0) communication=COMM_NULL;
				if((strcmp(optarg,"process"))==0) communication=COMM_PROC;
				if((strcmp(optarg,"thread"))==0) communication=COMM_THREAD;
				break;
			case 'f':
				strncpy(logfile,optarg,MAXSTR);
				break;
			case 's':
				size_arena=strtol(optarg,NULL,10);
				if(errno==EINVAL) usage();
				break;
			case 'z':
				sleeptime=strtol(optarg,NULL,10);
				if(errno==EINVAL) usage();
				break;
			case 'w':
				gtkwaittime=strtol(optarg,NULL,10);
				if(errno==EINVAL) usage();
				break;
			case 'v':
				if((strcmp(optarg,"none"))==0) vo_mode=VO_NONE;
				if((strcmp(optarg,"txt"))==0) vo_mode=VO_FRAMEBUFFER;
				if((strcmp(optarg,"x11"))==0) vo_mode=VO_X11;
				break;
			case 'l':
				b_log=1;log_mode=1;
				break;
			case 'e':
				b_multi_win=0;
				break;
			case 'h':
			case '?':
				usage();
				break;
			default:
				usage();
				break;
		}
		
	}
}
void look_data_ok()
{
	if(arena_mem_type==MEM_TYPE_ONE)
	{
		if(size_arena>64) die("in tiny model the size of the arena must be<64 or try a bigger model");
		if(max_prog_size>64) die("in tiny model the size of the warrior must be<64 or try a bigger model");
		if(maxprocesses>16) die("in tiny model maximum nember of processes must be<16 or try a bigger model");
		
	}
	if(arena_mem_type==MEM_TYPE_TWO)
	{
		if(size_arena>65535) die("in tiny model the size of the arena must be<65535 or try a bigger model");
		if(max_prog_size>65535) die("in tiny model the size of the warrior must be<65535 or try a bigger model");
		if(maxprocesses>65535) die("in tiny model maximum nember of processes must be<65535 or try a bigger model");
	}
	if(arena_mem_type==MEM_TYPE_FOUR)
	{
	
	}
}
int main(int argc,char **argv)
{
	proc_primo=NULL;
	proc_ultimo=NULL;
	primo_task=NULL;
	ultimo_task=NULL;
	get_defaults_arg();
	parse_args(argc,argv);
	if(b_log)
	{
		fpout=fopen(logfile,"w");
		if(fpout==NULL)
		{
			printf("error opening log file\n");
			exit(1);
		}
	}
	look_data_ok();
	if(output_mode>=OUTPUT_DEBUG)
	{
		sprintf(out_str,"outputmode=%d\n",output_mode);
		fputs(out_str,fpout);
		sprintf(out_str,"memsize=%d\n",size_arena);
		fputs(out_str,fpout);
		sprintf(out_str,"b_log=%d\n",b_log);
		fputs(out_str,fpout);
		sprintf(out_str,"vo=%d\n",vo_mode);
		fputs(out_str,fpout);
		sprintf(out_str,"sleeptime=%d\n",sleeptime);
		fputs(out_str,fpout);
	}
	//read_files (and parse it in 2 passes
	warriors=argc-optind;
	if(optind<argc)
	{
		while(optind<argc)
		{
			if(output_mode>=OUTPUT_NORMAL)
			{
				sprintf(out_str,"parsing %s ... ",argv[optind]);
				fputs(out_str,fpout);
			}
			parse(argv[optind++]);
			if(output_mode>=OUTPUT_NORMAL)
			{
				sprintf(out_str,"ok\n");
				fputs(out_str,fpout);
			}
		}
	}
	else
	{
		usage();
	}
	//init_game
	init_game();
	//play_game
	play_game();
	//result
	
	deinit_game();
	if(b_log) fclose(fpout);
	return 0;
}
