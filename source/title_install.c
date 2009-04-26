#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>

#include "nand.h"
#include "network.h"
#include "sys.h"
#include "title.h"
#include "video.h"

/* 'WAD header' struct */
typedef struct {
	/* Header length */
	u32 header_len;

	/* WAD type */
	char type[2];

	/* Padding data */
	u16 padding;

	/* Data length */
	u32 certs_len, crl_len, tik_len, tmd_len, data_len, footer_len;
} ATTRIBUTE_PACKED wad_header;

/* Variables */
static u8 titleBuf[BLOCK_SIZE] ATTRIBUTE_ALIGN(32);


s32 __Title_ReadNetwork(u64 tid, const char *filename, void **outbuf, u32 *outlen)
{
	void *buffer = NULL;

	char netpath[ISFS_MAXPATH];
	u32  len;
	s32  ret;

	/* Generate network path */
	sprintf(netpath, "%016llx/%s", tid, filename);

	/* Request file */
	ret = Network_Request(netpath, &len);
	if (ret < 0)
		return ret;

	/* Allocate memory */
	buffer = memalign(32, len);
	if (!buffer)
		return -1;

	/* Download file */
	ret = Network_Read(buffer, len);
	if (ret != len) {
		free(buffer);
		return -2;
	}

	/* Set values */
	*outbuf = buffer;
	if (outlen)
		*outlen = len;

	return 0;
}

s32 __Title_SaveContent(tik *p_tik, tmd_content *content, void *buffer, u32 len)
{
	char filename[ISFS_MAXPATH];
	s32  fd = -1, ret;

	/* Genereate filename */
	sprintf(filename, "%08x", content->cid);

	/* Create file */
	ret = Nand_CreateFile(p_tik->titleid, filename);
	if (ret < 0)
		goto out;

	/* Open file */
	fd = Nand_OpenFile(p_tik->titleid, filename, ISFS_OPEN_WRITE);
	if (fd < 0) {
		ret = fd;
		goto out;
	}

	/* Write file */
	ret = Nand_WriteFile(fd, buffer, len);
	if (ret != len) {
		ret = -1;
		goto out;
	}

	/* Success */
	ret = 0;

out:
	/* Close file */
	if (fd >= 0)
		Nand_CloseFile(fd);

	return ret;
}

s32 __Title_DownloadContent(tik *p_tik, tmd_content *content)
{
	u8 *buffer = NULL;

	char filename[ISFS_MAXPATH];
	u32  len;
	s32  ret;

	/* Genereate filename */
	sprintf(filename, "%08x", content->cid);

	/* Download content file */
	ret = __Title_ReadNetwork(p_tik->titleid, filename, (void *)&buffer, &len);
	if (ret < 0)
		return ret;

	/* Save content */
	ret = __Title_SaveContent(p_tik, content, buffer, len);

	/* Free memory */
	if (buffer)
		free(buffer);

	return ret;
}


s32 Title_Download(u64 tid, u16 version, signed_blob **p_tik, signed_blob **p_tmd)
{
	signed_blob *s_tik = NULL, *s_tmd = NULL;

	tik *tik_data = NULL;
	tmd *tmd_data = NULL;

	char filename[ISFS_MAXPATH];
	s32  cnt, ret;

	printf("\r\t\t>> Creating temp directory...");
	fflush(stdout);

	/* Create temp dir */
	ret = Nand_CreateDir(tid);
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		goto err;
	}

	Con_ClearLine();

	printf("\r\t\t>> Downloading ticket...");
	fflush(stdout);

	/* Download ticket */
	ret = __Title_ReadNetwork(tid, "cetk", (void *)&s_tik, NULL);
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		goto err;
	}

	Con_ClearLine();

	printf("\r\t\t>> Downloading TMD...");
	fflush(stdout);

	/* TMD filename */
	if (version)
		sprintf(filename, "tmd.%d", version);
	else
		sprintf(filename, "tmd");

	/* Download TMD */
	ret = __Title_ReadNetwork(tid, filename, (void *)&s_tmd, NULL);
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		goto err;
	}

	/* Get ticket/TMD info */
	tik_data = (tik *)SIGNATURE_PAYLOAD(s_tik);
	tmd_data = (tmd *)SIGNATURE_PAYLOAD(s_tmd);

	/* Title contents */
	for (cnt = 0; cnt < tmd_data->num_contents; cnt++) {
		tmd_content *content = &tmd_data->contents[cnt];

		Con_ClearLine();

		printf("\r\t\t>> Downloading content #%02d...", content->cid);
		fflush(stdout);

		/* Download content file */
		ret = __Title_DownloadContent(tik_data, content);
		if (ret < 0) {
			printf(" ERROR! (ret = %d)\n", ret);
			goto err;
		}
	}

	/* Set pointers */
	*p_tik = s_tik;
	*p_tmd = s_tmd;

	Con_ClearLine();

	printf("\r\t\t>> Title downloaded successfully!\n");
	fflush(stdout);

	return 0;

