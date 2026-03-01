#ifndef BL_UPDATE_H
#define BL_UPDATE_H

#include <stdbool.h>

#define DFU_MAGIC_VALUE 0xB1

/* Flash the embedded bootloader image to the boot partition.
 * Returns 0 on success. Device will reboot on success.
 * Returns negative on error (battery low, flash failure, verify failure).
 * Returns -1 if bootloader image not included in build. */
int bl_update_flash(void);

/* Set GPREGRET and reboot into DFU mode */
void bl_enter_dfu(void);

/* Returns true if this firmware was built with the embedded bootloader image */
bool bl_image_included(void);

#endif
