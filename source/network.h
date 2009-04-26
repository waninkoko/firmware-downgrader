#ifndef _NETWORK_H_
#define _NETWORK_H_

/* Prototypes */
char *Network_GetIP(void);

s32 Network_Init(void);
s32 Network_Connect(void);
s32 Network_Request(const char *, u32 *);
s32 Network_Read(void *, u32);
s32 Network_Write(void *, u32);

#endif
