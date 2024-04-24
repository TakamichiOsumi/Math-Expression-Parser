CC	= gcc
CFLAGS	= -Wall -O0 -g

SUBDIR_STACK	= Stack
SUBDIR_LIST	= Linked-List
SUBDIRS	= $(SUBDIR_STACK) $(SUBDIR_LIST)

LIB_STACK	= -L $(CURDIR)/$(SUBDIR_STACK)
LIB_LIST	= -L $(CURDIR)/$(SUBDIR_LIST)
LIBS	= -ll -lstack -llinked_list

MATH_PARSER	= exec_math_parser

SYSTEM_COMPONENTS	= MexprEnums.c MathExpression.c PostfixConverter.c MexprTree.c
OBJ_SYSTEM_COMPONENTS	= MexprEnums.o MathExpression.o PostfixConverter.o MexprTree.o

all: libraries lex.yy.o $(MATH_PARSER)

libraries:
	for dir in $(SUBDIRS); do make -C $$dir; done

lex.yy.o:
	lex Parser.l
	$(CC) -c lex.yy.c -o lex.yy.o

$(OBJ_SYSTEM_COMPONENTS): libraries
	for src in $(SYSTEM_COMPONENTS); do $(CC) $(CFLAGS) $$src -c; done

$(MATH_PARSER): $(OBJ_SYSTEM_COMPONENTS)
	$(CC) $(CFLAGS) $(LIB_STACK) $(LIB_LIST) $(LIBS) -g lex.yy.o $^ -o $@

.phony: clean test

clean:
	rm -rf *.o lex.yy.c $(MATH_PARSER)
	for dir in $(SUBDIRS); do cd $$dir; make clean; cd ..; done

test: lex.yy.o $(MATH_PARSER)
	@./$(MATH_PARSER) &> /dev/null && echo "Success when the return value is zero >>> $$?"
