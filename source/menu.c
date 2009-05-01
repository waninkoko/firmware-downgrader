#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>

#include "firmware.h"
#include "restart.h"
#include "video.h"
#include "wpad.h"

/* Firmware list */
struct firmware fwList[] = {
	{ "2.0J", FW_REGION_JAP, 0x100000002ULL, 128, 0x10000000bULL,   10, 0x100000101ULL, 4, 0x100000100ULL, 2 },
	{ "2.0U", FW_REGION_USA, 0x100000002ULL, 129, 0x10000000bULL,   10, 0x100000101ULL, 4, 0x100000100ULL, 2 },
	{ "2.0E", FW_REGION_EUR, 0x100000002ULL, 130, 0x10000000bULL,   10, 0x100000101ULL, 4, 0x100000100ULL, 2 },
	{ "2.1J", FW_REGION_JAP, 0x100000002ULL, 160, 0x10000000bULL,   10, 0x100000101ULL, 4, 0x100000100ULL, 2 },
	{ "2.1U", FW_REGION_USA, 0x100000002ULL, 161, 0x10000000bULL,   10, 0x100000101ULL, 4, 0x100000100ULL, 2 },
	{ "2.1E", FW_REGION_EUR, 0x100000002ULL, 162, 0x10000000bULL,   10, 0x100000101ULL, 4, 0x100000100ULL, 2 },
	{ "2.2J", FW_REGION_JAP, 0x100000002ULL, 192, 0x100000014ULL,   12, 0x100000101ULL, 4, 0x100000100ULL, 2 },
	{ "2.2U", FW_REGION_USA, 0x100000002ULL, 193, 0x100000014ULL,   12, 0x100000101ULL, 4, 0x100000100ULL, 2 },
	{ "2.2E", FW_REGION_EUR, 0x100000002ULL, 194, 0x100000014ULL,   12, 0x100000101ULL, 4, 0x100000100ULL, 2 },
	{ "3.0J", FW_REGION_JAP, 0x100000002ULL, 224, 0x10000001eULL, 1039, 0x100000101ULL, 5, 0x100000100ULL, 2 },
	{ "3.0U", FW_REGION_USA, 0x100000002ULL, 225, 0x10000001eULL, 1039, 0x100000101ULL, 5, 0x100000100ULL, 2 },
	{ "3.0E", FW_REGION_EUR, 0x100000002ULL, 226, 0x10000001eULL, 1039, 0x100000101ULL, 5, 0x100000100ULL, 2 },
	{ "3.1J", FW_REGION_JAP, 0x100000002ULL, 256, 0x10000001eULL, 1039, 0x100000101ULL, 5, 0x100000100ULL, 2 },
	{ "3.1U", FW_REGION_USA, 0x100000002ULL, 257, 0x10000001eULL, 1039, 0x100000101ULL, 5, 0x100000100ULL, 2 },
	{ "3.1E", FW_REGION_EUR, 0x100000002ULL, 258, 0x10000001eULL, 1039, 0x100000101ULL, 5, 0x100000100ULL, 2 },
	{ "3.2J", FW_REGION_JAP, 0x100000002ULL, 288, 0x10000001eULL, 1039, 0x100000101ULL, 5, 0x100000100ULL, 2 },
	{ "3.2U", FW_REGION_USA, 0x100000002ULL, 289, 0x10000001eULL, 1039, 0x100000101ULL, 5, 0x100000100ULL, 2 },
	{ "3.2E", FW_REGION_EUR, 0x100000002ULL, 290, 0x10000001eULL, 1039, 0x100000101ULL, 5, 0x100000100ULL, 2 },
	{ "3.3J", FW_REGION_JAP, 0x100000002ULL, 352, 0x10000001eULL, 2576, 0x100000101ULL, 8, 0x100000100ULL, 4 },
	{ "3.3U", FW_REGION_USA, 0x100000002ULL, 353, 0x10000001eULL, 2576, 0x100000101ULL, 8, 0x100000100ULL, 4 },
	{ "3.3E", FW_REGION_EUR, 0x100000002ULL, 354, 0x10000001eULL, 2576, 0x100000101ULL, 8, 0x100000100ULL, 4 },
	{ "3.4J", FW_REGION_JAP, 0x100000002ULL, 384, 0x10000001eULL, 2816, 0x100000101ULL, 9, 0x100000100ULL, 5 },
	{ "3.4U", FW_REGION_USA, 0x100000002ULL, 385, 0x10000001eULL, 2816, 0x100000101ULL, 9, 0x100000100ULL, 5 },
	{ "3.4E", FW_REGION_EUR, 0x100000002ULL, 386, 0x10000001eULL, 2816, 0x100000101ULL, 9, 0x100000100ULL, 5 },
	{ "4.0J", FW_REGION_JAP, 0x100000002ULL, 416, 0x10000003cULL, 6174, 0x100000101ULL, 9, 0x100000100ULL, 5 },
	{ "4.0U", FW_REGION_USA, 0x100000002ULL, 417, 0x10000003cULL, 6174, 0x100000101ULL, 9, 0x100000100ULL, 5 },
	{ "4.0E", FW_REGION_EUR, 0x100000002ULL, 418, 0x10000003cULL, 6174, 0x100000101ULL, 9, 0x100000100ULL, 5 },

};

