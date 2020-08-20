#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "config.h"
#include "types.h"
#include "util.h"
#include "phy_slot.h"

#ifdef __FreeBSD__
#include <errno.h>
#include <kenv.h>
#endif

#define out_of_spec "<OUT OF SPEC>"
static const char *bad_index = "<BAD INDEX>";
#define SUPPORTED_SMBIOS_VER 0x030200
#define FLAG_NO_FILE_OFFSET     (1 << 0)
#define FLAG_STOP_AT_EOT        (1 << 1)
#define SYS_FIRMWARE_DIR "/sys/firmware/dmi/tables"
#define SYS_ENTRY_FILE SYS_FIRMWARE_DIR "/smbios_entry_point"
#define SYS_TABLE_FILE SYS_FIRMWARE_DIR "/DMI"

struct pci_bus_addr{
	u16 domain;
	u8 bus;
	u8 dev;
	u8 fn;
};
struct locn_bus_pair{
	struct pci_bus_addr bus_addr;
    char *locn;
	struct locn_bus_pair *next;
};



const char *dmi_string(const struct dmi_header *dm, u8 s)
{
	char *bp = (char *)dm->data;
	size_t i, len;

	if (s == 0)
		return "Not Specified";

	bp += dm->length;
	while (s > 1 && *bp)
	{
		bp += strlen(bp);
		bp++;
		s--;
	}

	if (!*bp)
		return bad_index;

	/* ASCII filtering */
	len = strlen(bp);
	for (i = 0; i < len; i++)
		if (bp[i] < 32 || bp[i] == 127)
			bp[i] = '.';

	return bp;
}
static int dmi_slot_segment_bus_func(u16 code1, u8 code2, u8 code3, struct pci_bus_addr *bus_addr)
{
	/* 7.10.8 */
	if (!(code1 == 0xFFFF && code2 == 0xFF && code3 == 0xFF)){
		bus_addr->domain = code1;
		bus_addr->bus = code2;
		bus_addr->dev = code3 >> 3;
		bus_addr->fn = code3 & 0x7;
		return 1;
	}
	return 0;
}

static void to_dmi_header(struct dmi_header *h, u8 *data)
{
	h->type = data[0];
	h->length = data[1];
	h->handle = WORD(data + 2);
	h->data = data;
}

static void dmi_decode(const struct dmi_header *h, struct locn_bus_pair *item)
{
	const u8 *data = h->data;

	if (h->length < 0x0C)
		return;
	// Step 1 - set the locn (r*)
	item->locn = dmi_string(h, data[0x04]); 
	if (h->length < 0x11)
		return;
	// Step 2 - set bus addr
	dmi_slot_segment_bus_func(WORD(data + 0x0D), data[0x0F], data[0x10], &item->bus_addr);
}


static void dmi_table_decode(u8 *buf, u32 len, u16 num,u32 flags, struct locn_bus_pair **head)
{
	
	struct locn_bus_pair *curr;
	u8 *data;
	int i = 0;

	data = buf;
	while ((i < num || !num)
	    && data + 4 <= buf + len) /* 4 is the length of an SMBIOS structure header */
	{
		u8 *next;
		struct dmi_header h;

		to_dmi_header(&h, data);

		/*
		 * If a short entry is found (less than 4 bytes), not only it
		 * is invalid, but we cannot reliably locate the next entry.
		 * Better stop at this point, and let the user know his/her
		 * table is broken.
		 */
		if (h.length < 4)
		{
				fprintf(stderr,
					"Invalid entry length (%u). DMI table "
					"is broken! Stop.\n\n",
					(unsigned int)h.length);
			break;
		}
		i++;


		/* Look for the next handle */
		next = data + h.length;
		while ((unsigned long)(next - buf + 1) < len
		    && (next[0] != 0 || next[1] != 0))
			next++;
		next += 2;

		/* Make sure the whole structure fits in the table */
		if ((unsigned long)(next - buf) > len)
			break;

		if (h.type == 9){
			if(!*head){
				if(!(*head=curr=calloc(1, sizeof(struct locn_bus_pair)))){
					perror("calloc");
					return;
				}
			}else{
				if(!(curr->next=calloc(1, sizeof(struct locn_bus_pair)))){
					perror("calloc");
					return;
				}
				curr=curr->next;
			}
			dmi_decode(&h, curr);
		}

		data = next;

		/* SMBIOS v3 requires stopping at this marker */
		if (h.type == 127 && (flags & FLAG_STOP_AT_EOT))
			break;
	}

}


