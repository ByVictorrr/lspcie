struct loc_pair{
    char *pci_addr;
    char *loc;
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

	if (!(opt.flags & FLAG_DUMP))
	{
		/* ASCII filtering */
		len = strlen(bp);
		for (i = 0; i < len; i++)
			if (bp[i] < 32 || bp[i] == 127)
				bp[i] = '.';
	}

	return bp;
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
	/* Case 1 - */
	/*
	 * First try reading from sysfs tables.  The entry point file could
	 * contain one of several types of entry points, so read enough for
	 * the largest one, then determine what type it contains.
	 */
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
	

}