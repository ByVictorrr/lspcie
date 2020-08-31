# lspcie
An enhancement of lspci

## Added Options
Sometimes a IO tester might want to look at some information quickly. Here are some options that was added to lspci to do so.

## IO card dipslay table format (-T[...[T]] options)
### -T 
* Shows information about the IO card such as:
    * PCI address, Slot #, Card info, Vendor name, and Driver

### -TT
* Shows all of the same data as -T, but adds on by showing The device info

### -TTT
* Same information as -TT, buts shows the device number info.
* The device number info is just the Vendor ID and the Device ID as one string

### -TTTT (Version information)
* Exceeding four T's allows one to see the firmware and driver version. With the rest of information explained above.

#### -TTTTT
* Finally, this extends -TTTT, but with capabilites of seeing the opt rom versions

#### -vT...T
* Mixing the verbose flag invokes a new way to view slot numbers. Lspcie simply reads from the smbios to extract the data.

## How is the version information pull?
* Using sysfs, using regex's to pull certain files sysfs.c, sysfs-utils.c, sysfs-class.c
## Why isnt me version information showing?
* Two reasons, the regex for your device folder/file isnt correct or there are no version files that exist.

### How to change the regex so my 
## How to add on to -T...T
* Well of course everything that deals with these options is in files:
    * lspcie.c, ls-table.c, lib/access.c, lib/sysfs.c, lib/sysfs-class.{h, c}, lib/syfs-class-utils.c





