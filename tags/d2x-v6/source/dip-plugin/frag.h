
#define FRAG_MAX 20000

typedef struct
{
	u32 offset; // file offset, in sectors unit
	u32 sector;
	u32 count;
} Fragment;

typedef struct
{
	u32 size; // num sectors
	u32 num;  // num fragments
	u32 maxnum;
	Fragment frag[FRAG_MAX];
} FragList;

extern FragList fraglist_data;
extern FragList *frag_list;

int set_frag_list(FragList *p, int size);

// in case a sparse block is requested,
// the returned poffset might not be equal to requested offset
// the difference should be filled with 0
int frag_get(FragList *ff, u32 offset, u32 count,
		u32 *poffset, u32 *psector, u32 *pcount);

// woffset is pointing 32bit words to address the whole dvd, len is in bytes
int frag_read(u32 woffset, u8 *data, u32 len);

s32 Frag_Init(u32 device, void *fraglist, int size);
void Frag_Close(void);
s32 Frag_Read(void *data, u32 len, u32 woffset);

