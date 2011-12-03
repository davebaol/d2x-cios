#ifndef _ES_H_
#define _ES_H_

#include "types.h"


/* Constants */
#define ES_SIG_RSA4096		0x10000
#define ES_SIG_RSA2048		0x10001

#define TIK_SIZE		676


/* RSA (2048 bits) signature structure */
typedef struct {
	u32 type;
	u8 sig[256];
	u8 fill[60];
} ATTRIBUTE_PACKED sig_rsa2048;

/* RSA (4096 bits) signature structure */
typedef struct {
	u32 type;
	u8  sig[512];
	u8  fill[60];
} ATTRIBUTE_PACKED sig_rsa4096;

/* Ticket limit structure */
typedef struct {
	u32 tag;
	u32 value;
} ATTRIBUTE_PACKED tiklimit;

/* Ticket view structure */
typedef struct {
	u32 view;
	u64 ticketid;
	u32 devicetype;
	u64 titleid;
	u16 access_mask;
	u8  reserved[0x3c];
	u8  cidx_mask[0x40];
	u16 padding;
	tiklimit limits[8];
} ATTRIBUTE_PACKED tikview;

/* Ticket structure */
typedef struct {
	char issuer[0x40];
	u8  fill[63];
	u8  cipher_title_key[16];
	u8  fill2;
	u64 ticketid;
	u32 devicetype;
	u64 titleid;
	u16 access_mask;
	u8  reserved[0x3c];
	u8  cidx_mask[0x40];
	u16 padding;
	tiklimit limits[8];
} ATTRIBUTE_PACKED tik;

/* TMD content structure */
typedef struct {
	u32 cid;
	u16 index;
	u16 type;
	u64 size;
	u8 hash[20];
} ATTRIBUTE_PACKED tmd_content;

/* TMD structure */
typedef struct {
	char issuer[0x40];
	u8 version;
	u8 ca_crl_version;
	u8 signer_crl_version;
	u8 fill2;
	u64 sys_version;
	u64 title_id;
	u32 title_type;
	u16 group_id;
	u16 zero;
	u16 region;
	u8 ratings[16];
	u8 reserved[12];
	u8 ipc_mask[12];
	u8 reserved2[18];
	u32 access_rights;
	u16 title_version;
	u16 num_contents;
	u16 boot_index;
	u16 fill3;
	tmd_content contents[];
} ATTRIBUTE_PACKED tmd;


/* Macros */
#define SIGNATURE_SIZE(x) (\
	((*(x))==ES_SIG_RSA2048) ? sizeof(sig_rsa2048) : ( \
	((*(x))==ES_SIG_RSA4096) ? sizeof(sig_rsa4096) : 0 ))

#define SIGNATURE_PAYLOAD(x) ((void *)(((u8*)(x)) + SIGNATURE_SIZE(x)))

 
#endif
