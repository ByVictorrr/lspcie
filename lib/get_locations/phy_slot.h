/*
 * This file is part of the dmidecode project.
 *
 *   Copyright (C) 2005-2008 Jean Delvare <jdelvare@suse.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef PHY_SLOT_H_
#define PHY_SLOT_H_
#include <stdint.h>


struct pci_bus_addr{
	uint16_t domain;
	uint8_t bus;
	uint8_t dev;
	uint8_t fn;
};

struct locn_bus_pair{
	struct pci_bus_addr bus_addr;
    char *locn;
	struct locn_bus_pair *next;
};


int pci_fill_locn_map(struct locn_bus_pair **head);
#endif