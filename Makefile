CC	= gcc
CFLAGS	= -Wall -O0 -g

SUBDIR_STACK	= Stack
SUBDIR_LIST	= Linked-List
SUBDIRS	= $(SUBDIR_STACK) $(SUBDIR_LIST)

LIB_STACK	= -L $(CURDIR)/$(SUBDIR_STACK)
LIB_LIST	= -L $(CURDIR)/$(SUBDIR_LIST)
LIBS	= -ll -lstack -llinked_list

OUTPUT_LIB	= libmexpr.a
TEST_APP	= exec_application

SYSTEM_COMPONENTS	= MexprEnums.c MathExpression.c PostfixConverter.c MexprTree.c
OBJ_SYSTEM_COMPONENTS	= MexprEnums.o MathExpression.o PostfixConverter.o MexprTree.o

all: libraries lex.yy.o $(OUTPUT_LIB) $(TEST_APP)

libraries:
	for dir in $(SUBDIRS); do make -C $$dir; done

lex.yy.o:
	lex Parser.l
	$(CC) -c lex.yy.c -o lex.yy.o

$(OBJ_SYSTEM_COMPONENTS): libraries
	for src in $(SYSTEM_COMPONENTS); do $(CC) $(CFLAGS) $$src -c; done

$(OUTPUT_LIB): $(OBJ_SYSTEM_COMPONENTS)
	ar rcs $(OUTPUT_LIB) lex.yy.o $^

$(TEST_APP): $(OUTPUT_LIB)
	$(CC)  -L . $(LIB_STACK) $(LIB_LIST) $(LIBS) -lmexpr application.c -o $(TEST_APP)

.phony: clean test

clean:
	rm -rf *.o lex.yy.c $(OUTPUT_LIB) $(TEST_APP)
	for dir in $(SUBDIRS); do cd $$dir; make clean; cd ..; done

test: lex.yy.o $(TEST_APP)
	@./$(TEST_APP) &> /dev/null && echo "Success when the return value is zero >>> $$?"
