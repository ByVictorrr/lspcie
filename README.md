# lspcie
An enhancement of lspci

## Added Options
Sometimes an IO tester might want to look at some information quickly. Here are some options that were added to lspci to do so.

### Table view of IO cards -T
* Shows information about IO cards in a tabular form, columns of the table include:
    * PCI address, Slot #, Card info, Vendor name, and Driver name
    
![single T](https://i.imgur.com/Eh1fRc6.png)

### Json view of IO cards -j
* Shows information IO cards in a json object format, such attributes of each json object include:
    * PCI address, Slot #, Card info, Vendor name, and Driver name
    
![single j](https://i.imgur.com/znymfaF.png)

### Mixing -v with -T or -j
* This will tell the program you want to use the smbios/dmi to get the slot number, instead of the default way.
![ single T with v ](https://i.imgur.com/ofyFeB6.png)
![ single j with v ](https://i.imgur.com/2xEmztN.png)

### Mulitple -T's or -j's
* Lets say n is the number of T's or j's:
    * For n=1, each entry should contain PCI address, Slot #, Card info, Vendor name, and Driver name

    * For n=2, each entry should have the same attributes as n=1, but with an additonal field called Device info.
 
    * For n=3, each entry should have the same attributes as n=2, but with additonal fields: vendor_id and device_id

    * For n=4, each entry should have the same attributes as n=3, but with the driver versions if it exists

    * For n=5, each entry should have the same attributes as n=4, but with the fimrware versions if it exists

    * For n=6, each entry should have the same attributes as n=5, but with the option rom versions if it exists

![ six js ](https://i.imgur.com/iaCE0hR.png)

![ six Ts ](https://i.imgur.com/eqeYZxb.png)
    
## How is the version information pull?
* Each PCI device has its own folder in sysfs - /sys/bus/pci/$PCI_ADDR (device folder)
* Using a regex(sysfs-class.h), it is possible to match certain files (called version files).
   * These files usually contain the version information.
## Why isnt the version information showing?
* There couble be multiple reasons, one is that your PCI class of devices is not in the pcm_vers_map (in sysfs-class.c). 
### How to include a missing PCI class in lspcie
* First off you will be working in lib so every file talked about is in that folder.
* In sysfs-class.c
    1. Create a pci_class_method structure for the class your setting up
    2. Use Macros to guard specific functions in the structure like:
       *  <CLASS_ABBREVIATION>_READ_[DR|FW|OPT]V
    3. Create functions to read the different versions ( driver, firmware, optrom )
    4. Insert your structure address into the pcm_vers_map at the right class index
* Another reason is that the devices driver does not support these files.
* Finally, the regex might need some fine tunning.






