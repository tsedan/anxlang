CC = clang++
CFLAGS = -O3 -Wall -pedantic

bin/anx: bin/anx.o bin/lexer.o
	$(CC) -g -o bin/anx bin/anx.o bin/lexer.o $(CFLAGS)

bin/anx.o: src/anx.cpp src/anx.h src/lexer.h
	mkdir -p bin
	$(CC) -g -c -o bin/anx.o src/anx.cpp $(CFLAGS)

bin/lexer.o: src/lexer.cpp src/anx.h src/lexer.h
	mkdir -p bin
	$(CC) -g -c -o bin/lexer.o src/lexer.cpp $(CFLAGS)

clean:
	rm -r bin
