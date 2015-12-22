VERSION = 0.5
REVISION = $(shell date +"%y%m%d")

SEARCHPATH += src/ src/battle src/draw src/game src/galaxy src/json src/system src/test
vpath %.c $(SEARCHPATH)
vpath %.h $(SEARCHPATH)

DEPS += defs.h structs.h

OBJS += ai.o
OBJS += battle.o bullets.o
OBJS += capitalShips.o challenges.o cJSON.o
OBJS += debris.o dev.o draw.o
OBJS += effects.o entities.o extractionPoint.o
OBJS += fighters.o
OBJS += galacticMap.o game.o grid.o
OBJS += hud.o
OBJS += init.o input.o io.o items.o
OBJS += load.o locations.o lookup.o
OBJS += main.o messageBox.o mission.o missionInfo.o
OBJS += objectives.o options.o
OBJS += player.o
OBJS += radar.o rope.o
OBJS += save.o script.o sound.o starfield.o starSystems.o stats.o
OBJS += testMission.o textures.o text.o title.o transition.o
OBJS += util.o
OBJS += waypoints.o widgets.o

# top-level rule to create the program.
all: $(PROG)

# compiling other source files.
%.o: %.c %.h $(DEPS)
	$(CC) $(CFLAGS) $(CXXFLAGS) -c $<
	
# cleaning everything that can be automatically recreated with "make".
clean:
	$(RM) $(OBJS) $(PROG)
