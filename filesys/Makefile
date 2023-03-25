# name of the program to build
#
PROG=pennfat
PROMPT='"PennFAT> "'

# Remove -DNDEBUG during development if assert(3) is used
#
override CPPFLAGS += -DNDEBUG -DPROMPT=$(PROMPT)

CC = clang

# Replace -O1 with -g for a debug version during development
#
CFLAGS = -Wall -Werror -O1

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

.PHONY : clean

$(PROG) : $(OBJS)
	$(CC) -o $@ $^

clean :
	$(RM) $(OBJS) $(PROG)
