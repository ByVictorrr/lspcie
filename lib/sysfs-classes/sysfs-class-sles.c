#include <stdio.h> // NULL declaration
#include "vendors.h"
#include "sysfs-class-sles.h"

/* README: How to update a device 
    In order to update a device, setup a macro
    #define $(DEVICE_NAME)_{FW,DR}V_FILE_PATTN
    This tells ../sysfs-class.h that the function exists,
    so the corresponding pci_class_method structure can
    invoke the function  
*/

/* Class 0x01 */

/*===========SAS ====================================*/
char *sas_drv_file_pattns[PCI_VENDOR_MAX] = {
    [ADAPTEC_VENID] = "version*"
};
char *sas_fwv_file_pattns[PCI_VENDOR_MAX] = {
    [ADAPTEC_VENID] = "version*"
};

/*=================================================*/
/*=================FC================*/

char *fc_drv_file_pattns[PCI_VENDOR_MAX] = {
    [QLOGIC_VENID] = "driver_version*",
    [EMULEX_VENID] = "lpfc_drvr_version*"
};

char *fc_fwv_file_pattns[PCI_VENDOR_MAX] = {
    [QLOGIC_VENID] = "(option_rom|option_)*",
    [EMULEX_VENID] = "option_rom*"
};