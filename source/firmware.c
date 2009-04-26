#include <stdio.h>
#include <stdlib.h>
#include <ogcsys.h>

#include "countries.h"
#include "firmware.h"
#include "setting.h"
#include "title.h"
#include "title_install.h"
#include "video.h"
#include "wpad.h"

/* Tickets and TMDs */
static signed_blob *iosTik  = NULL, *iosTmd  = NULL;
static signed_blob *miosTik = NULL, *miosTmd = NULL;
static signed_blob *bcTik   = NULL, *bcTmd   = NULL;
static signed_blob *sysTik  = NULL, *sysTmd  = NULL;


s32 __Firmware_Download(struct firmware *fw)
{
	u16 version;
	s32 ret;

	/* Download IOS */
	ret = Title_GetVersion(fw->iosTid, &version);

	if (!ret && (version != fw->iosVersion)) {
		printf("\n");
		printf("[+] Downloading IOS%d, please wait...\n", (u8)(fw->iosTid & 0xFF));

		ret = Title_Download(fw->iosTid, fw->iosVersion, &iosTik, &iosTmd);
		if (ret < 0)
			return ret;
	}

	/* Download MIOS */
	ret = Title_GetVersion(fw->miosTid, &version);

	if (!ret && (version != fw->miosVersion)) {
		printf("\n");
		printf("[+] Downloading MIOS v%d, please wait...\n", fw->miosVersion);

		ret = Title_Download(fw->miosTid, fw->miosVersion, &miosTik, &miosTmd);
		if (ret < 0)
			return ret;
	}

	/* Download BC */
	ret = Title_GetVersion(fw->bcTid, &version);

	if (!ret && (version != fw->bcVersion)) {
		printf("\n");
		printf("[+] Downloading BC v%d, please wait...\n", fw->bcVersion);

		ret = Title_Download(fw->bcTid, fw->bcVersion, &bcTik, &bcTmd);
		if (ret < 0)
			return ret;
	}

	/* Download system menu */
	ret = Title_GetVersion(fw->sysTid, &version);

	if (!ret && (version != fw->sysVersion)) {
		printf("\n");
		printf("[+] Downloading System Menu v%d, please wait...\n", fw->sysVersion);

		ret = Title_Download(fw->sysTid, fw->sysVersion, &sysTik, &sysTmd);
		if (ret < 0)
			return ret;
	}

	return 0;
}

s32 __Firmware_Install(struct firmware *fw)
{
	s32 ret;

	/* Install IOS */
	if (iosTik && iosTmd) {
		printf("\n");
		printf("[+] Installing IOS%d, please wait...\n", (u8)(fw->iosTid & 0xFF));

		ret = Title_Install(iosTik, iosTmd);
		if (ret < 0)
			return ret;
	}

	/* Install MIOS */
	if (miosTik && miosTmd) {
		printf("\n");
		printf("[+] Installing MIOS v%d, please wait...\n", fw->miosVersion);

		ret = Title_Install(miosTik, miosTmd);
		if (ret < 0)
			return ret;
	}

	/* Install BC */
	if (bcTik && bcTmd) {
		printf("\n");
		printf("[+] Installing BC v%d, please wait...\n", fw->bcVersion);

		ret = Title_Install(bcTik, bcTmd);
		if (ret < 0)
			return ret;
	}

	/* Install system menu */
	if (sysTik && sysTmd) {
		printf("\n");
		printf("[+] Installing System Menu v%d, please wait...\n", fw->sysVersion);

		ret = Title_Install(sysTik, sysTmd);
		if (ret < 0)
			return ret;
	}

	return 0;
}

void __Firmware_Clean(struct firmware *fw)
{
	printf("\n");
	printf("[+] Cleaning temporary data, please wait...\n");

	/* Clean IOS */
	if (iosTmd)
		Title_Clean(iosTmd);

	/* Clean MIOS */
	if (miosTmd)
		Title_Clean(miosTmd);

	/* Download BC */
	if (bcTmd)
		Title_Clean(bcTmd);

	/* Download system menu */
	if (sysTmd)
		Title_Clean(sysTmd);
}

