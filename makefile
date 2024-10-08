CC = gcc
CFLAGS = -Iinclude

all: main test

main: src/main.c src/cbcode.c
	$(CC) $(CFLAGS) -o main src/main.c src/barcodex39.c

test: tests/test_barcode39.c src/barcode39.c
	$(CC) $(CFLAGS) -o test tests/test_barcode39.c src/barcodex39.c

clean:
	rm -f main test
