PROG = tbftss
CC = gcc
PREFIX ?= /usr
BIN_DIR ?= $(PREFIX)/bin
DATA_DIR ?= /opt/$(PROG)
LOCALE_DIR = $(PREFIX)/share/locale
ICON_DIR = $(PREFIX)/share/icons/hicolor
DESKTOP_DIR = $(PREFIX)/share/applications

DESTDIR ?=
INST_BIN_DIR = $(DESTDIR)$(BIN_DIR)
INST_DATA_DIR = $(DESTDIR)$(DATA_DIR)
INST_LOCALE_DIR = $(DESTDIR)$(LOCALE_DIR)
INST_ICON_DIR = $(DESTDIR)$(ICON_DIR)
INST_DESKTOP_DIR = $(DESTDIR)$(DESKTOP_DIR)

SEARCHPATH += src/plat/unix
_OBJS += unixInit.o

include common.mk

CXXFLAGS += `sdl2-config --cflags` -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DDATA_DIR=\"$(DATA_DIR)\" -DLOCALE_DIR=\"$(LOCALE_DIR)\"
CXXFLAGS += -Wall -Wempty-body -ansi -pedantic -Werror -Wstrict-prototypes -Werror=maybe-uninitialized -Warray-bounds
CXXFLAGS += -g -lefence

LDFLAGS += `sdl2-config --libs` -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lm

SHARED_FILES = CHANGELOG LICENSE README.md data gfx manual music sound icons
DIST_FILES = $(SHARED_FILES) locale tbftss
SRC_DIST_FILES = $(SHARED_FILES) src makefile* common.mk

# linking the program.
$(PROG): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

install:
	mkdir -p $(INST_BIN_DIR)
	install -m 0755 $(PROG) $(INST_BIN_DIR)
	mkdir -p $(INST_DATA_DIR)
	cp -r data $(INST_DATA_DIR)
	cp -r gfx $(INST_DATA_DIR)
	cp -r manual $(INST_DATA_DIR)
	cp -r music $(INST_DATA_DIR)
	cp -r sound $(INST_DATA_DIR)
	mkdir -p $(INST_ICON_DIR)/16x16/apps
	mkdir -p $(INST_ICON_DIR)/32x32/apps
	mkdir -p $(INST_ICON_DIR)/64x64/apps
	mkdir -p $(INST_ICON_DIR)/128x128/apps
	cp -p icons/$(PROG)-16x16.png $(INST_ICON_DIR)/16x16/apps/$(PROG).png
	cp -p icons/$(PROG)-32x32.png $(INST_ICON_DIR)/32x32/apps/$(PROG).png
	cp -p icons/$(PROG)-64x64.png $(INST_ICON_DIR)/64x64/apps/$(PROG).png
	cp -p icons/$(PROG)-128x128.png $(INST_ICON_DIR)/128x128/apps/$(PROG).png
	mkdir -p $(INST_DESKTOP_DIR)
	cp -p icons/$(PROG).desktop $(INST_DESKTOP_DIR)
	
	@for f in $(LOCALE_MO); do \
		lang=`echo $$f | sed -e 's/^locale\///;s/\.mo$$//'`; \
		mkdir -p $(INST_LOCALE_DIR)/$$lang/LC_MESSAGES; \
		cp -v $$f $(INST_LOCALE_DIR)/$$lang/LC_MESSAGES/$(PROG).mo; \
	done
	
uninstall:
	$(RM) $(BIN_DIR)/$(PROG)
	$(RM) -rf $(DATA_DIR)
	$(RM) $(ICON_DIR)/16x16/apps/$(PROG).png
	$(RM) $(ICON_DIR)/32x32/apps/$(PROG).png
	$(RM) $(ICON_DIR)/64x64/apps/$(PROG).png
	$(RM) $(ICON_DIR)/128x128/apps/$(PROG).png
	$(RM) $(DESKTOP_DIR)/$(PROG).desktop
	
	@for f in $(LOCALE_MO); do \
		lang=`echo $$f | sed -e 's/^locale\///;s/\.mo$$//'`; \
		$(RM) -v $(LOCALE_DIR)/$$lang/LC_MESSAGES/$(PROG).mo; \
	done

# prepare an archive for the program
dist:
	$(RM) -rf $(PROG)-$(VERSION)
	mkdir $(PROG)-$(VERSION)
	cp -r $(DIST_FILES) $(PROG)-$(VERSION)
	tar czf $(PROG)-$(VERSION)-$(REVISION).linux-x86.tar.gz $(PROG)-$(VERSION)
	mkdir -p dist
	mv $(PROG)-$(VERSION)-$(REVISION).linux-x86.tar.gz dist
	$(RM) -rf $(PROG)-$(VERSION)
	
# prepare an archive for the program
src-dist:
	$(RM) -rf $(PROG)-$(VERSION)
	mkdir $(PROG)-$(VERSION)
	cp -r $(SRC_DIST_FILES) $(PROG)-$(VERSION)
	git log --pretty=format:"%h%x09%an%x09%ad%x09%s" --date=short >$(PROG)-$(VERSION)/CHANGELOG.raw
	tar czf $(PROG)-$(VERSION)-$(REVISION).src.tar.gz $(PROG)-$(VERSION)
	mkdir -p dist
	mv $(PROG)-$(VERSION)-$(REVISION).src.tar.gz dist
	$(RM) -rf $(PROG)-$(VERSION)

.PHONY: dist
