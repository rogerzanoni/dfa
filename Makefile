CC = gcc
CFLAGS = -g -Wall

all: af

af: main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o af main.c

tests:
	./test.sh input output1
	./test.sh input2 output2
	./test.sh input3 output3
	./test.sh input4 output4
	./test.sh input5 output5
	./test.sh input6 output6
	./test.sh input7 output7
	./test.sh input8 output8
	./test.sh input9 output9
	./test.sh input10 output10
