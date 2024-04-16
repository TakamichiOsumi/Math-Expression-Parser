CC	= gcc
CFLAGS	= -Wall -O0

SUBDIR_stack	= Stack
SUBDIRS	= $(SUBDIR_stack)

LIB_stack	= -L $(CURDIR)/$(SUBDIR_stack)
LIBS	= -ll -lstack

MATH_PARSER	= exec_math_parser

SYSTEM_COMPONENTS	= MexprEnums.c MathExpression.c PostfixConverter.c
OBJ_SYSTEM_COMPONENTS	= MexprEnums.o MathExpression.o PostfixConverter.o

all: libraries lex.yy.o $(MATH_PARSER)

libraries:
	for dir in $(SUBDIRS); do make -C $$dir; done

lex.yy.o:
	lex Parser.l
	$(CC) -c lex.yy.c -o lex.yy.o

$(OBJ_SYSTEM_COMPONENTS): libraries
	for src in $(SYSTEM_COMPONENTS); do $(CC) $(CFLAGS) $$src -c; done

$(MATH_PARSER): $(OBJ_SYSTEM_COMPONENTS)
	$(CC) $(LIB_stack) $(LIBS) -g lex.yy.o $^ -o $@

.phony: clean test

clean:
	rm -rf *.o lex.yy.c
	for dir in $(SUBDIRS); do cd $$dir; make clean; cd ..; done

test: lex.yy.o $(MATH_PARSER)
	@./$(MATH_PARSER) &> /dev/null && echo "Success if the return value is zero >>> $$?"
