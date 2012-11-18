#!Makefile

.PHONY:clean

OBJECT  = huffman.o
INCLUDE = huffman.h
CC = gcc

all : ${OBJECT}
	${CC} ${OBJECT} -o huffman

huffman.o : huffman.c ${INCLUDE}
	${CC} -c huffman.c

clean : 
	${RM} ${OBJECT} huffman
