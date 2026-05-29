#pragma once
#include <stdint.h>

struct mbr_partition_t {
    uint8_t status;
    uint8_t chs_first[3];
    uint8_t type;
    uint8_t chs_last[3];
    uint32_t lba_start;
    uint32_t sector_count;
} __attribute__((packed));

void mbr_parse();