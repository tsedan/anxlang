CC = clang++
CFLAGS = -O3 -Wall -pedantic -std=c++11

bin/anx: bin/anx.o bin/lexer.o bin/ast.o bin/ir.o
	$(CC) -g -o bin/anx bin/anx.o bin/lexer.o bin/ast.o bin/ir.o $(CFLAGS)

bin/anx.o: src/anx.cpp src/anx.h src/lexer.h src/ast.h src/ir.h
	mkdir -p bin
	$(CC) -g -c -o bin/anx.o src/anx.cpp $(CFLAGS)

bin/lexer.o: src/lexer.cpp src/lexer.h src/anx.h
	mkdir -p bin
	$(CC) -g -c -o bin/lexer.o src/lexer.cpp $(CFLAGS)

bin/ast.o : src/ast.cpp src/ast.h src/lexer.h
	mkdir -p bin
	$(CC) -g -c -o bin/ast.o src/ast.cpp $(CFLAGS)

bin/ir.o : src/ir.cpp src/ir.h src/ast.h
	mkdir -p bin
	$(CC) -g -c -o bin/ir.o src/ir.cpp $(CFLAGS)

clean:
	rm -r bin
