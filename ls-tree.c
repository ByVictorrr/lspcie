/*
 *	The PCI Utilities -- Show Bus Tree
 *
 *	Copyright (c) 1997--2020 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "lspci.h"

struct bridge host_bridge = { NULL, NULL, NULL, NULL, 0, ~0, 0, ~0, NULL };

static struct bus *
find_bus(struct bridge *b, unsigned int domain, unsigned int n)
{
  struct bus *bus;

  for (bus=b->first_bus; bus; bus=bus->sibling)
    if (bus->domain == domain && bus->number == n)
      break;
  return bus;
}

static struct bus *
new_bus(struct bridge *b, unsigned int domain, unsigned int n)
{
  struct bus *bus = xmalloc(sizeof(struct bus));
  bus->domain = domain;
  bus->number = n;
  bus->sibling = b->first_bus;
  bus->first_dev = NULL;
  bus->last_dev = &bus->first_dev;
  bus->parent_bridge = b;
  b->first_bus = bus;
  return bus;
}

static void
insert_dev(struct device *d, struct bridge *b)
{
  struct pci_dev *p = d->dev;
  struct bus *bus;

  if (! (bus = find_bus(b, p->domain, p->bus)))
    {
      struct bridge *c;
      for (c=b->child; c; c=c->next)
	if (c->domain == (unsigned)p->domain && c->secondary <= p->bus && p->bus <= c->subordinate)
          {
            insert_dev(d, c);
            return;
          }
      bus = new_bus(b, p->domain, p->bus);
    }
  /* Simple insertion at the end _does_ guarantee the correct order as the
   * original device list was sorted by (domain, bus, devfn) lexicographically
   * and all devices on the new list have the same bus number.
   */
  *bus->last_dev = d;
  bus->last_dev = &d->bus_next;
  d->bus_next = NULL;
  d->parent_bus = bus;
}
void
delete_tree(void){
  struct device *d, *next;
  struct bridge *br;
  struct bus *bu;

  for(d=first_dev; d; d=d->next){
    if((br = d->bridge)){
      if ((bu=find_bus(br, br->domain, br->secondary)))
        free(bu);
      free(br);
    }
  }
}

void
grow_tree(void)
{
  struct device *d;
  struct bridge **last_br, *b;
  struct pci_dev *p;
  /* Build list of bridges */

  last_br = &host_bridge.chain;
  for (d=first_dev; d; d=d->next)
    {
      struct pci_dev *dd = d->dev;
      word class = dd->device_class;
      byte ht = get_conf_byte(d, PCI_HEADER_TYPE) & 0x7f;
    

      if ((class >> 8) == PCI_BASE_CLASS_BRIDGE &&
	  (ht == PCI_HEADER_TYPE_BRIDGE || ht == PCI_HEADER_TYPE_CARDBUS))
	{
	  b = xmalloc(sizeof(struct bridge));
	  b->domain = dd->domain;
	  if (ht == PCI_HEADER_TYPE_BRIDGE)
	    {
	      b->primary = get_conf_byte(d, PCI_PRIMARY_BUS);
	      b->secondary = get_conf_byte(d, PCI_SECONDARY_BUS);
	      b->subordinate = get_conf_byte(d, PCI_SUBORDINATE_BUS);
	    }
	  else
	    {
	      b->primary = get_conf_byte(d, PCI_CB_PRIMARY_BUS);
	      b->secondary = get_conf_byte(d, PCI_CB_CARD_BUS);
	      b->subordinate = get_conf_byte(d, PCI_CB_SUBORDINATE_BUS);
	    }
  
	  *last_br = b;
	  last_br = &b->chain;
	  b->next = b->child = NULL;
	  b->first_bus = NULL;
	  b->br_dev = d;
	  d->bridge = b;
	  pacc->debug("Tree: bridge %04x:%02x:%02x.%d: %02x -> %02x-%02x\n",
	    dd->domain, dd->bus, dd->dev, dd->func,
	    b->primary, b->secondary, b->subordinate);
	}
    }
  *last_br = NULL;

  /* Create a bridge tree */

  for (b=&host_bridge; b; b=b->chain)
    {
      struct bridge *c, *best;
      best = NULL;
      for (c=&host_bridge; c; c=c->chain)
	if (c != b && (c == &host_bridge || b->domain == c->domain) &&
	    b->primary >= c->secondary && b->primary <= c->subordinate &&
	    (!best || best->subordinate - best->primary > c->subordinate - c->primary))
	  best = c;
      if (best)
	{
	  b->next = best->child;
	  best->child = b;
	}
    }

  /* Insert secondary bus for each bridge */

  for (b=&host_bridge; b; b=b->chain)
    if (!find_bus(b, b->domain, b->secondary))
      new_bus(b, b->domain, b->secondary);

  /* Create bus structs and link devices */

  for (d=first_dev; d; d=d->next)
    insert_dev(d, &host_bridge);
}

