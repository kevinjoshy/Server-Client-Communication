# define more variables so it is easier to make changes
# again, study the new varabiles and rules
CC=cc
CFLAGS=-g -Wall -Werror -std=c99
TARGETS=workon strrender generate-fontfile strrender2

all: $(TARGETS)

#Static Pattern Rules
$(TARGETS): % : %.c
	$(CC) $(CFLAGS) -o $@ $<

bugs: bugs.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o *~ $(TARGETS) bugs *.out font8x8.dat
	cp -f bugs0.c bugs.c
