
SOURCES=main.c check.c sequence.c recurse.c coloring.c ramsey.c

all: $(SOURCES)
	gcc -Wall -Wextra -ansi -pedantic -O3 $(SOURCES) -o ramsey

debug: $(SOURCES)
	gcc -Wall -Wextra -ansi -pedantic -g $(SOURCES) -o ramsey

