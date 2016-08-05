VERSION = 0.97
REVISION = $(shell date +"%y%m%d")
LOCALE_MO = $(patsubst %.po,%.mo,$(wildcard locale/*.po))

SEARCHPATH += src/ src/battle src/challenges src/draw src/game src/galaxy src/json src/system src/test
vpath %.c $(SEARCHPATH)
vpath %.h $(SEARCHPATH)

DEPS += defs.h structs.h

OBJS += ai.o
OBJS += battle.o bullets.o
OBJS += capitalShips.o challengeHome.o challenges.o cJSON.o controls.o credits.o
OBJS += debris.o dev.o draw.o
OBJS += effects.o entities.o
OBJS += fighters.o
OBJS += galacticMap.o game.o
OBJS += hud.o
OBJS += i18n.o init.o input.o io.o items.o
OBJS += jumpgate.o
OBJS += load.o locations.o lookup.o
OBJS += main.o messageBox.o mine.o mission.o missionInfo.o modalDialog.o
OBJS += objectives.o options.o
OBJS += player.o
OBJS += quadtree.o
OBJS += radar.o resources.o rope.o
OBJS += save.o script.o sound.o spawners.o starfield.o starSystems.o stats.o
OBJS += testMission.o textures.o text.o title.o transition.o trophies.o
OBJS += util.o
OBJS += waypoints.o widgets.o

# top-level rule to create the program.
all: $(PROG) $(LOCALE_MO)

# compiling other source files.
%.o: %.c %.h $(DEPS)
	$(CC) $(CFLAGS) $(CXXFLAGS) -c $<
	
%.mo: %.po
	msgfmt -c -o $@ $<
	
# cleaning everything that can be automatically recreated with "make".
clean:
	$(RM) $(OBJS) $(PROG) $(LOCALE_MO)