/* Macros */
#define NB_FIRMWARES		(sizeof(fwList) / sizeof(struct firmware))

/* Constants */
#define ENTRIES_PER_PAGE	6

/* Variables */
static s32 selected = 0, start = 0;


void __Menu_MoveList(s8 delta)
{
	s32 index;

	/* Select next entry */
	selected += delta;

	/* Out of the list? */
	if (selected <= -1)
		selected = (NB_FIRMWARES - 1);
	if (selected >= NB_FIRMWARES )
		selected = 0;

	/* List scrolling */
	index = (selected - start);

	if (index >= ENTRIES_PER_PAGE)
		start += index - (ENTRIES_PER_PAGE - 1);
	if (index <= -1)
		start += index;
}

void __Menu_PrintList(void)
{
	u32 cnt;

	printf("[+] Available firmwares:\n\n");

	for (cnt = start; cnt < NB_FIRMWARES; cnt++) {
		struct firmware *fw = &fwList[cnt];

		/* Entries per page limit reached */
		if ((cnt - start) >= ENTRIES_PER_PAGE)
			break;

		/* Selected entry */
		(cnt == selected) ? printf(">> ") : printf("   ");
		fflush(stdout);

		/* Print downgrade info */
		printf("Firmware %s [SysMenu v%d - IOS%d v%d - MIOS v%d - BC v%d]\n",
			fw->version, (u16)(fw->iosTid & 0xFF), fw->sysVersion, fw->iosVersion, fw->miosVersion, fw->bcVersion);
	}
}

void __Menu_Controls(void)
{
	u32 buttons = Wpad_WaitButtons();

	/* UP/DOWN buttons */
	if (buttons & WPAD_BUTTON_UP)
		__Menu_MoveList(-1);
	if (buttons & WPAD_BUTTON_DOWN)
		__Menu_MoveList(1);

	/* LEFT/RIGHT buttons */
	if (buttons & WPAD_BUTTON_LEFT)
		__Menu_MoveList(-ENTRIES_PER_PAGE);
	if (buttons & WPAD_BUTTON_RIGHT)
		__Menu_MoveList(ENTRIES_PER_PAGE);

	/* HOME button */
	if (buttons & WPAD_BUTTON_HOME)
		Restart();

	/* A button */
	if (buttons & WPAD_BUTTON_A)
		Firmware_Install(&fwList[selected]);
}


void Menu_Loop(void)
{
	/* Menu loop */
	for (;;) {
		/* Clear console */
		Con_Clear();

		/* Print entries */
		__Menu_PrintList();

		/* Controls */
		__Menu_Controls();
	}
}
