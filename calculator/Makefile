all: main

main: lex.yy.c y.tab.c exprtree.c
	gcc y.tab.c lex.yy.c exprtree.c -o calculator.out

lex.yy.c: calculator.l y.tab.h 
	lex calculator.l

y.tab.c: calculator.y express.h
	yacc -d calculator.y

y.tab.h : calculator.y
	yacc -d calculator.y

clean:	
	rm -rf calculator.out lex.yy.c y.tab.c y.tab.h 

run: all
	./calculator.out
