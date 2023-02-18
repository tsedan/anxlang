CC = clang++
CFLAGS = -O3 -Wall -pedantic -std=c++17
LLVMFLAGS = `llvm-config --cxxflags`
LINKERFLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs core`

bin/anx: bin/anx.o bin/lexer.o bin/ast.o bin/ir.o bin/utils.o bin/opti.o | bin
	$(CC) -g -o bin/anx bin/anx.o bin/lexer.o bin/ast.o bin/ir.o bin/utils.o bin/opti.o $(CFLAGS) $(LINKERFLAGS) 

bin/anx.o: src/anx.cpp src/anx.h src/ast.h src/ir.h | bin
	$(CC) -g -c -o bin/anx.o src/anx.cpp $(CFLAGS) $(LLVMFLAGS)

bin/lexer.o: src/lexer.cpp src/lexer.h src/anx.h | bin
	$(CC) -g -c -o bin/lexer.o src/lexer.cpp $(CFLAGS) $(LLVMFLAGS)

bin/ast.o : src/ast.cpp src/ast.h src/lexer.h src/anx.h | bin
	$(CC) -g -c -o bin/ast.o src/ast.cpp $(CFLAGS) $(LLVMFLAGS)

bin/ir.o : src/ir.cpp src/ir.h src/ast.h src/anx.h src/opti.h | bin
	$(CC) -g -c -o bin/ir.o src/ir.cpp $(CFLAGS) $(LLVMFLAGS)

bin/utils.o : src/utils.cpp src/ir.h src/ast.h src/anx.h | bin
	$(CC) -g -c -o bin/utils.o src/utils.cpp $(CFLAGS) $(LLVMFLAGS)

bin/opti.o : src/opti.cpp src/opti.h src/anx.h | bin
	$(CC) -g -c -o bin/opti.o src/opti.cpp $(CFLAGS) $(LLVMFLAGS)

clean:
	rm -rf bin

bin:
	mkdir -p $@
