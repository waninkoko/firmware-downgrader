#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>

/* Constants */
#define BASE_PATH	"/tmp"


s32 Nand_Init(void)
{
	return ISFS_Initialize();
}

s32 Nand_CreateDir(u64 tid)
{
	char dirpath[ISFS_MAXPATH];

	/* Generate dirpath */
	sprintf(dirpath, BASE_PATH "/%08x", (u32)(tid & 0xFFFFFFFF));

	/* Create directory */
	return ISFS_CreateDir(dirpath, 0, ISFS_OPEN_RW, ISFS_OPEN_RW, ISFS_OPEN_RW);
}

s32 Nand_CreateFile(u64 tid, const char *filename)
{
	char filepath[ISFS_MAXPATH];

	/* Generate filepath */
	sprintf(filepath, BASE_PATH "/%08x/%s", (u32)(tid & 0xFFFFFFFF), filename);

	/* Create file */
	return ISFS_CreateFile(filepath, 0, ISFS_OPEN_RW, ISFS_OPEN_RW, ISFS_OPEN_RW);
}

s32 Nand_OpenFile(u64 tid, const char *filename, u8 mode)
{
	char filepath[ISFS_MAXPATH];

	/* Generate filepath */
	sprintf(filepath, BASE_PATH "/%08x/%s", (u32)(tid & 0xFFFFFFFF), filename);

	/* Open file */
	return IOS_Open(filepath, mode);
}

s32 Nand_CloseFile(s32 fd)
{
	/* Close file */
	return IOS_Close(fd);
}

s32 Nand_ReadFile(s32 fd, void *buf, u32 len)
{
	/* Read file */
	return IOS_Read(fd, buf, len);
}

s32 Nand_WriteFile(s32 fd, void *buf, u32 len)
{
	/* Write file */
	return IOS_Write(fd, buf, len);
}

s32 Nand_RemoveDir(u64 tid)
{
	char *dirlist = NULL;

	char dirpath[ISFS_MAXPATH], filepath[ISFS_MAXPATH];
	u32  cnt, idx, nb_files;
	s32  ret;

	/* Generate dirpath */
	sprintf(dirpath, BASE_PATH "/%08x", (u32)(tid & 0xFFFFFFFF));

	/* Retrieve number of files */
	ret = ISFS_ReadDir(dirpath, dirlist, &nb_files);
	if (ret < 0)
		return ret;

	/* There are files inside the directory */
	if (nb_files) {
		/* Allocate memory */
		dirlist = (char *)memalign(32, ISFS_MAXPATH * nb_files);
		if (!dirlist)
			return -1;

		/* Retrieve filelist */
		ret = ISFS_ReadDir(dirpath, dirlist, &nb_files);
		if (ret < 0)
			goto out;

		for (cnt = idx = 0; cnt < nb_files; cnt++) {
			char *ptr = dirlist + idx;

			/* Generate filepath */
			sprintf(filepath, "%s/%s", dirpath, ptr);

			/* Delete file */
			ret = ISFS_Delete(filepath);
			if (ret < 0)
				goto out;

			/* Move to next entry */
			idx += strlen(ptr) + 1;
		}
	}

	/* Delete directory */
	ret = ISFS_Delete(dirpath);

out:
	/* Free memory */
	if (dirlist)
		free(dirlist);

	return ret;
}
