# Makefile for executable adjust

# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -Werror -g

# ****************************************************
# Entries to bring the executable up to date

all: student2.o project2.o linkedlist.o
	$(CC) $(CFLAGS) -o project2 student2.o project2.o linkedlist.o

student2.o: student2.c project2.h ./linkedlist/linkedlist.h
	$(CC) $(CFLAGS) -c student2.c

project2.o: project2.c project2.h
	$(CC) $(CFLAGS) -c project2.c
	
linkedlist.o: ./linkedlist/linkedlist.c ./linkedlist/linkedlist.h
	$(CC) $(CFLAGS) -c ./linkedlist/linkedlist.c
	
clean:
	rm *.o project2