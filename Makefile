PROG=pennOS
CC = clang

# Replace -O3 with -g for a debug version during development
CFLAGS = -Wall

TOPDIR = $(PWD)
FS_DIR = $(TOPDIR)/PennFAT
KERNEL_DIR = $(TOPDIR)/kernel
SUBDIRS = $(TOPDIR)/PennFAT $(TOPDIR)/kernel
MAIN = $(KERNEL_DIR)/main.c
# LOGFILE = \"$(TOPDIR)/log/log.txt\"
# override CPPFLAGS += -DLOGFILE=$(LOGFILE)

FS_SRCS = $(filter-out $(FS_DIR)/test-playground.c $(FS_DIR)/pennFAT.c,  $(wildcard $(FS_DIR)/*.c))
KERNEL_SRCS = $(filter-out $(KERNEL_DIR)/main.c, $(wildcard $(KERNEL_DIR)/*.c))
FS_OBJS = $(FS_SRCS:.c=.o)
KERNEL_OBJS = $(KERNEL_SRCS:.c=.o)

FS_MAIN = $(FS_DIR)/pennFAT.c
FS_PROG = $(FS_DIR)/pennFAT
PROG = pennOS

.PHONY : $(SUBDIRS) clean

all : $(PROG) $(FS_PROG)
$(PROG) : $(SUBDIRS)
	$(CC) ${CFLAGS} -lm -o $@ $(MAIN) $(KERNEL_OBJS) $(FS_SRCS) $(KERNEL_DIR)/parser.o

$(FS_PROG) : $(FS_SRCS)
	$(CC) ${CFLAGS} -lm -o $@ $(FS_MAIN) $(FS_SRCS) $(KERNEL_OBJS) $(KERNEL_DIR)/parser.o

$(SUBDIRS) :
	$(MAKE) -C $@

clean :
	$(RM) $(KERNEL_OBJS) $(FS_OBJS) $(PROG)


