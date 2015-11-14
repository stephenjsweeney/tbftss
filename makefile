PROG = tbftss

VERSION = 0.3
REVISION := $(shell git rev-list --count HEAD)
DEBUG = 0

CXXFLAGS += `sdl2-config --cflags` -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DUNIX=1 -DDEBUG=$(DEBUG)
CXXFLAGS += -DUNIX
CXXFLAGS += $(CFLAGS) -Wall -ansi -pedantic -Werror -Wstrict-prototypes
CXXFLAGS += -g -lefence

LIBS = `sdl2-config --libs` -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lm

SEARCHPATH += src/ src/battle src/draw src/game src/galaxy src/json src/system src/test
vpath %.c $(SEARCHPATH)
vpath %.h $(SEARCHPATH)

DEPS += defs.h structs.h

OBJS += ai.o
OBJS += battle.o bullets.o
OBJS += challenges.o cJSON.o
OBJS += draw.o
OBJS += effects.o entities.o extractionPoint.o
OBJS += fighters.o fighterDefs.o
OBJS += galacticMap.o game.o grid.o
OBJS += hud.o
OBJS += init.o io.o
OBJS += load.o lookup.o
OBJS += main.o mission.o missionInfo.o
OBJS += objectives.o options.o
OBJS += player.o
OBJS += radar.o rope.o
OBJS += save.o sound.o starfield.o starSystems.o stats.o
OBJS += testMission.o textures.o text.o title.o transition.o triggers.o
OBJS += util.o
OBJS += waypoints.o widgets.o

DIST_FILES = data gfx manual music sound src LICENSE makefile README.md

# top-level rule to create the program.
all: $(PROG)

# compiling other source files.
%.o: %.c %.h $(DEPS)
	$(CC) $(CXXFLAGS) -c $<

# linking the program.
$(PROG): $(OBJS)
	$(CC)  -o $(PROG) $(OBJS) $(LIBS)
	
# prepare an archive for the program
dist:
	$(RM) -rf $(PROG)-$(VERSION)
	mkdir $(PROG)-$(VERSION)
	cp -r $(DIST_FILES) $(PROG)-$(VERSION)
	git log --oneline master..develop >$(PROG)-$(VERSION)/CHANGELOG
	tar czf $(PROG)-$(VERSION).$(REVISION)-src.tar.gz $(PROG)-$(VERSION)
	mkdir -p dist
	mv $(PROG)-$(VERSION).$(REVISION)-src.tar.gz dist
	$(RM) -rf $(PROG)-$(VERSION)

# cleaning everything that can be automatically recreated with "make".
clean:
	$(RM) $(OBJS) $(PROG)

.PHONY: dist