static void
print_it(char *line, char *p)
{
  *p++ = '\n';
  *p = 0;
  fputs(line, stdout);
  for (p=line; *p; p++)
    if (*p == '+' || *p == '|')
      *p = '|';
    else
      *p = ' ';
}

static void show_tree_bridge(struct bridge *, char *, char *);

#define LINE_BUF_SIZE 1024

static char * FORMAT_CHECK(printf, 3, 4)
tree_printf(char *line, char *p, char *fmt, ...)
{
  va_list args;
  char *end = line + LINE_BUF_SIZE - 2;

  if (p >= end)
    return p;

  va_start(args, fmt);
  int res = vsnprintf(p, end - p, fmt, args);
  if (res < 0)
    {
      /* Ancient C libraries return -1 on overflow */
      p += strlen(p);
    }
  else
    p += res;

  va_end(args);
  return p;
}

static void
show_tree_dev(struct device *d, char *line, char *p)
{
  struct pci_dev *q = d->dev;
  struct bridge *b;
  char namebuf[256];

  p = tree_printf(line, p, "%02x.%x", q->dev, q->func);
  for (b=&host_bridge; b; b=b->chain)
    if (b->br_dev == d)
      {
	if (b->secondary == b->subordinate)
	  p = tree_printf(line, p, "-[%02x]-", b->secondary);
	else
	  p = tree_printf(line, p, "-[%02x-%02x]-", b->secondary, b->subordinate);
        show_tree_bridge(b, line, p);
        return;
      }
  if (verbose)
    p = tree_printf(line, p, "  %s",
		    pci_lookup_name(pacc, namebuf, sizeof(namebuf),
				    PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE,
				    q->vendor_id, q->device_id));
  print_it(line, p);
}

static void
show_tree_bus(struct bus *b, char *line, char *p)
{
  if (!b->first_dev)
    print_it(line, p);
  else if (!b->first_dev->bus_next)
    {
      p = tree_printf(line, p, "--");
      show_tree_dev(b->first_dev, line, p);
    }
  else
    {
      struct device *d = b->first_dev;
      while (d->bus_next)
	{
	  char *p2 = tree_printf(line, p, "+-");
	  show_tree_dev(d, line, p2);
	  d = d->bus_next;
	}
      p = tree_printf(line, p, "\\-");
      show_tree_dev(d, line, p);
    }
}

static void
show_tree_bridge(struct bridge *b, char *line, char *p)
{
  *p++ = '-';
  if (!b->first_bus->sibling)
    {
      if (b == &host_bridge)
        p = tree_printf(line, p, "[%04x:%02x]-", b->domain, b->first_bus->number);
      show_tree_bus(b->first_bus, line, p);
    }
  else
    {
      struct bus *u = b->first_bus;
      char *k;

      while (u->sibling)
        {
          k = tree_printf(line, p, "+-[%04x:%02x]-", u->domain, u->number);
          show_tree_bus(u, line, k);
          u = u->sibling;
        }
      k = tree_printf(line, p, "\\-[%04x:%02x]-", u->domain, u->number);
      show_tree_bus(u, line, k);
    }
}

void
show_forest(struct pci_filter *filter)
{
  char line[LINE_BUF_SIZE];
  if (filter == NULL)
    show_tree_bridge(&host_bridge, line, line);
  else
    {
      struct bridge *b;
      for (b=&host_bridge; b; b=b->chain)
        {
          if (b->br_dev && pci_filter_match(filter, b->br_dev->dev))
            {
                struct pci_dev *d = b->br_dev->dev;
                char *p = line;
                p = tree_printf(line, p, "%04x:%02x:", d->domain_16, d->bus);
                show_tree_dev(b->br_dev, line, p);
            }
        }
    }
}
