CC=gcc
CFLAGS=-O0 -g -Wall

LD=$(CC)
LDFLAGS=-g

all:	logtest

help:
	@echo "Targets:"
	@echo
	@echo "all  - build logtest."
	@echo "clean - remove all targets."
	@echo "help  - this message."

logging.o: logging.c logging.h
	$(CC) $(CFLAGS) -c $< -o $@

logtest.o: logtest.c logging.h
	$(CC) $(CFLAGS) -c $< -o $@

logtest: logtest.o logging.o
	$(LD) -o logtest $(LDFLAGS) $^

clean:
	rm -f logtest *.o

.PHONY:	clean
