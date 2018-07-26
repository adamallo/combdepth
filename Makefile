##Global confs
PERF=-O2
DBG=-DDEBUG -O0
OBJDIR=./out
SRCDIR=./src
BINDIR=./bin
EXECUTABLES=combdepth

##No object files
#_OBJECTS=
#OBJECTS=$(patsubst %,$(OBJDIR)/%,$(_OBJECTS))
#_DBG_OBJECTS=
#DBG_OBJECTS=$(patsubst %,$(OBJDIR)/%,$(_DBG_OBJECTS))

#Dynamic libraries
C_LIBS= -lm #Allways dynamically linked
_D_D_LIBS= -lz
_L_D_LIBS= -lz -lbsd

#Static libraries for MAC
_S_LIBS= libz.a
LD_LIBRARY_PATH=/opt/local/lib
MS_LIBS=$(patsubst %,$(LD_LIBRARY_PATH)/%,$(_S_LIBS)) #BSD's LD needs the full path

#Static libraries for Linux
LS_LIBS= -lz -lbsd

##Detecting OS

ifeq ($(OS),Windows_NT)
	detected_OS := Windows
else
	detected_OS := $(shell sh -c 'uname -s 2>/dev/null || echo not')
endif

##Getline requires XOPEN>=700, not needed in Mac
CFLAGS:= $(CFLAGS) -Wall

ifeq ($(detected_OS), Darwin)
	LIBS:= -Wl,-no_pie $(C_LIBS) $(MS_LIBS)
	D_LIBS:= $(_D_D_LIBS)
endif

ifeq ($(detected_OS), Linux)
	CC=gcc
	CFLAGS:= $(CFLAGS) -std=c99 -D_XOPEN_SOURCE=700
	D_LIBS:= $(_L_D_LIBS)
	LIBS:= -Wl,-Bstatic $(LS_LIBS) -Wl,-Bdynamic $(C_LIBS)
endif

##Rules

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
.PHONY: combdepth_debug
.PHONY: static

combdepth: $(BINDIR)/combdepth
combdepth_dbg: $(BINDIR)/combdepth_dbg
combdepth_debug: combdepth_dbg
combdepth_static: $(BINDIR)/combdepth_static
static: combdepth_static
debug: combdepth_dbg
all: $(EXECUTABLES)

clean:
	@echo "Cleaning object files directory\n"
	rm -f $(OBJECTS) $(DBG_OBJECTS)
	rm -rf $(OBJDIR)
	rm -rf combdepth_dbg.dSYM
