CC	= gcc
CFLAGS	= -Wall -O0

all: parser MexprEnums.o executable

Equations.o:
	$(CC) $(CFLAGS) -c $(LIBS) Equations.c -o $@

MexprEnums.o:
	$(CC) $(CFLAGS) -c MexprEnums.c -o $@

parser:
	lex Parser.l
	$(CC) -c lex.yy.c -o lex.yy.o

executable: MexprEnums.o Equations.o
	gcc -ll -g lex.yy.o $^ -o $@

.phony: clean

clean:
	rm -rf *.o executable lex.yy.c
