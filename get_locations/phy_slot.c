struct loc_pair{
	struct pci_bus_addr bus_addr;
    char *loc;
	struct loc_pair *next;
};
struct pci_bus_addr{
	u16 domain;
	u8 bus;
	u8 dev;
	u8 fn;
}

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

static void dmi_decode(const struct dmi_header *h, struct )
{
	const u8 *data = h->data;

	switch (h->type)
	{
		case 9: /* 7.10 System Slots */
			if (h->length < 0x0C) break;
			printf("\tDesignation: %s\n",
				dmi_string(h, data[0x04])); // r*
			if (h->length < 0x11) break;
			dmi_slot_segment_bus_func(WORD(data + 0x0D), data[0x0F], data[0x10], );
			break;
}

static void to_dmi_header(struct dmi_header *h, u8 *data)
{
	h->type = data[0];
	h->length = data[1];
	h->handle = WORD(data + 2);
	h->data = data;
}


static void dmi_table_decode(u8 *buf, u32 len, u16 num, u32 flags)
{
	/* Dependencies */
	/* dmidecode.c */
	static void to_dmi_header(struct dmi_header *h, u8 *data);
	static void dmi_decode(const struct dmi_header *h, u16 ver);
	

	u8 *data;
	int i = 0;

	data = buf;
	while ((i < num || !num)
	    && data + 4 <= buf + len) /* 4 is the length of an SMBIOS structure header */
	{
		u8 *next;
		struct dmi_header h;
		int display;

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

		/* Fixup a common mistake */
		if (h.type == 34)
			dmi_fixup_type_34(&h, display);

		if (h.type == 9)
			dmi_decode(&h);

		data = next;

		/* SMBIOS v3 requires stopping at this marker */
		if (h.type == 127 && (flags & FLAG_STOP_AT_EOT))
			break;
	}

}


static void dmi_table(off_t base, u32 len, u16 num, u32 ver, const char *devmem,
		      u32 flags)
{
	/* Dependencies */
	/* util.h */
	void *read_file(off_t base, size_t *len, const char *filename);
	void *mem_chunk(off_t base, size_t len, const char *devmem);

	/* dmidecode.c*/
    static void dmi_table_decode(u8 *buf, u32 len, u16 num, u16 ver, u32 flags);

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

	dmi_table_decode(buf, len, num, ver >> 8, flags);

	free(buf);
}


static int smbios3_decode(u8 *buf, const char *devmem, u32 flags)
{
	/* Dependencies */
	static void dmi_table(off_t base, u32 len, u16 num, u32 ver, const char *devmem, u32 flags);
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
		  DWORD(buf + 0x0C), 0, ver, devmem, flags | FLAG_STOP_AT_EOT);

	return 1;
}



struct loc_pair *loc_pairget_loc_map(const struct dmi_header *h){
 case 9: /* 7.10 System Slots */
             if (h->length < 0x0C) break;
             printf("\tDesignation: %s\n",
                 dmi_string(h, data[0x04]));
             if (h->length < 0x11) break;
             dmi_slot_segment_bus_func(WORD(data + 0x0D), data[0x0F], data[0x10], "\t");
} 
int main(){
	/* dependencies*/
	int ret = 0;                /* Returned value */
	int found = 0;
	off_t fp;
	size_t size;
	int efi;
	u8 *buf;
	
	/* dmidecode.h */
	static int smbios3_decode(u8 *buf, const char *devmem, u32 flags);
	static int smbios_decode(u8 *buf, const char *devmem, u32 flags);
	static int legacy_decode(u8 *buf, const char *devmem, u32 flags);
	static int address_from_efi(off_t *address);

	/* util.h */
	void *mem_chunk(off_t base, size_t len, const char *devmem);
	void *read_file(off_t base, size_t *max_len, const char *filename);


	/* Case 1 - */
	/*
	 * First try reading from sysfs tables.  The entry point file could
	 * contain one of several types of entry points, so read enough for
	 * the largest one, then determine what type it contains.
	 */
	/* Set default option values */
	opt.devmem = DEFAULT_MEM_DEV;
	opt.flags = 0;
	opt.handle = ~0U;

	if (parse_command_line(argc, argv)<0)
	{
		ret = 2;
		goto exit_free;
	}
	size = 0x20;

	if (!(opt.flags & FLAG_NO_SYSFS)
	 && (buf = read_file(0, &size, SYS_ENTRY_FILE)) != NULL)
	{
		if (!(opt.flags & FLAG_QUIET))
			printf("Getting SMBIOS data from sysfs.\n");
		if (size >= 24 && memcmp(buf, "_SM3_", 5) == 0)
		{
			if (smbios3_decode(buf, SYS_TABLE_FILE, FLAG_NO_FILE_OFFSET))
				found++;
		}
		else if (size >= 31 && memcmp(buf, "_SM_", 4) == 0)
		{
			if (smbios_decode(buf, SYS_TABLE_FILE, FLAG_NO_FILE_OFFSET))
				found++;
		}
		else if (size >= 15 && memcmp(buf, "_DMI_", 5) == 0)
		{
			if (legacy_decode(buf, SYS_TABLE_FILE, FLAG_NO_FILE_OFFSET))
				found++;
		}

		if (found)
			goto done;
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

	if (!(opt.flags & FLAG_QUIET))
		printf("Found SMBIOS entry point in EFI, reading table from %s.\n",
		       opt.devmem);
	if ((buf = mem_chunk(fp, 0x20, opt.devmem)) == NULL)
	{
		ret = 1;
		goto exit_free;
	}

	if (memcmp(buf, "_SM3_", 5) == 0)
	{
		if (smbios3_decode(buf, opt.devmem, 0))
			found++;
	}
	else if (memcmp(buf, "_SM_", 4) == 0)
	{
		if (smbios_decode(buf, opt.devmem, 0))
			found++;
	}
	goto done;

memory_scan:
	if (!(opt.flags & FLAG_QUIET))
		printf("Scanning %s for entry point.\n", opt.devmem);
	/* Fallback to memory scan (x86, x86_64) */
	if ((buf = mem_chunk(0xF0000, 0x10000, opt.devmem)) == NULL)
	{
		ret = 1;
		goto exit_free;
	}

	/* Look for a 64-bit entry point first */
	for (fp = 0; fp <= 0xFFE0; fp += 16)
	{
		if (memcmp(buf + fp, "_SM3_", 5) == 0)
		{
			if (smbios3_decode(buf + fp, opt.devmem, 0))
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
			if (smbios_decode(buf + fp, opt.devmem, 0))
			{
				found++;
				goto done;
			}
		}
		else if (memcmp(buf + fp, "_DMI_", 5) == 0)
		{
			if (legacy_decode(buf + fp, opt.devmem, 0))
			{
				found++;
				goto done;
			}
		}
	}

done:
	if (!found && !(opt.flags & FLAG_QUIET))
		printf("# No SMBIOS nor DMI entry point found, sorry.\n");

	free(buf);
exit_free:
	free(opt.type);
	
	return ret;
}