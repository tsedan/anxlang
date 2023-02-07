CC = clang++
CFLAGS = -O3 -Wall -pedantic -std=c++14 `llvm-config --cxxflags`
OBJFLAGS = -O3 -Wall -pedantic -std=c++14 `llvm-config --cxxflags --ldflags --system-libs --libs core`

build/anx: build/anx.o build/lexer.o build/ast.o build/ir.o | build
	$(CC) -g -o build/anx build/anx.o build/lexer.o build/ast.o build/ir.o $(OBJFLAGS) 

build/anx.o: src/anx.cpp src/anx.h src/ast.h src/ir.h | build
	$(CC) -g -c -o build/anx.o src/anx.cpp $(CFLAGS)

build/lexer.o: src/lexer.cpp src/lexer.h src/anx.h | build
	$(CC) -g -c -o build/lexer.o src/lexer.cpp $(CFLAGS)

build/ast.o : src/ast.cpp src/ast.h src/lexer.h src/anx.h | build
	$(CC) -g -c -o build/ast.o src/ast.cpp $(CFLAGS)

build/ir.o : src/ir.cpp src/ir.h src/ast.h src/anx.h | build
	$(CC) -g -c -o build/ir.o src/ir.cpp $(CFLAGS)

clean:
	rm -rf build

build:
	mkdir -p $@
