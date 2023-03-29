PROG=pennOS
CC = clang

# Replace -O3 with -g for a debug version during development
CFLAGS = -Wall

TOPDIR = $(PWD)
# LOGFILE = \"$(TOPDIR)/log/log.txt\"
override CPPFLAGS += -DLOGFILE=$(LOGFILE)

SRCS = $(filter-out main.c, $(wildcard *.c))
OBJS = $(SRCS:.c=.o)

.PHONY : all clean

$(PROG) : $(OBJS)
	$(CC) -o $@ $^

clean :
	$(RM) $(OBJS) $(PROG)


