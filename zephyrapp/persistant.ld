endmap_presist_region (NOLOAD) : {
	. += RAM_SIZE - __kernel_ram_size - 0x1000;
	*(.endmap_presist_region);
} > RAM