static void dmi_table(off_t base, u32 len, u16 num, u32 ver, const char *devmem,
		      u32 flags, struct locn_bus_pair **head)
{
	/* Dependencies */

	u8 *buf;

	if (ver > SUPPORTED_SMBIOS_VER)
	{
		printf("# SMBIOS implementations newer than version %u.%u.%u are not\n"
		       "# fully supported by this version of dmidecode.\n",
		       SUPPORTED_SMBIOS_VER >> 16,
		       (SUPPORTED_SMBIOS_VER >> 8) & 0xFF,
		       SUPPORTED_SMBIOS_VER & 0xFF);
	}


	if (flags & FLAG_NO_FILE_OFFSET) 
	{
		/*
		 * When reading from sysfs or from a dump file, the file may be
		 * shorter than announced. For SMBIOS v3 this is expcted, as we
		 * only know the maximum table size, not the actual table size.
		 * For older implementations (and for SMBIOS v3 too), this
		 * would be the result of the kernel truncating the table on
		 * parse error.
		 */
		size_t size = len;
		buf = read_file(flags & FLAG_NO_FILE_OFFSET ? 0 : base,
			&size, devmem);
		if (num && size != (size_t)len)
		{
			fprintf(stderr, "Wrong DMI structures length: %u bytes "
				"announced, only %lu bytes available.\n",
				len, (unsigned long)size);
		}
		len = size;
	}
	else
		buf = mem_chunk(base, len, devmem);

	if (buf == NULL)
	{
		fprintf(stderr, "Failed to read table, sorry.\n");
#ifndef USE_MMAP
		if (!(flags & FLAG_NO_FILE_OFFSET))
			fprintf(stderr,
				"Try compiling dmidecode with -DUSE_MMAP.\n");
#endif
		return;
	}

	dmi_table_decode(buf, len, num, flags, head);

	free(buf);
}


static int smbios3_decode(u8 *buf, const char *devmem, u32 flags, struct locn_bus_pair **head)
{
	/* Dependencies */
	u32 ver;
	u64 offset;

	/* Don't let checksum run beyond the buffer */
	if (buf[0x06] > 0x20)
	{
		fprintf(stderr,
			"Entry point length too large (%u bytes, expected %u).\n",
			(unsigned int)buf[0x06], 0x18U);
		return 0;
	}

	if (!checksum(buf, buf[0x06]))
		return 0;

	ver = (buf[0x07] << 16) + (buf[0x08] << 8) + buf[0x09];
	printf("SMBIOS %u.%u.%u present.\n",
		       buf[0x07], buf[0x08], buf[0x09]);

	offset = QWORD(buf + 0x10);
	if (!(flags & FLAG_NO_FILE_OFFSET) && offset.h && sizeof(off_t) < 8)
	{
		fprintf(stderr, "64-bit addresses not supported, sorry.\n");
		return 0;
	}

	dmi_table(((off_t)offset.h << 32) | offset.l,
		  DWORD(buf + 0x0C), 0, ver, devmem, flags | FLAG_STOP_AT_EOT, head);

	return 1;
}

