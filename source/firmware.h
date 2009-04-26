#ifndef _FIRMWARE_H_
#define _FIRMWARE_H_

/* Constants */
enum {
	FW_REGION_JAP = 0,
	FW_REGION_USA,
	FW_REGION_EUR
};

/* "firmware" structure */
struct firmware {
	char version[16];

	/* Firmware region */
	u8 region;

	/* System Menu info */
	u64 sysTid;
	u16 sysVersion;

	/* IOS info */
	u64 iosTid;
	u16 iosVersion;

	/* MIOS info */
	u64 miosTid;
	u16 miosVersion;

	/* BC info */
	u64 bcTid;
	u16 bcVersion;
};

/* Prototypes */
void Firmware_Install(struct firmware *);

#endif

