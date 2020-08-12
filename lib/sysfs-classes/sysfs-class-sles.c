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
    [ADAPTEC_VENID] = "version"
};
char *sas_fwv_file_pattns[PCI_VENDOR_MAX] = {
    [ADAPTEC_VENID] = "version"
};

char *nvm_fwv_file_pattns[PCI_VENDOR_MAX] = {
    [INTEL_VENID] = "firmware_rev" 
};
/*=================================================*/
/*=================FC================*/

char *fc_drv_file_pattns[PCI_VENDOR_MAX] = {
    [QLOGIC_VENID] = "driver_version",
    [BROADCOM_VENID] = "lpfc_drvr_version"
};

char *fc_fwv_file_pattns[PCI_VENDOR_MAX] = {
    [QLOGIC_VENID] = "(.{0,}fw_version|optrom_.{1,}_version)",
    [BROADCOM_VENID] = "option_rom_version"
};