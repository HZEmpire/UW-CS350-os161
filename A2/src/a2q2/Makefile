OBJS	= assignment.o orderme.o
SOURCE	= assignment.c orderme.c
HEADER	= structs.h assignment.h
OUT	= a.out
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = -pthread

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

assignment.o: assignment.c
	$(CC) $(FLAGS) assignment.c

orderme.o: orderme.c
	$(CC) $(FLAGS) orderme.c

clean:
	rm -f $(OBJS) $(OUT)
.PHONY: all clean