err:
	/* Free memory */
	if (s_tik)
		free(s_tik);
	if (s_tmd)
		free(s_tmd);

	return ret;
}

s32 Title_ExtractWAD(u8 *buffer, signed_blob **p_tik, signed_blob **p_tmd)
{
	wad_header  *header = (wad_header *)buffer;
	signed_blob *s_tik  = NULL, *s_tmd = NULL;

	tik *tik_data = NULL;
	tmd *tmd_data = NULL;

	u32 cnt, offset = 0;
	s32 ret;

	/* Move to ticket */
	offset += round_up(header->header_len, 64);
	offset += round_up(header->certs_len,  64);
	offset += round_up(header->crl_len,    64);

	printf("\r\t\t>> Extracting ticket...");
	fflush(stdout);

	/* Copy ticket */
	s_tik = (signed_blob *)memalign(32, header->tik_len);
	if (!s_tik) {
		ret = -1;

		printf(" ERROR! (ret = %d)\n", ret);
		goto err;
	}

	memcpy(s_tik, buffer + offset, header->tik_len);

	/* Move to TMD */
	offset += round_up(header->tik_len, 64);

	Con_ClearLine();

	printf("\r\t\t>> Extracting TMD...");
	fflush(stdout);

	/* Copy TMD */
	s_tmd = (signed_blob *)memalign(32, header->tmd_len);
	if (!s_tmd) {
		ret = -1;

		printf(" ERROR! (ret = %d)\n", ret);
		goto err;
	}

	memcpy(s_tmd, buffer + offset, header->tmd_len);

	offset += round_up(header->tmd_len, 64);

	/* Get ticket/TMD info */
	tik_data = (tik *)SIGNATURE_PAYLOAD(s_tik);
	tmd_data = (tmd *)SIGNATURE_PAYLOAD(s_tmd);

	printf("\r\t\t>> Creating temp directory...");
	fflush(stdout);

	/* Create temp dir */
	ret = Nand_CreateDir(tik_data->titleid);
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		goto err;
	}

	/* Title contents */
	for (cnt = 0; cnt < tmd_data->num_contents; cnt++) {
		tmd_content *content = &tmd_data->contents[cnt];
		u32      content_len = round_up(content->size, 64);

		void *p_content = NULL;

		Con_ClearLine();

		printf("\r\t\t>> Extracting content #%02d...", content->cid);
		fflush(stdout);

		/* Allocate memory */
		p_content = memalign(32, content_len);
		if (!p_content) {
			ret = -1;

			printf(" ERROR! (ret = %d)\n", ret);
			goto err;
		}

		/* Extract content */
		memcpy(p_content, buffer + offset, content_len);

		/* Save content */
		ret = __Title_SaveContent(tik_data, content, p_content, content_len);
		if (ret < 0) {
			free(p_content);

			printf(" ERROR! (ret = %d)\n", ret);
			goto err;
		}

		/* Free memory */
		free(p_content);

		/* Move to next content */
		offset += content_len;
	}

	/* Set pointers */
	*p_tik = s_tik;
	*p_tmd = s_tmd;

	Con_ClearLine();

	printf("\r\t\t>> Title extracted successfully!\n");
	fflush(stdout);

	return 0;

