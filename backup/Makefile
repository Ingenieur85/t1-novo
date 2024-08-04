# Names of executables
SERVER_PROG = server
CLIENT_PROG = client

# Directories
SRCDIR = src
INCDIR = include

# Compiler
CC = gcc

# Compilation flags
CFLAGS = -Wall -Wextra -Wpedantic -O2 -I$(INCDIR)
LFLAGS = -lm

# Source files
SRCS = $(wildcard $(SRCDIR)/*.c)

# Object files
OBJS = $(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(SRCS))

# Exclude client.o and server.o from OBJS
SERVER_OBJS = $(filter-out $(SRCDIR)/client.o, $(OBJS))
CLIENT_OBJS = $(filter-out $(SRCDIR)/server.o, $(OBJS))

# Files for distribution
DISTFILES = $(SRCDIR)/*.c $(INCDIR)/*.h Makefile README.md
DISTDIR = fasf22
DISTNAME = fasf22.tar.gz

all: $(SERVER_PROG) $(CLIENT_PROG)

$(SERVER_PROG): $(SERVER_OBJS) $(SRCDIR)/server.o
	$(CC) -o $@ $^ $(LFLAGS)

$(CLIENT_PROG): $(CLIENT_OBJS) $(SRCDIR)/client.o
	$(CC) -o $@ $^ $(LFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean purge dist

clean:
	@echo "Cleaning up..."
	@rm -f $(SRCDIR)/*.o *~ *.bak

purge: clean
	@echo "Purging build artifacts..."
	@rm -f $(SERVER_PROG) $(CLIENT_PROG) core a.out $(DISTDIR) $(DISTDIR).tar.gz

dist: purge
	@echo "Generating distribution file ($(DISTNAME))..."
	@mkdir -p $(DISTDIR)/$(SRCDIR) $(DISTDIR)/$(INCDIR)
	@cp -r $(SRCDIR)/*.c $(DISTDIR)/$(SRCDIR)
	@cp -r $(INCDIR)/*.h $(DISTDIR)/$(INCDIR)
	@cp Makefile README.md $(DISTDIR)
	@tar -czvf $(DISTNAME) $(DISTDIR)
	@rm -rf $(DISTDIR)

