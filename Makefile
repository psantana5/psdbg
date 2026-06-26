CC = gcc
CFLAGS = -Wall -Wextra -O2 -Isrc
SRCDIR = src
OBJDIR = obj
BINDIR = bin

OBJS = $(OBJDIR)/proc.o $(OBJDIR)/list.o $(OBJDIR)/detail.o \
       $(OBJDIR)/why.o $(OBJDIR)/tree.o $(OBJDIR)/threads.o $(OBJDIR)/sched.o \
       $(OBJDIR)/memory.o $(OBJDIR)/affinity.o $(OBJDIR)/namespaces.o $(OBJDIR)/main.o

$(BINDIR)/psdbg: $(OBJS) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/proc.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR) $(BINDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY: clean
