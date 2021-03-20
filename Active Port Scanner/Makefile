# define more variables so it is easier to make changes
# again, study the new varabiles and rules
CC=cc
CFLAGS=-g -Wall -Werror -std=c99
TARGETS=scanner

all: $(TARGETS)

#Static Pattern Rules
$(TARGETS): % : %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf *.o *~ $(TARGETS)

