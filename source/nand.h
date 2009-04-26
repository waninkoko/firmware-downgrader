#ifndef _NAND_H_
#define _NAND_H_

/* Prototypes */
s32 Nand_Init(void);
s32 Nand_CreateDir(u64);
s32 Nand_CreateFile(u64, const char *);
s32 Nand_OpenFile(u64, const char *, u8);
s32 Nand_CloseFile(s8);
s32 Nand_ReadFile(s32, void *, u32);
s32 Nand_WriteFile(s32, void *, u32);
s32 Nand_RemoveDir(u64);

#endif
