VERSION = 1.3
REVISION = 1
LOCALE_MO = $(patsubst %.po,%.mo,$(wildcard locale/*.po))

OUT = bin

SEARCHPATH += src/ src/battle src/challenges src/draw src/game src/galaxy src/json src/system src/test
vpath %.c $(SEARCHPATH)
vpath %.h $(SEARCHPATH)

DEPS += defs.h structs.h

_OBJS += ai.o
_OBJS += battle.o bullets.o
_OBJS += capitalShips.o challengeHome.o challenges.o cJSON.o controls.o credits.o
_OBJS += debris.o dev.o draw.o
_OBJS += effects.o entities.o
_OBJS += fighters.o fighterDatabase.o
_OBJS += galacticMap.o game.o
_OBJS += hud.o
_OBJS += i18n.o init.o input.o io.o items.o
_OBJS += jumpgate.o
_OBJS += load.o locations.o lookup.o
_OBJS += main.o messageBox.o mine.o mission.o missionInfo.o modalDialog.o
_OBJS += objectives.o options.o
_OBJS += player.o
_OBJS += quadtree.o
_OBJS += radar.o resources.o rope.o
_OBJS += save.o script.o sound.o spawners.o starfield.o starSystems.o stats.o
_OBJS += testMission.o textures.o text.o title.o transition.o trophies.o
_OBJS += util.o
_OBJS += waypoints.o widgets.o

OBJS = $(patsubst %,$(OUT)/%,$(_OBJS))

# top-level rule to create the program.
all: $(PROG) $(LOCALE_MO)

# compiling other source files.
$(OUT)/%.o: %.c %.h $(DEPS)
	@mkdir -p $(OUT)
	$(CC) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<
	
%.mo: %.po
	msgfmt -c -o $@ $<
	
# cleaning everything that can be automatically recreated with "make".
clean:
	$(RM) $(OBJS) $(PROG) $(LOCALE_MO)
