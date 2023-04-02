# note: it is recommended to run make with the -j option to significantly reduce build times

CC = clang++
CFLAGS = -O3 -Wall -pedantic -std=c++17
LLVMFLAGS = `llvm-config --cxxflags`
LINKERFLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs core`

bin/anx: bin/anx.o bin/lexer.o bin/ast.o bin/ir.o bin/intr.o bin/utils.o bin/opti.o bin/printer.o | bin
	$(CC) -o bin/anx bin/anx.o bin/lexer.o bin/ast.o bin/ir.o bin/intr.o bin/utils.o bin/opti.o bin/printer.o $(CFLAGS) $(LINKERFLAGS) 

bin/anx.o: src/anx.cpp src/anx.h src/frontend/ast.h src/codegen/ir.h | bin
	$(CC) -c -o bin/anx.o src/anx.cpp $(CFLAGS) $(LLVMFLAGS)

bin/lexer.o: src/frontend/lexer.cpp src/frontend/lexer.h src/anx.h | bin
	$(CC) -c -o bin/lexer.o src/frontend/lexer.cpp $(CFLAGS) $(LLVMFLAGS)

bin/ast.o: src/frontend/ast.cpp src/frontend/ast.h src/frontend/lexer.h src/codegen/ir.h src/anx.h | bin
	$(CC) -c -o bin/ast.o src/frontend/ast.cpp $(CFLAGS) $(LLVMFLAGS)

bin/ir.o: src/codegen/ir.cpp src/codegen/ir.h src/frontend/ast.h src/intrinsics/intr.h src/codegen/opti.h src/anx.h | bin
	$(CC) -c -o bin/ir.o src/codegen/ir.cpp $(CFLAGS) $(LLVMFLAGS)

bin/intr.o: src/intrinsics/intr.cpp src/intrinsics/intr.h src/codegen/ir.h src/anx.h | bin
	$(CC) -c -o bin/intr.o src/intrinsics/intr.cpp $(CFLAGS) $(LLVMFLAGS)

bin/utils.o: src/utils.cpp src/utils.h src/codegen/ir.h src/frontend/ast.h src/anx.h | bin
	$(CC) -c -o bin/utils.o src/utils.cpp $(CFLAGS) $(LLVMFLAGS)

bin/opti.o: src/codegen/opti.cpp src/codegen/opti.h src/codegen/ir.h src/anx.h | bin
	$(CC) -c -o bin/opti.o src/codegen/opti.cpp $(CFLAGS) $(LLVMFLAGS)

bin/printer.o: src/assembly/printer.cpp src/assembly/printer.h src/codegen/ir.h src/anx.h | bin
	$(CC) -c -o bin/printer.o src/assembly/printer.cpp $(CFLAGS) $(LLVMFLAGS)

clean:
	rm -rf bin

bin:
	mkdir -p $@
