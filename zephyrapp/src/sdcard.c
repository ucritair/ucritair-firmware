

#include <ff.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sdcard, LOG_LEVEL_DBG);

#include "sdcard.h"
#include "rtc.h"
#include "flash.h"

/*
 *  Note the fatfs library is able to mount only strings inside _VOLUME_STRS
 *  in ffconf.h
 */
#define DISK_DRIVE_NAME "SD"
#define DISK_MOUNT_PT "/"DISK_DRIVE_NAME":"

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

#define FS_RET_OK FR_OK


#define MAX_PATH 128
#define SOME_FILE_NAME "some.dat"
#define SOME_DIR_NAME "some"
#define SOME_REQUIRED_LEN MAX(sizeof(SOME_FILE_NAME), sizeof(SOME_DIR_NAME))

static const char *disk_mount_pt = DISK_MOUNT_PT;

bool did_post_sdcard = false;

static int lsdir(const char *path)
{
	int res;
	struct fs_dir_t dirp;
	static struct fs_dirent entry;
	int count = 0;

	fs_dir_t_init(&dirp);

	/* Verify fs_opendir() */
	res = fs_opendir(&dirp, path);
	if (res) {
		printk("Error opening dir %s [%d]\n", path, res);
		return res;
	}

	printk("\nListing dir %s ...\n", path);
	for (;;) {
		/* Verify fs_readdir() */
		res = fs_readdir(&dirp, &entry);

		/* entry.name[0] == 0 means end-of-dir */
		if (res || entry.name[0] == 0) {
			break;
		}

		if (entry.type == FS_DIR_ENTRY_DIR) {
			printk("[DIR ] %s\n", entry.name);
			if (strcmp(entry.name, "XYZ") == 0)
			{
				did_post_sdcard = true;
			}
		} else {
			printk("[FILE] %s (size = %zu)\n",
				entry.name, entry.size);
		}
		count++;
	}

	/* Verify fs_closedir() */
	fs_closedir(&dirp);
	if (res == 0) {
		res = count;
	}

	return res;
}

void test_sdcard()
{
	/* raw disk i/o */
	static const char *disk_pdrv = DISK_DRIVE_NAME;
	uint64_t memory_size_mb;
	uint32_t block_count;
	uint32_t block_size;

	if (disk_access_ioctl(disk_pdrv,
			DISK_IOCTL_CTRL_INIT, NULL) != 0) {
		LOG_ERR("Storage init ERROR!");
		return;
	}

	if (disk_access_ioctl(disk_pdrv,
			DISK_IOCTL_GET_SECTOR_COUNT, &block_count)) {
		LOG_ERR("Unable to get sector count");
		return;
	}
	LOG_INF("Block count %u", block_count);

	if (disk_access_ioctl(disk_pdrv,
			DISK_IOCTL_GET_SECTOR_SIZE, &block_size)) {
		LOG_ERR("Unable to get sector size");
		return;
	}
	printk("Sector size %u\n", block_size);

	memory_size_mb = (uint64_t)block_count * block_size;
	printk("Memory Size(MB) %u\n", (uint32_t)(memory_size_mb >> 20));

	if (disk_access_ioctl(disk_pdrv,
			DISK_IOCTL_CTRL_DEINIT, NULL) != 0) {
		LOG_ERR("Storage deinit ERROR!");
		return;
	}

	mp.mnt_point = disk_mount_pt;

	int res = fs_mount(&mp);

	if (res == FS_RET_OK) {
		printk("Disk mounted.\n");
		/* Try to unmount and remount the disk */
		res = fs_unmount(&mp);
		if (res != FS_RET_OK) {
			printk("Error unmounting disk\n");
			return;
		}
		res = fs_mount(&mp);
		if (res != FS_RET_OK) {
			printk("Error remounting disk\n");
			return;
		}

		if (lsdir(disk_mount_pt) == 0) {
#ifdef CONFIG_FS_SAMPLE_CREATE_SOME_ENTRIES
			if (create_some_entries(disk_mount_pt)) {
				lsdir(disk_mount_pt);
			}
#endif
		}
	} else {
		printk("Error mounting disk.\n");
	}

	fs_unmount(&mp);
}

