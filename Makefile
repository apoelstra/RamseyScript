
all: main.c check.c sequence.c recurse.c coloring.c
	gcc -Wall -Wextra -ansi -pedantic -O3 main.c check.c sequence.c recurse.c coloring.c -o ramsey

debug: main.c check.c sequence.c recurse.c coloring.c
	gcc -Wall -Wextra -ansi -pedantic -g main.c check.c sequence.c recurse.c coloring.c -o ramsey

