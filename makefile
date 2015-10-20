PROG = tbftss

VERSION = 0.1
RELEASE = 1

CXXFLAGS += `sdl2-config --cflags` -DVERSION=$(VERSION) -DRELEASE=$(RELEASE) -DUNIX=1
CXXFLAGS += -DUNIX
CXXFLAGS += $(CFLAGS) -Wall -ansi -pedantic -Werror -Wstrict-prototypes
CXXFLAGS += -g -lefence

LIBS = `sdl2-config --libs` -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lm

SEARCHPATH += src/ src/battle src/draw src/game src/galaxy src/json src/system
vpath %.c $(SEARCHPATH)
vpath %.h $(SEARCHPATH)

DEPS += defs.h structs.h

OBJS += ai.o
OBJS += battle.o bullets.o
OBJS += challenges.o cJSON.o
OBJS += draw.o
OBJS += effects.o
OBJS += fighters.o fighterDefs.o
OBJS += galacticMap.o game.o
OBJS += hud.o
OBJS += init.o io.o
OBJS += load.o lookup.o
OBJS += main.o mission.o missionInfo.o
OBJS += objectives.o options.o
OBJS += player.o
OBJS += radar.o
OBJS += save.o sound.o starfield.o starSystems.o stats.o
OBJS += textures.o text.o title.o transition.o
OBJS += util.o
OBJS += widgets.o

# top-level rule to create the program.
all: $(PROG)

# compiling other source files.
%.o: %.c %.h $(DEPS)
	$(CC) $(CXXFLAGS) -c $<

# linking the program.
$(PROG): $(OBJS)
	$(CC)  -o $(PROG) $(OBJS) $(LIBS)

# cleaning everything that can be automatically recreated with "make".
clean:
	$(RM) $(OBJS) $(PROG) $(SERVEROBJS) $(SERVER)
