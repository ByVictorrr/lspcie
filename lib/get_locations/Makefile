#
#   DMI Decode
#   BIOS Decode
#   VPD Decode
#
#   Copyright (C) 2000-2002 Alan Cox <alan@redhat.com>
#   Copyright (C) 2002-2015 Jean Delvare <jdelvare@suse.de>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#

CC      = gcc
CFLAGS  = -W -g -O0 -Wall -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual \
          -Wcast-align -Wwrite-strings -Wmissing-prototypes -Winline -Wundef

# Let lseek and mmap support 64-bit wide offsets
CFLAGS += -D_FILE_OFFSET_BITS=64

#CFLAGS += -DBIGENDIAN
#CFLAGS += -DALIGNMENT_WORKAROUND

# When debugging, disable -O2 and enable -g.
#CFLAGS += -O2
#CFLAGS += -g

# Pass linker flags here
LDFLAGS =

DESTDIR =

INSTALL         := install
INSTALL_DATA    := $(INSTALL) -m 644
INSTALL_DIR     := $(INSTALL) -m 755 -d
INSTALL_PROGRAM := $(INSTALL) -m 755
RM              := rm -f

# BSD make provides $MACHINE, but GNU make doesn't
MACHINE ?= $(shell uname -m 2>/dev/null)

# These programs are only useful on x86

PROGRAMS := phy_slot

all : $(PROGRAMS)

#
# Programs
#

phy_slot : phy_slot.o util.o
	$(CC) $(LDFLAGS) phy_slot.o util.o -o $@


#
# Objects
#

phy_slot.o : phy_slot.c phy_slot.h types.h util.h config.h 
	$(CC) $(CFLAGS) -c $< -o $@

util.o : util.c types.h util.h config.h
	$(CC) $(CFLAGS) -c $< -o $@

#
# Commands
#

strip : $(PROGRAMS)
	strip $(PROGRAMS)

install : install-bin install-man install-doc

uninstall : uninstall-bin uninstall-man uninstall-doc

install-bin : $(PROGRAMS)
	$(INSTALL_DIR) $(DESTDIR)$(sbindir)
	for program in $(PROGRAMS) ; do \
	$(INSTALL_PROGRAM) $$program $(DESTDIR)$(sbindir) ; done

uninstall-bin :
	for program in $(PROGRAMS) ; do \
	$(RM) $(DESTDIR)$(sbindir)/$$program ; done

install-man :
	$(INSTALL_DIR) $(DESTDIR)$(man8dir)
	for program in $(PROGRAMS) ; do \
	$(INSTALL_DATA) man/$$program.8 $(DESTDIR)$(man8dir) ; done

uninstall-man :
	for program in $(PROGRAMS) ; do \
	$(RM) $(DESTDIR)$(man8dir)/$$program.8 ; done

install-doc :
	$(INSTALL_DIR) $(DESTDIR)$(docdir)
	$(INSTALL_DATA) README $(DESTDIR)$(docdir)
	$(INSTALL_DATA) NEWS $(DESTDIR)$(docdir)
	$(INSTALL_DATA) AUTHORS $(DESTDIR)$(docdir)

uninstall-doc :
	$(RM) -r $(DESTDIR)$(docdir)

clean :
	$(RM) *.o $(PROGRAMS) core
