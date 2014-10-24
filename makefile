# Variables
EXE=stegano

# Usual compilation flags
CFLAGS=-std=c99 -Wall -Wextra -g -O0
CPPFLAGS=-DDEBUG
LDFLAGS=

# Special rules and targets
.PHONY: all clean help

# Rules and targets
all: $(EXE)

$(EXE): main.o stegano.o
	$(CC) $(CFLAGS) main.o stegano.o -o $@ $(LDFLAGS)

main.o: main.c stegano.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c main.c -o main.o

stegano.o: stegano.c stegano.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c stegano.c -o stegano.o

clean:
	@rm -f *~ *.o $(EXE)

help:
	@echo -e "Usage:"
	@echo -e " make [all]\t\tBuild the software"
	@echo -e " make clean\t\tRemove all files generated by make"
	@echo -e " make help\t\tDisplay this help"