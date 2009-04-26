#include <stdio.h>
#include <string.h>
#include <ogcsys.h>

#include "firmware.h"
#include "sysconf.h"
#include "video.h"
#include "wpad.h"

/* Variables */
static const char setting_fs[] ATTRIBUTE_ALIGN(32) = "/title/00000001/00000002/data/setting.txt";


s32 __Setting_SetPerms(void)
{
	u32 ownerID;
	u16 groupID;
	u8  attributes, ownerperm, groupperm, otherperm;

	s32 ret;

	/* Get current setting file permissions */
	ret = ISFS_GetAttr(setting_fs, &ownerID, &groupID, &attributes, &ownerperm, &groupperm, &otherperm);
	if (ret < 0)
		return ret;

	/* Change owner permissions */
	ownerperm = ISFS_OPEN_RW;

	/* Set new setting file permissions */
	ret = ISFS_SetAttr(setting_fs, ownerID, groupID, attributes, ownerperm, groupperm, otherperm);
	if (ret < 0)
		return ret;

	return 0;
}


s32 Setting_GetCountry(u8 *code)
{
	u8  sadr[0x1008];
	s32 ret;

	/* Get country code */
	ret = SYSCONF_Get("IPL.SADR", sadr, sizeof(sadr));
	if (ret < 0)
		return ret;

	/* Set value */
	*code = sadr[0];

	return 0;
}

s32 Setting_GetRegion(u8 *region)
{
	/* Get region code */
	switch (CONF_GetRegion()) {
	case CONF_REGION_JP: {
		*region = FW_REGION_JAP;
		break;
	}

	case CONF_REGION_US: {
		*region = FW_REGION_USA;
		break;
	}

	case CONF_REGION_EU: {
		*region = FW_REGION_EUR;
		break;
	}

	default:
		return -1;
	}

	return 0;
}

s32 Setting_SetCountry(u8 code)
{
	u8  sadr[0x1008];
	s32 ret;

	/* Get country code */
	ret = SYSCONF_Get("IPL.SADR", sadr, sizeof(sadr));
	if (ret < 0)
		return ret;

	/* Code already set */
	if (sadr[0] == code)
		return 0;

	memset(sadr, 0, sizeof(sadr));

	/* Set country code */
	sadr[0] = code;

	/* Write country code */
	ret = SYSCONF_Set("IPL.SADR", sadr, sizeof(sadr));
	if (ret < 0)
		return ret;

	/* Write conf file */
	return SYSCONF_SaveChanges();
}

s32 Setting_SetRegion(u8 region)
{
	char *regname = NULL;

	u8  area, game, video;
	s32 ret;

	/* Select region codes */
	switch (region) {
	case FW_REGION_JAP: {
		area    = CONF_AREA_JPN;
		game    = CONF_REGION_JP;
		video   = CONF_VIDEO_NTSC;
		regname = "JAP";

		break;
	}

	case FW_REGION_USA: {
		area    = CONF_AREA_USA;
		game    = CONF_REGION_US;
		video   = CONF_VIDEO_NTSC;
		regname = "USA";

		break;
	}

	case FW_REGION_EUR: {
		area    = CONF_AREA_EUR;
		game    = CONF_REGION_EU;
		video   = CONF_VIDEO_PAL;
		regname = "EUR";

		break;
	}

	default:
		return -1;
	}

	/* Change setting file permissions */
	ret = __Setting_SetPerms();
	if (ret < 0)
		return ret;

	/* Change area region code */
	ret = SYSCONF_SetArea(area);
	if (ret < 0)
		return ret;

	/* Change game region code */
	ret = SYSCONF_SetRegion(region);
	if (ret < 0)
		return ret;

	/* Change video mode code */
	ret = SYSCONF_SetVideo(video);
	if (ret < 0)
		return ret;

	/* Write setting file */
	ret = SYSCONF_SaveChanges();
	if (ret < 0)
		return ret;

	return 0;
}