enum sdcard_result write_log_to_sdcard()
{
	mp.mnt_point = disk_mount_pt;

	int ret = OK;
	int err = fs_mount(&mp);

	if (err == FS_RET_OK) {
		// OK
	}
	else if (err == FR_DISK_ERR || err == FR_NOT_READY)
	{
		LOG_ERR("Failed to mount, disk err");
		ret = FAIL_INIT;
		goto out;
	}
	else if (err == FR_NO_FILESYSTEM)
	{
		LOG_ERR("Failed to mount, no FS");
		ret = FAIL_MOUNT;
		goto out;
	}
	else
	{
		LOG_ERR("Unknown mount error: %d", err);
		ret = FAIL_UNKNOWN;
		goto out;
	}

	err = fs_mkdir(DISK_MOUNT_PT "/CAT_LOG");

	if (err == 0 || err == -EEXIST)
	{
		// OK
	}
	else
	{
		LOG_ERR("Failed to mkdir");
		ret = FAIL_MKDIR;
		goto out;
	}

	char buf[256];

	int count = 0;
	while (true)
	{
		snprintf(buf, sizeof(buf), DISK_MOUNT_PT "/CAT_LOG/LOG_%d.CSV", count);
		struct fs_dirent ent;
		err = fs_stat(buf, &ent);

		if (err == 0)
		{
			count++;
			continue;
		}
		else if (err == -ENOENT)
		{
			break;
		}
		else
		{
			LOG_DBG("Unknown error stat: %d", err);
			goto out;
		}
	}

	LOG_DBG("Identified path '%s'", buf);

	struct fs_file_t file;
	fs_file_t_init(&file);
	err = fs_open(&file, buf, FS_O_WRITE|FS_O_CREATE|FS_O_TRUNC);

	if (err != 0)
	{
		LOG_ERR("Failed to open '%s': %d", buf, err);
		ret = FAIL_CREATE;
		goto out;
	}

	int len = snprintf(buf, sizeof(buf), "Timestamp,CO2,PM1.0,PM2.5,PM4.0,PM10,TempC,RH,VOC,NOX\n");

	err = fs_write(&file, buf, len);
	if (err != len)
	{
		LOG_ERR("Failed to write header: %d", err);
		ret = FAIL_WRITE;
		goto out_f;
	}

	for (int nr = 0; nr < next_log_cell_nr; nr++)
	{
		LOG_DBG("Write cell %d", nr);

		struct flash_log_cell cell;
		flash_get_cell_by_nr(nr, &cell);

#define UG(x) (((double)x)/100.)

		len = snprintf(buf, sizeof(buf), "%lld,%d,%.1f,%.1f,%.1f,%.1f,%.2f,%.1f,%d,%d\n",
			RTC_TIME_TO_EPOCH_TIME(cell.timestamp),
			cell.co2_ppmx1,
			UG(cell.pm_ugmx100[0]),
			UG(cell.pm_ugmx100[1]),
			UG(cell.pm_ugmx100[2]),
			UG(cell.pm_ugmx100[3]),
			(((double)cell.temp_Cx1000)/1000.),
			(((double)cell.rh_pctx100)/100.),
			cell.voc_index,
			cell.nox_index
		);
		err = fs_write(&file, buf, len);

		if (err != len)
		{
			LOG_ERR("Failed to write row: %d", len);
			ret = FAIL_WRITE;
			goto out_f;
		}
	}

	out_f:
	if (fs_close(&file))
	{
		LOG_ERR("Failed closing file");

		if (ret == OK)
		{
			ret = FAIL_CLOSE;
		}
	}

	out:
	fs_unmount(&mp);
	return ret;
}
