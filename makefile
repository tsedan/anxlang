CC = clang++
CFLAGS = -O3 -Wall -pedantic -std=c++17
LLVMFLAGS = `llvm-config --cxxflags`
LINKERFLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs core`

build/anx: build/anx.o build/lexer.o build/ast.o build/ir.o | build
	$(CC) -g -o build/anx build/anx.o build/lexer.o build/ast.o build/ir.o $(CFLAGS) $(LINKERFLAGS) 

build/anx.o: src/anx.cpp src/anx.h src/ast.h src/ir.h | build
	$(CC) -g -c -o build/anx.o src/anx.cpp $(CFLAGS) $(LLVMFLAGS)

build/lexer.o: src/lexer.cpp src/lexer.h src/anx.h | build
	$(CC) -g -c -o build/lexer.o src/lexer.cpp $(CFLAGS) $(LLVMFLAGS)

build/ast.o : src/ast.cpp src/ast.h src/lexer.h src/anx.h | build
	$(CC) -g -c -o build/ast.o src/ast.cpp $(CFLAGS) $(LLVMFLAGS)

build/ir.o : src/ir.cpp src/ir.h src/ast.h src/anx.h | build
	$(CC) -g -c -o build/ir.o src/ir.cpp $(CFLAGS) $(LLVMFLAGS)

clean:
	rm -rf build

build:
	mkdir -p $@
