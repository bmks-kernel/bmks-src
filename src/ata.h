#pragma once
#include <stdint.h>

bool ata_read_sector(uint32_t lba, uint8_t* buffer);