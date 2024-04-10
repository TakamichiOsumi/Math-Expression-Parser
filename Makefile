CC	= gcc
CFLAGS	= -Wall -O0

EQUATIONS	= exec_equation_parser
SQL_PARSER	= exec_sql_parser
MATH_PARSER	= exec_math_parser

# all: parser MexprEnums.o $(EQUATIONS)
# all: parser MexprEnums.o $(SQL_PARSER)
all: parser MexprEnums.o $(MATH_PARSER)

parser:
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

# $(EQUATIONS): MexprEnums.o Equations.o
#	gcc -ll -g lex.yy.o $^ -o $@

# $(SQL_PARSER): MexprEnums.o SqlSelectParser.o
#	gcc -ll -g lex.yy.o $^ -o $@

$(MATH_PARSER): MexprEnums.o MathExpression.o
	gcc -ll -g lex.yy.o $^ -o $@

.phony: clean

clean:
	rm -rf *.o $(EQUATIONS) $(SQL_PARSER) $(MATH_PARSER) lex.yy.c
