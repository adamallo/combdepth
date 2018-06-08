ifndef CC
CC=gcc
endif

CFLAGS:= $(CFLAGS) -Wall
PERF=-O2
DBG=-DDEBUG -O0
OBJDIR=./out
SRCDIR=./src
BINDIR=./bin
EXECUTABLES=combdepth

_OBJECTS=
OBJECTS=$(patsubst %,$(OBJDIR)/%,$(_OBJECTS))
_DBG_OBJECTS=
DBG_OBJECTS=$(patsubst %,$(OBJDIR)/%,$(_DBG_OBJECTS))

#Dynamic libraries
C_LIBS= -lm #Allways dynamically linked
D_LIBS= -lz

#Static libraries for MAC
_S_LIBS= libz.a
LD_LIBRARY_PATH=/opt/local/lib
MS_LIBS=$(patsubst %,$(LD_LIBRARY_PATH)/%,$(_S_LIBS)) #BSD's LD needs the full path

#Static libraries for Linux
LS_LIBS= -lz

UNAME_S= $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
LIBS= -Wl,-no_pie $(C_LIBS) $(MS_LIBS)
else
LIBS= -Wl,-Bstatic $(LS_LIBS) -Wl,-Bdynamic $(C_LIBS)
endif

$(BINDIR)/combdepth: $(SRCDIR)/main.c $(OBJECTS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(PERF) $^ -o $@ $(C_LIBS) $(D_LIBS)
	@echo "\nCombDepth built"

$(BINDIR)/combdepth_dbg: $(SRCDIR)/main.c $(DBG_OBJECTS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DBG) -g $^ -o $@ $(C_LIBS) $(D_LIBS)
	@echo "\nDebug version of CombDepth built"

$(BINDIR)/combdepth_static: $(SRCDIR)/main.c $(OBJECTS)
	mkdir -p $(BINDIR)
	@echo "\nThis target has been designed for internal usage and may not work properly in your system\n"
	$(CC) $(CFLAGS) $(LDFLAGS) $(PERF) $^ -o $@ $(LIBS)
	@echo "\nStatic-linked version of CombDepth built"

.PHONY: clean
.PHONY: all
.PHONY: debug
.PHONY: static
.PHONY: combdepth

combdepth: $(BINDIR)/combdepth
combdepth_debug: $(BINDIR)/combdepth_dbg
combdepth_dbg: combdepth_debug
combdepth_static: $(BINDIR)/combdepth_static
static: combdepth_static
debug: combdepth_dbg
all: $(EXECUTABLES)

clean:
	@echo "Cleaning object files directory\n"
	rm -f $(OBJECTS) $(DBG_OBJECTS)
	rm -rf $(OBJDIR)
	rm -rf combdepth_dbg.dSYM
