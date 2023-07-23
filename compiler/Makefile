all: bin main 

main: bin/y.tab.c bin/lex.yy.c  src/express.c
	gcc bin/y.tab.c bin/lex.yy.c src/express.c
	mv a.out bin/
	
bin: 
	mkdir bin

bin/lex.yy.c: src/calculator.l 
	lex src/calculator.l
	mv lex.yy.c bin/

bin/y.tab.c: src/calculator.y include/express.h
	yacc -d src/calculator.y
	mv y.tab.h bin/
	mv y.tab.c bin/

clean: 
	rm -rf bin/* 

run: all
	./bin/a.out

test: all
	./bin/a.out < test.in
