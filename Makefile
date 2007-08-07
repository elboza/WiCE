xcomp: main.o parse.o list_utils.o parse2.o pack.o init_game.o scheduler.o execute.o debug_output.o txt_output.o x11_output.o
	gcc -o wice main.o parse.o list_utils.o pack.o init_game.o scheduler.o execute.o parse2.o debug_output.o txt_output.o x11_output.o -ggdb -lm -lncurses  `pkg-config gtk+-2.0 --libs`

main.o : main.h main.c
	gcc -c main.c -ggdb `pkg-config gtk+-2.0 --cflags`
parse.o : main.h parse.h parse.c
	gcc -c parse.c -ggdb `pkg-config gtk+-2.0 --cflags`
parse2.o : main.h parse.h parse2.h parse2.c
	gcc -c parse2.c -ggdb `pkg-config gtk+-2.0 --cflags`
pack.o : main.h pack.h pack.c
	gcc -c pack.c -ggdb `pkg-config gtk+-2.0 --cflags`
init_game.o : main.h parse.h parse2.h init_game.h init_game.c
	gcc -c init_game.c -ggdb `pkg-config gtk+-2.0 --cflags`
scheduler.o : main.h parse.h parse2.h init_game.h scheduler.h scheduler.c
	gcc -c scheduler.c -ggdb `pkg-config gtk+-2.0 --cflags`
execute.o : main.h pack.h execute.h execute.c
	gcc -c execute.c -ggdb `pkg-config gtk+-2.0 --cflags`
debug_output.o : main.h debug_output.h debug_output.c txt_output.h x11_output.h
	gcc -c debug_output.c -ggdb `pkg-config gtk+-2.0 --cflags`
txt_output.o : main.h txt_output.h txt_output.c
	gcc -c txt_output.c -ggdb `pkg-config gtk+-2.0 --cflags`
x11_output.o : main.h x11_output.h x11_output.c
	gcc -c x11_output.c `pkg-config gtk+-2.0 --cflags` -ggdb
list_utils.o : main.h list_utils.h
	gcc -c list_utils.c -ggdb `pkg-config gtk+-2.0 --cflags`


clean:
	rm wice *.o


