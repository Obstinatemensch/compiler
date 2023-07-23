all: bin main 

main: bin/y.tab.c bin/lex.yy.c  src/express.cpp
	g++ -g bin/y.tab.c bin/lex.yy.c src/express.cpp -o compiler
	mv compiler bin/
	
bin: 
	mkdir bin

bin/lex.yy.c: src/calculator.l 
	lex src/calculator.l
	mv lex.yy.c bin/

bin/y.tab.c: src/calculator.y include/express.hpp
	yacc -d src/calculator.y
	mv y.tab.h bin/
	mv y.tab.c bin/

clean: 
	rm -rf bin/* 

run: all
	./bin/compiler

test: all
	./bin/compiler < test.in