err:
	/* Free memory */
	if (s_tik)
		free(s_tik);
	if (s_tmd)
		free(s_tmd);

	return ret;
}

s32 Title_Install(signed_blob *p_tik, signed_blob *p_tmd)
{
	signed_blob *p_certs  = NULL;
	tmd         *tmd_data = NULL;

	u32 certs_len, tik_len, tmd_len;
	s32 cfd = -1, fd = -1;

	s32 cnt, ret;

	printf("\t\t\t>> Getting certificates...");
	fflush(stdout);

	/* Get certificates */
	ret = Sys_GetCerts(&p_certs, &certs_len);
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		return ret;
	}

	/* Get ticket lenght */
	tik_len = SIGNED_TIK_SIZE(p_tik);

	/* Get TMD length */
	tmd_len = SIGNED_TMD_SIZE(p_tmd);

	/* Get TMD info */
	tmd_data = (tmd *)SIGNATURE_PAYLOAD(p_tmd);

	Con_ClearLine();

	printf("\r\t\t>> Installing ticket...");
	fflush(stdout);

	/* Install ticket */
	ret = ES_AddTicket(p_tik, tik_len, p_certs, certs_len, NULL, 0);
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		return ret;
	}

	Con_ClearLine();

	printf("\r\t\t>> Installing title...");
	fflush(stdout);

	/* Install title */
	ret = ES_AddTitleStart(p_tmd, tmd_len, p_certs, certs_len, NULL, 0);
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		return ret;
	}

	/* Install contents */
	for (cnt = 0; cnt < tmd_data->num_contents; cnt++) {
		tmd_content *content = &tmd_data->contents[cnt];

		char filename[ISFS_MAXPATH];

		Con_ClearLine();

		printf("\r\t\t>> Installing content #%02d...", content->cid);
		fflush(stdout);

		/* Content filename */
		sprintf(filename, "%08x", content->cid);

		/* Open content file */
		fd = Nand_OpenFile(tmd_data->title_id, filename, ISFS_OPEN_READ);
		if (fd < 0) {
			printf("ERROR! (ret = %d)\n", fd);
			ret = fd;

			goto err;
		}

		/* Add content */
		cfd = ES_AddContentStart(tmd_data->title_id, content->cid);
		if (cfd < 0) {
			printf(" ERROR! (ret = %d)\n", cfd);
			ret = cfd;

			goto err;
		}

		/* Add content data */
		for (;;) {
			u32 size;

			/* Read content data */
			ret = Nand_ReadFile(fd, titleBuf, BLOCK_SIZE);
			if (ret < 0) {
				printf(" ERROR! (ret = %d)\n", ret);
				goto err;
			}

			/* EOF */
			if (!ret)
				break;

			/* Block size */
			size = ret;

			/* Add content data */
			ret = ES_AddContentData(cfd, titleBuf, size);
			if (ret < 0) {
				printf(" ERROR! (ret = %d)\n", ret);
				goto err;
			}
		}

		/* Finish content installation */
		ret = ES_AddContentFinish(cfd);
		if (ret < 0) {
			printf(" ERROR! (ret = %d)\n", ret);
			goto err;
		}

		/* Close content file */
		Nand_CloseFile(fd);
	}

	Con_ClearLine();

	printf("\r\t\t>> Finishing installation...");
	fflush(stdout);

	/* Finish title install */
	ret = ES_AddTitleFinish();
	if (ret < 0) {
		printf(" ERROR! (ret = %d)\n", ret);
		goto err;
	} else
		printf(" OK!\n");

	return 0;

err:
	/* Close content file */
	if (fd >= 0)
		Nand_CloseFile(fd);

	/* Finish content installation */
	if (cfd >= 0)
		ES_AddContentFinish(cfd);

	/* Cancel title installation */
	ES_AddTitleCancel();

	return ret;
}

s32 Title_Clean(signed_blob *p_tmd)
{
	tmd *tmd_data = NULL;

	/* Retrieve TMD info */
	tmd_data = (tmd *)SIGNATURE_PAYLOAD(p_tmd);

	/* Delete title contents */
	return Nand_RemoveDir(tmd_data->title_id);
}
