CC = clang++
CFLAGS = -O3 -Wall -pedantic

bin/anx: bin/anx.o bin/tokenizer.o
	$(CC) -g -o bin/anx bin/anx.o bin/tokenizer.o $(CFLAGS)

bin/anx.o: src/anx.cpp src/tokenizer.h
	mkdir -p bin
	$(CC) -g -c -o bin/anx.o src/anx.cpp $(CFLAGS)

bin/tokenizer.o: src/tokenizer.cpp src/tokenizer.h
	mkdir -p bin
	$(CC) -g -c -o bin/tokenizer.o src/tokenizer.cpp $(CFLAGS)

clean:
	rm -r bin