s32 __Firmware_SetCountry(void)
{
	char *name = NULL;
	u8    code;

	s32 ret;

	printf("\n");

	printf("[+] Change Wii Shop country code?\n");
	printf("    Press A button to continue, otherwise press B button.\n\n");

	/* Wait for user response */
	for (;;) {
		u32 buttons = Wpad_WaitButtons();

		/* A button */
		if (buttons & WPAD_BUTTON_A)
			break;
	
		/* B button */
		if (buttons & WPAD_BUTTON_B)
			return 0;
	}

	/* Get country code */
	ret = Setting_GetCountry(&code);
	if (ret < 0) {
		printf("\n[+] ERROR: Could not retrieve current country code! (ret = %d)\n", ret);
		return ret;
	}

	for (;;) {
		Con_ClearLine();

		/* Retrieve country name */
		name = countryCodes[code-1];
		if (!name)
			name = "Unknown Country";

		/* Print selected country code */
		printf(">> Select new Wii Shop country code: %s (code %d)", name, code);
		fflush(stdout);

		u32 buttons = Wpad_WaitButtons();

		/* LEFT/RIGHT button */
		if (buttons & WPAD_BUTTON_LEFT) {
			if ((--code) < MIN_COUNTRY_CODE)
				code = MAX_COUNTRY_CODE;
		}
		if (buttons & WPAD_BUTTON_RIGHT) {
			if ((++code) > MAX_COUNTRY_CODE)
				code = MIN_COUNTRY_CODE;
		}

		/* A button */
		if (buttons & WPAD_BUTTON_A)
			break;
	}

	printf("[+] Changing Wii country code, please wait...");
	fflush(stdout);

	/* Set country code */
	ret = Setting_SetCountry(code);
	if (ret < 0)
		printf(" ERROR! (ret = %d)\n", ret);
	else
		printf(" OK!\n");

	return ret;
}

s32 __Firmware_SetRegion(struct firmware *fw)
{
	u8  region;
	s32 ret;

	printf("\n");

	/* Check current region code */
	ret = Setting_GetRegion(&region);
	if (ret < 0) {
		printf("[+] ERROR: Could not retrieve current region code! (ret = %d)\n", ret);
		return ret;
	}

	if (region != fw->region) {
		printf("[+] Change your Wii console region to match the firmware region?\n");
		printf("    If you do not, your Wii will be semibricked!!\n\n");

		printf("    Press A button to continue, otherwise press B button.\n\n");

		/* Wait for user response */
		for (;;) {
			u32 buttons = Wpad_WaitButtons();

			/* A button */
			if (buttons & WPAD_BUTTON_A)
				break;
	
			/* B button */
			if (buttons & WPAD_BUTTON_B)
				return 0;
		}

		printf("[+] Changing Wii region code, please wait...");
		fflush(stdout);

		/* Change Wii region code */
		ret = Setting_SetRegion(fw->region);
		if (ret < 0)
			printf(" ERROR! (ret = %d)\n", ret);
		else
			printf(" OK!\n");
	}

	return ret;
}

void __Firmware_Reset(void)
{
	/* Reset IOS variables */
	iosTik = iosTmd = NULL;

	/* Reset MIOS variables */
	miosTik = miosTmd = NULL;

	/* Reset BC variables */
	bcTik = bcTmd = NULL;

	/* Reset system menu variables */
	sysTik = sysTmd = NULL;
}

void __Firmware_Free(void)
{
	/* IOS data */
	if (iosTik)
		free(iosTik);
	if (iosTmd)
		free(iosTmd);

	/* MIOS data */
	if (miosTik)
		free(miosTik);
	if (miosTmd)
		free(miosTmd);

	/* BC data */
	if (bcTik)
		free(bcTik);
	if (bcTmd)
		free(bcTmd);

	/* System Menu data */
	if (sysTik)
		free(sysTik);
	if (sysTmd)
		free(sysTmd);
}


void Firmware_Install(struct firmware *fw)
{
	s32 ret;

	/* Clear screen */
	Con_Clear();

	printf("[+] Are you sure you want to downgrade your Wii firmware?\n");
	printf("    Firmware: \"%s\"\n\n", fw->version);

	printf("    Press A button to continue, otherwise press B button.\n\n");

	/* Wait for user response */
	for (;;) {
		u32 buttons = Wpad_WaitButtons();

		/* A button */
		if (buttons & WPAD_BUTTON_A)
			break;

		/* B button */
		if (buttons & WPAD_BUTTON_B)
			return;
	}

	/* Reset variables */
	__Firmware_Reset();

	/* Download titles */
	ret = __Firmware_Download(fw);
	if (ret < 0)
		goto out;

	/* Install titles */
	ret = __Firmware_Install(fw);
	if (ret < 0)
		goto out;

	/* Set console region */
	ret = __Firmware_SetRegion(fw);
	if (ret < 0)
		goto out;

	/* Set shop country */
	ret = __Firmware_SetCountry();

out:
	/* Clean titles downloads */
	__Firmware_Clean(fw);

	/* Free memory */
	__Firmware_Free();

	printf("\n");

	if (ret < 0)
		printf("[+] Firmware installation FAILED!\n\n");
	else
		printf("[+] Firmware installation finished successfully!\n\n");


	printf("    Press any button to continue...\n");

	/* Wait for any button */
	Wpad_WaitButtons();
}
