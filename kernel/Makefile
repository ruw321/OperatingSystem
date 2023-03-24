PROG=pennOS
CC = clang

# Replace -O3 with -g for a debug version during development
CFLAGS = -Wall -Werror -ggdb3

# Comment out the line below during development if assert(3) is used
# CPPFLAGS = -DNDEBUG


TOPDIR = $(PWD)
LOGFILE = \"$(TOPDIR)/log/log.txt\"
override CPPFLAGS += -DLOGFILE=$(LOGFILE)

SRCS = $(filter-out main.c, $(wildcard *.c))
OBJS = $(SRCS:.c=.o)

.PHONY : all clean

$(PROG) : $(OBJS)
	$(CC) -o $@ $^

clean :
	$(RM) $(OBJS) $(PROG)


