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

void get_fw_v(struct device *first_d);
#endif