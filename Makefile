CC	= gcc
CFLAGS	= -Wall -O0

EQUATIONS	= exec_equation_parser
SQL_PARSER	= exec_sql_parser
MATH_PARSER	= exec_math_parser

all: lex.yy.o MexprEnums.o $(MATH_PARSER)

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

$(MATH_PARSER): MexprEnums.o MathExpression.o
	gcc -ll -g lex.yy.o $^ -o $@

.phony: clean test

clean:
	rm -rf *.o $(EQUATIONS) $(SQL_PARSER) $(MATH_PARSER) lex.yy.c

test: lex.yy.o $(MATH_PARSER)
	@./$(MATH_PARSER) &> /dev/null && echo "Success if the return value is zero >>> $$?"
