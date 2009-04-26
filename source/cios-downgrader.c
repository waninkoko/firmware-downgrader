#include <stdio.h>
#include <ogcsys.h>

#include "gui.h"
#include "menu.h"
#include "nand.h"
#include "network.h"
#include "restart.h"
#include "sys.h"
#include "video.h"
#include "wpad.h"


void Disclaimer(void)
{
	/* Print disclaimer */
	printf("[+] [DISCLAIMER]:\n\n");

	printf("    THIS APPLICATION COMES WITH NO WARRANTY AT ALL,\n");
	printf("    NEITHER EXPRESS NOR IMPLIED.\n");
	printf("    I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE IN YOUR\n");
	printf("    WII CONSOLE BECAUSE OF A IMPROPER USAGE OF THIS SOFTWARE.\n\n");

	printf(">>  If you agree, press A button to continue.\n");
	printf(">>  Otherwise, press B button to restart your Wii.\n\n");

	/* Wait for user answer */
	for (;;) {
		u32 buttons = Wpad_WaitButtons();

		/* A button */
		if (buttons & WPAD_BUTTON_A)
			break;

		/* B button */
		if (buttons & WPAD_BUTTON_B)
			Restart();
	}
}

int main(int argc, char **argv)
{
	s32 ret;

	/* Load Custom IOS */
	ret = IOS_ReloadIOS(249);

	/* Initialize subsystems */
	Sys_Init();

	/* Set video mode */
	Video_SetMode();

	/* Initialize console */
	Gui_InitConsole();

	/* Draw background */
	Gui_DrawBackground();

	/* Initialize Wiimote subsystem */
	Wpad_Init();

	/* Custom IOS not loaded */
	if (ret < 0) {
		printf("[+] ERROR: Could not load Custom IOS! (ret = %d)\n", ret);
		goto err;
	}

	printf("[+] Initializing NAND subsystem...");
	fflush(stdout);

	/* Initialize NAND subsystem */
	ret = Nand_Init();
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		goto err;
	} else
		printf(" OK!\n");

	printf("[+] Initializing network, please wait...");
	fflush(stdout);

	/* Initialize network */
	ret = Network_Init();
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		goto err;
	} else
		printf(" %s\n\n", Network_GetIP());

	/* Print disclaimer */
	Disclaimer();

	/* Menu loop */
	Menu_Loop();

err:
	/* Restart Wii */
	Restart_Wait();

	return 0;
}
