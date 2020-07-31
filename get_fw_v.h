#ifndef GET_FW_V
#define GET_FW_V

#include <stdint.h>
#include "lspci.h"


/* =========This is for x86 processors==========*/
/* OFFSETS (in bytes) */
#define ROM_SIG_OFFSET 0
#define ROM_LEN_OFFSET 2
#define ROM_INITVEC_OFFSET 3
#define ROM_RES_OFFSET 7

/*=== For pnp devices only ====*/
#define ROM_EXPANOFFSET_OFFSET 0x1a


/* Lengths (in bytes) */
#define ROM_SIG_LEN 2 
#define ROM_LEN_LEN 1
#define ROM_INITVEC_LEN 4
#define ROM_RES_LEN  13
/*=== For pnp only ====*/
#define ROM_EXPANOFFSET_LEN  2

/* Signatures */
#define STD_SIG 0xaa55
/* pnp devices only */
#define PNP_SIG "$PnP"

/* Expansion header for pnp devices */


#define EROM_BLK 2 /* 2 bytes/offset */


struct erom_hdr{
    uint16_t sig[ROM_SIG_LEN];
    uint16_t length[ROM_LEN_LEN];
    uint16_t init[ROM_INITVEC_LEN];
    uint16_t res[ROM_RES_LEN];
    uint16_t offset[ROM_EXPANOFFSET_LEN]; /* only for pnp */
};
struct erom_pnp_hdr{
    struct erom_hdr hdr;
    uint16_t offset[ROM_EXPANOFFSET_LEN]; /* only for pnp */
};
#define SIG_DATA_LEN 4
#define VENID_DATA_LEN 2
#define DEVID_DATA_LEN 2
#define PTRVPD_DATA_LEN 2
#define PCISTRUCTLEN_DATA_LEN 2
#define PCISTRUCTREV_DATA_LEN 1
#define CLASSCODE_DATA_LEN 3
#define IMAGELEN_DATA_LEN 2
#define REVCODE_DATA_LEN 2
#define CODETYPE_DATA_LEN 1
#define INDIC_DATA_LEN 1
#define RES_DATA_LEN 2
/* https://sites.google.com/site/pinczakko/low-cost-embedded-x86-teaching-tool-2 (2.3.2.1.2)*/
struct erom_data{
    uint8_t sig[SIG_DATA_LEN];
    uint8_t venid[VENID_DATA_LEN];
    uint8_t devid[DEVID_DATA_LEN];
    uint8_t ptr_vpd[PTRVPD_DATA_LEN];
    uint8_t pcistruct_len[PCISTRUCTLEN_DATA_LEN];
    uint8_t pcistruct_rev[PCISTRUCTREV_DATA_LEN];
    uint8_t class_code[CLASSCODE_DATA_LEN];
    uint8_t img_len[IMAGELEN_DATA_LEN];
    uint8_t rev_code[REVCODE_DATA_LEN];
    uint8_t code_type[CODETYPE_DATA_LEN];
    uint8_t indic[INDIC_DATA_LEN];
    uint8_t res[RES_DATA_LEN];
};

void get_fw_v(struct device *first_d);
#endif