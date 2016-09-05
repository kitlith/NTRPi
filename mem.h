#pragma once

#include <stdint.h>

void* memcpy(void* dst, const void* src, uint32_t len);
void* memset(void* dst, uint8_t val, uint32_t len);
