CFLAGS = -g -Wall

encrypt: build
	./filesec -e testfile1.txt
build:
	gcc filesec.c -o filesec $(CFLAGS)
clean:
	rm *enc.txt *dec.txt
