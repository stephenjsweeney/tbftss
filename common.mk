PROG = tbftss

TARGET = $(PROG)$(EXEEXT)

VERSION = 0.4
REVISION = $(shell date +"%y%m%d")
DEBUG = 0

CXXFLAGS += `sdl2-config --cflags` -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DDEBUG=$(DEBUG) -DDATA_DIR=\"$(DATA_DIR)\"
CXXFLAGS += -Wall -ansi -pedantic -Werror -Wstrict-prototypes
CXXFLAGS += -g -lefence

LIBS := `sdl2-config --libs` -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lm

SEARCHPATH += src/ src/battle src/draw src/game src/galaxy src/json src/system src/test
vpath %.c $(SEARCHPATH)
vpath %.h $(SEARCHPATH)

DEPS += defs.h structs.h

OBJS += ai.o
OBJS += battle.o bullets.o
OBJS += challenges.o cJSON.o
OBJS += draw.o
OBJS += effects.o entities.o extractionPoint.o
OBJS += fighters.o
OBJS += galacticMap.o game.o grid.o
OBJS += hud.o
OBJS += init.o input.o io.o items.o
OBJS += load.o lookup.o
OBJS += main.o messageBox.o mission.o missionInfo.o
OBJS += objectives.o options.o
OBJS += player.o
OBJS += radar.o rope.o
OBJS += save.o script.o sound.o starfield.o starSystems.o stats.o
OBJS += testMission.o textures.o text.o title.o transition.o
OBJS += util.o
OBJS += waypoints.o widgets.o

DIST_FILES = data gfx manual music sound src LICENSE makefile* common.mk README.md CHANGELOG

# top-level rule to create the program.
all: $(TARGET)

# compiling other source files.
%.o: %.c %.h $(DEPS)
	$(CC) $(CFLAGS) $(CXXFLAGS) -c $<

# linking the program.
$(TARGET): $(OBJS)
	$(CC)  -o $@ $(OBJS) $(LIBS)
	
install:
	cp $(TARGET) $(BIN_DIR)
	mkdir -p $(DATA_DIR)
	cp -r data $(DATA_DIR)
	cp -r gfx $(DATA_DIR)
	cp -r manual $(DATA_DIR)
	cp -r music $(DATA_DIR)
	cp -r sound $(DATA_DIR)
	
uninstall:
	$(RM) $(BIN_DIR)/$(TARGET)
	$(RM) -rf $(DATA_DIR)

# prepare an archive for the program
dist:
	$(RM) -rf $(PROG)-$(VERSION)
	mkdir $(PROG)-$(VERSION)
	cp -r $(DIST_FILES) $(PROG)-$(VERSION)
	git log --oneline --decorate >$(PROG)-$(VERSION)/CHANGELOG.raw
	tar czf $(PROG)-$(VERSION)-$(REVISION).src.tar.gz $(PROG)-$(VERSION)
	mkdir -p dist
	$(RM) -rf dist/*
	mv $(PROG)-$(VERSION)-$(REVISION).src.tar.gz dist
	$(RM) -rf $(PROG)-$(VERSION)

# cleaning everything that can be automatically recreated with "make".
clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY: dist
