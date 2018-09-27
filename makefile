# Makefile for executable adjust

# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -Werror

# ****************************************************
# Entries to bring the executable up to date

all: student2.o project2.o
	$(CC) $(CFLAGS) -o project2 student2.o project2.o

student2.o: student2.c project2.h
	$(CC) $(CFLAGS) -c student2.c

project2.o: project2.c project2.h
	$(CC) $(CFLAGS) -c project2.c
	
clean:
	rm *.o project2