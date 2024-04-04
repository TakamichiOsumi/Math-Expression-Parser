CC	= gcc
CFLAGS	= -Wall -O0

all: parser MexprEnums.o executable

MexprEnums.o:
	$(CC) $(CFLAGS) -c MexprEnums.c -o $@

parser:
	lex Parser.l
	$(CC) -c lex.yy.c -o lex.yy.o

executable: MexprEnums.o
	gcc -ll -g lex.yy.o $^ -o $@

.phony: clean

clean:
	rm -rf *.o executable lex.yy.c
