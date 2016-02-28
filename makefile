PROG = tbftss
CC = gcc
BIN_DIR = /usr/bin
DATA_DIR = /opt/tbftss
LOCALE_DIR = /usr/share/locale

SEARCHPATH += src/plat/unix
OBJS += unixInit.o

include common.mk

CXXFLAGS += `sdl2-config --cflags` -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DDATA_DIR=\"$(DATA_DIR)\" -DLOCALE_DIR=\"$(LOCALE_DIR)\"
CXXFLAGS += -Wall -ansi -pedantic -Werror -Wstrict-prototypes
CXXFLAGS += -g -lefence

LFLAGS := `sdl2-config --libs` -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lm

DIST_FILES = data gfx manual music sound src LICENSE makefile* common.mk README.md CHANGELOG

# linking the program.
$(PROG): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LFLAGS)

install:
	cp $(PROG) $(BIN_DIR)
	mkdir -p $(DATA_DIR)
	cp -r data $(DATA_DIR)
	cp -r gfx $(DATA_DIR)
	cp -r manual $(DATA_DIR)
	cp -r music $(DATA_DIR)
	cp -r sound $(DATA_DIR)
	
	@for f in $(LOCALE_MO); do \
		lang=`echo $$f | sed -e 's/^locale\///;s/\.mo$$//'`; \
		mkdir -p $(LOCALE_DIR)/$$lang/LC_MESSAGES; \
		cp -v $$f $(LOCALE_DIR)/$$lang/LC_MESSAGES/$(PROG).mo; \
	done
	
uninstall:
	$(RM) $(BIN_DIR)/$(PROG)
	$(RM) -rf $(DATA_DIR)
	
	@for f in $(LOCALE_MO); do \
		lang=`echo $$f | sed -e 's/^locale\///;s/\.mo$$//'`; \
		$(RM) -v $(LOCALE_DIR)/$$lang/LC_MESSAGES/$(PROG).mo; \
	done

# prepare an archive for the program
dist:
	$(RM) -rf $(PROG)-$(VERSION)
	mkdir $(PROG)-$(VERSION)
	cp -r $(DIST_FILES) $(PROG)-$(VERSION)
	git log --oneline --decorate >$(PROG)-$(VERSION)/CHANGELOG.raw
	tar czf $(PROG)-$(VERSION)-$(REVISION).src.tar.gz $(PROG)-$(VERSION)
	mkdir -p dist
	mv $(PROG)-$(VERSION)-$(REVISION).src.tar.gz dist
	$(RM) -rf $(PROG)-$(VERSION)

.PHONY: dist