static int smbios_decode(u8 *buf, const char *devmem, u32 flags, struct locn_bus_pair **head)
{
	u16 ver;

	/* Don't let checksum run beyond the buffer */
	if (buf[0x05] > 0x20)
	{
		fprintf(stderr,
			"Entry point length too large (%u bytes, expected %u).\n",
			(unsigned int)buf[0x05], 0x1FU);
		return 0;
	}

	if (!checksum(buf, buf[0x05])
	 || memcmp(buf + 0x10, "_DMI_", 5) != 0
	 || !checksum(buf + 0x10, 0x0F))
		return 0;

	ver = (buf[0x06] << 8) + buf[0x07];
	/* Some BIOS report weird SMBIOS version, fix that up */
	switch (ver)
	{
		case 0x021F:
		case 0x0221:
			fprintf(stderr,
				"SMBIOS version fixup (2.%d -> 2.%d).\n",
				ver & 0xFF, 3);
			ver = 0x0203;
			break;
		case 0x0233:
			fprintf(stderr,
				"SMBIOS version fixup (2.%d -> 2.%d).\n",
				51, 6);
			ver = 0x0206;
			break;
	}
	printf("SMBIOS %u.%u present.\n",
			ver >> 8, ver & 0xFF);

	dmi_table(DWORD(buf + 0x18), WORD(buf + 0x16), WORD(buf + 0x1C),
		ver << 8, devmem, flags, head);

	return 1;
}

static int legacy_decode(u8 *buf, const char *devmem, u32 flags, struct locn_bus_pair **head)
{
	if (!checksum(buf, 0x0F))
		return 0;

	printf("Legacy DMI %u.%u present.\n",
		buf[0x0E] >> 4, buf[0x0E] & 0x0F);

	dmi_table(DWORD(buf + 0x08), WORD(buf + 0x06), WORD(buf + 0x0C),
		((buf[0x0E] & 0xF0) << 12) + ((buf[0x0E] & 0x0F) << 8),
		devmem, flags, head);


	return 1;
}


/*
 * Probe for EFI interface
 */
#define EFI_NOT_FOUND   (-1)
#define EFI_NO_SMBIOS   (-2)
static int address_from_efi(off_t *address)
{
#if defined(__linux__)
	FILE *efi_systab;
	const char *filename;
	char linebuf[64];
#elif defined(__FreeBSD__)
	char addrstr[KENV_MVALLEN + 1];
#endif
	const char *eptype;
	int ret;

	*address = 0; /* Prevent compiler warning */

#if defined(__linux__)
	/*
	 * Linux up to 2.6.6: /proc/efi/systab
	 * Linux 2.6.7 and up: /sys/firmware/efi/systab
	 */
	if ((efi_systab = fopen(filename = "/sys/firmware/efi/systab", "r")) == NULL
	 && (efi_systab = fopen(filename = "/proc/efi/systab", "r")) == NULL)
	{
		/* No EFI interface, fallback to memory scan */
		return EFI_NOT_FOUND;
	}
	ret = EFI_NO_SMBIOS;
	while ((fgets(linebuf, sizeof(linebuf) - 1, efi_systab)) != NULL)
	{
		char *addrp = strchr(linebuf, '=');
		*(addrp++) = '\0';
		if (strcmp(linebuf, "SMBIOS3") == 0
		 || strcmp(linebuf, "SMBIOS") == 0)
		{
			*address = strtoull(addrp, NULL, 0);
			eptype = linebuf;
			ret = 0;
			break;
		}
	}
	if (fclose(efi_systab) != 0)
		perror(filename);

	if (ret == EFI_NO_SMBIOS)
		fprintf(stderr, "%s: SMBIOS entry point missing\n", filename);
#elif defined(__FreeBSD__)
	/*
	 * On FreeBSD, SMBIOS anchor base address in UEFI mode is exposed
	 * via kernel environment:
	 * https://svnweb.freebsd.org/base?view=revision&revision=307326
	 */
	ret = kenv(KENV_GET, "hint.smbios.0.mem", addrstr, sizeof(addrstr));
	if (ret == -1)
	{
		if (errno != ENOENT)
			perror("kenv");
		return EFI_NOT_FOUND;
	}

	*address = strtoull(addrstr, NULL, 0);
	eptype = "SMBIOS";
	ret = 0;
#else
	ret = EFI_NOT_FOUND;
#endif

	if (ret == 0)
		printf("# %s entry point at 0x%08llx\n",
		       eptype, (unsigned long long)*address);

	return ret;
}


