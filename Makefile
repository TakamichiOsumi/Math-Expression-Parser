CC	= gcc
CFLAGS	= -Wall -O0

SUBDIR_stack	= Stack
SUBDIRS	= $(SUBDIR_stack)

LIB_stack	= -L $(CURDIR)/$(SUBDIR_stack)
LIBS	= -ll -lstack

EQUATIONS	= exec_equation_parser
SQL_PARSER	= exec_sql_parser
MATH_PARSER	= exec_math_parser

all: libraries lex.yy.o MexprEnums.o $(MATH_PARSER)

libraries:
	for dir in $(SUBDIRS); do make -C $$dir; done

lex.yy.o:
	lex Parser.l
	$(CC) -c lex.yy.c -o lex.yy.o

MexprEnums.o:
	$(CC) $(CFLAGS) -c MexprEnums.c -o $@

Equations.o:
	$(CC) $(CFLAGS) -c Equations.c -o $@

SqlSelectParser.o:
	$(CC) $(CFLAGS) -c SqlSelectParser.c -o $@

MathExpression.o:
	$(CC) $(CFLAGS) -c MathExpression.c -o $@

PostfixConverter.o: libraries
	$(CC) $(CFLAGS) -c PostfixConverter.c -o $@

$(MATH_PARSER): MexprEnums.o MathExpression.o PostfixConverter.o
	$(CC) $(LIB_stack) $(LIBS) -g lex.yy.o $^ -o $@

.phony: clean test

clean:
	rm -rf *.o $(EQUATIONS) $(SQL_PARSER) $(MATH_PARSER) lex.yy.c
	for dir in $(SUBDIRS); do cd $$dir; make clean; cd ..; done

test: lex.yy.o $(MATH_PARSER)
	@./$(MATH_PARSER) &> /dev/null && echo "Success if the return value is zero >>> $$?"
