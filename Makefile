# Makes the game

TARGET := psgame
TYPE := ps-exe
BUILD := Debug

CURRDIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

BINDIR := build/bin/
SRCDIR := src

CPPFLAGS := -I$(CURRDIR)nugget
CPPFLAGS += -I$(CURRDIR)nugget/psyq/include
CPPFLAGS += -I$(CURRDIR)psyq/include
CPPFLAGS += -I$(CURRDIR)inc
CPPFLAGS += -Wall -Wextra -pedantic

CFLAGS := $(CPPFLAGS)

ifeq ($(BUILD),Debug)
DEFINES += DEBUG
else
DEFINES += NDEBUG
endif

LDFLAGS := -L$(CURRDIR)nugget/psyq/lib
LDFLAGS += -L$(CURRDIR)psyq/lib
LDFLAGS += -T$(CURRDIR)linker.ld
LDFLAGS += -Wl,--start-group
LDFLAGS += -lapi
LDFLAGS += -lc
LDFLAGS += -lc2
LDFLAGS += -lcard
LDFLAGS += -lcomb
LDFLAGS += -lds
LDFLAGS += -letc
LDFLAGS += -lgpu
LDFLAGS += -lgs
LDFLAGS += -lgte
LDFLAGS += -lgun
LDFLAGS += -lhmd
LDFLAGS += -lmath
LDFLAGS += -lmcrd
LDFLAGS += -lmcx
LDFLAGS += -lpad
LDFLAGS += -lpress
LDFLAGS += -lsio
LDFLAGS += -lsnd
LDFLAGS += -lspu
LDFLAGS += -ltap
LDFLAGS += -lcd
LDFLAGS += -Wl,--end-group

SRCS := $(SRCDIR)/main.c
SRCS += $(SRCDIR)/camera.c
SRCS += $(SRCDIR)/cd.c
SRCS += $(SRCDIR)/gfx.c
SRCS += $(SRCDIR)/image.c
SRCS += $(SRCDIR)/input.c
SRCS += $(SRCDIR)/mem.c
SRCS += $(SRCDIR)/sprite.c
SRCS += $(SRCDIR)/text.c

SRCS += $(CURRDIR)nugget/common/crt0/crt0.s
SRCS += $(CURRDIR)nugget/common/syscalls/printf.s

include $(CURRDIR)nugget/common.mk

.PHONY: all
all:
	@echo "BUILD 1997-$(shell date +'%m-%d %H:%M:%S')" >build/INFO.TXT
	mkpsxiso -y -o build/iso/psgame.bin -c build/iso/psgame.cue build/iso.xml
	rm $(SRCDIR)/*.dep
	rm $(SRCDIR)/*.o

