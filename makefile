#
# Make file for simple compiler2scanner and compiler2parser example
#

# flags and defs for built-in compiler rules
CFLAGS = -I. -Wall -g -Wno-unused-function
CC = gcc

# default rule, build the compiler2parser into a 'test' executable
all: test

symbtable.o: symtable.c symbtable.h
	gcc -c -g symtable.c

astree.o: astree.c astree.h
	gcc -c -g astree.c
	
# yacc "-d" flag creates y.tab.h header
y.tab.c: compiler6parser.y
	yacc -d compiler6parser.y

# lex rule includes y.tab.c to force yacc to run first
# lex "-d" flag turns on debugging output
lex.yy.c: compiler6scanner.l y.tab.c
	lex compiler6scanner.l

# test executable needs compiler2scanner and compiler2parser object files
test: lex.yy.o y.tab.o symtable.o astree.o
	gcc -o test y.tab.o lex.yy.o symtable.o astree.o

test2:
	make
	./test testC.c > test.s
	gcc -g test.s -o assemblyTest

# ltest is a standalone lexer (compiler2scanner)
# build this by doing "make ltest"
# -ll for compiling lexer as standalone
ltest: compiler6scanner.l
	lex compiler6scanner.l
	gcc -DLEXONLY lex.yy.c -o ltest -ll

# clean the directory for a pure rebuild (do "make clean")
clean:
	rm -f lex.yy.c a.out y.tab.c y.tab.h *.o test assemblyTest ltest test.s
