CC=gcc
CFLAGS=-W  -Wall -Werror -g -std=gnu99
SOURCES = shell.c
OBJECTS = $(SOURCES:.c=.o)
EXEC=shell
.PHONY : default clean

default: $(EXEC)

shell.o : shell.c 


%.o :%.c
	$(CC) -o $@ -c  $< $(CFLAGS)

$(EXEC) : $(OBJECTS)
			$(CC) -o $@ $^ 

clean : 
	 rm $(EXEC) $(OBJECTS)