int main(){
	/* dependencies*/
	int ret = 0;                /* Returned value */
	int found = 0;
	off_t fp;
	size_t size;
	int efi;
	u8 *buf;
	
	/* location pair list */
	struct locn_bus_pair *head;
	const char *devmem;


	/* Case 1 - */
	/*
	 * First try reading from sysfs tables.  The entry point file could
	 * contain one of several types of entry points, so read enough for
	 * the largest one, then determine what type it contains.
	 */
	/* Set default option values */
	devmem = DEFAULT_MEM_DEV;

	size = 0x20;
	if((buf = read_file(0, &size, SYS_ENTRY_FILE)) != NULL)
	{
		printf("Getting SMBIOS data from sysfs.\n");
		if (size >= 24 && memcmp(buf, "_SM3_", 5) == 0)
		{
			if (smbios3_decode(buf, SYS_TABLE_FILE, FLAG_NO_FILE_OFFSET, &head))
				found++;
		}
		else if (size >= 31 && memcmp(buf, "_SM_", 4) == 0)
		{
			if (smbios_decode(buf, SYS_TABLE_FILE, FLAG_NO_FILE_OFFSET, &head))
				found++;
		}
		else if (size >= 15 && memcmp(buf, "_DMI_", 5) == 0)
		{
			if (legacy_decode(buf, SYS_TABLE_FILE, FLAG_NO_FILE_OFFSET, &head))
				found++;
		}

		if (found)
			goto done;
		printf("Failed to get SMBIOS data from sysfs.\n");
	}
/* Next try EFI (ia64, Intel-based Mac) */
	efi = address_from_efi(&fp);
	switch (efi)
	{
		case EFI_NOT_FOUND:
			goto memory_scan;
		case EFI_NO_SMBIOS:
			ret = 1;
			goto exit_free;
	}

	printf("Found SMBIOS entry point in EFI, reading table from %s.\n",
		       devmem);
	if ((buf = mem_chunk(fp, 0x20, devmem)) == NULL)
	{
		ret = 1;
		goto exit_free;
	}

	if (memcmp(buf, "_SM3_", 5) == 0)
	{
		if (smbios3_decode(buf, devmem, 0, &head))
			found++;
	}
	else if (memcmp(buf, "_SM_", 4) == 0)
	{
		if (smbios_decode(buf, devmem, 0, &head))
			found++;
	}
	goto done;

memory_scan:
	printf("Scanning %s for entry point.\n", devmem);
	/* Fallback to memory scan (x86, x86_64) */
	if ((buf = mem_chunk(0xF0000, 0x10000, devmem)) == NULL)
	{
		ret = 1;
		goto exit_free;
	}

	/* Look for a 64-bit entry point first */
	for (fp = 0; fp <= 0xFFE0; fp += 16)
	{
		if (memcmp(buf + fp, "_SM3_", 5) == 0)
		{
			if (smbios3_decode(buf + fp, devmem, 0, &head))
			{
				found++;
				goto done;
			}
		}
	}

	/* If none found, look for a 32-bit entry point */
	for (fp = 0; fp <= 0xFFF0; fp += 16)
	{
		if (memcmp(buf + fp, "_SM_", 4) == 0 && fp <= 0xFFE0)
		{
			if (smbios_decode(buf + fp, devmem, 0, &head))
			{
				found++;
				goto done;
			}
		}
		else if (memcmp(buf + fp, "_DMI_", 5) == 0)
		{
			if (legacy_decode(buf + fp, devmem, 0, &head))
			{
				found++;
				goto done;
			}
		}
	}

done:
	if (!found )
		printf("# No SMBIOS nor DMI entry point found, sorry.\n");

	free(buf);
exit_free:
	
	return ret;
}