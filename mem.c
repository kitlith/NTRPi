// Taken from Stary in CirnOS by Kitlith. Thanks!
#include "mem.h"

void* memcpy(void* dst, const void* src, uint32_t len) {
	if(len == 0) return dst;

	uint8_t *d = (uint8_t*)dst;
	const uint8_t *c = (const uint8_t*)src;

	do {
		*d++ = *c++;
	} while(--len);

	return dst;
}

void* memset(void* dst, uint8_t val, uint32_t len) {
	if(len == 0) return dst;

	uint8_t *d = (uint8_t*)dst;
	do {
		*d++ = val;
	} while(--len);

	return dst;
}
