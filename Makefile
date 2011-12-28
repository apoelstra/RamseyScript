
all: main.c check.c sequence.c recurse.c
	gcc -Wall -Wextra -ansi -pedantic -O3 main.c check.c sequence.c recurse.c -o ramsey

debug: main.c
	gcc -Wall -Wextra -ansi -pedantic -g main.c -o ramsey

