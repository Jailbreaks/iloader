#define PUT_QWORD_BE(ptr, off, val) \
    *(uint64_t *)((char *)(ptr) + (off)) = ((((uint64_t)(val) >> 56) & 0xFF) | \
					    (((uint64_t)(val) >> 40) & 0xFF00) | \
					    (((uint64_t)(val) >> 24) & 0xFF0000) | \
					    (((uint64_t)(val) >> 8) & 0xFF000000) | \
					    (((uint64_t)(val) & 0xFF000000) << 8) | \
					    (((uint64_t)(val) & 0xFF0000) << 24) | \
					    (((uint64_t)(val) & 0xFF00) << 40) | \
					    (((uint64_t)(val) & 0xFF) << 56))

#define PUT_DWORD_BE(ptr, off, val) \
    *(uint32_t *)((char *)(ptr) + (off)) = ((((uint32_t)(val) >> 24) & 0xFF) | \
					    (((uint32_t)(val) >> 8) & 0xFF00) | \
					    (((uint32_t)(val) & 0xFF00) << 8) | \
					    (((uint32_t)(val) & 0xFF) << 24))

#define PUT_WORD_BE(ptr, off, val) \
    *(uint16_t *)((char *)(ptr) + (off)) = ((((val) >> 8) & 0xFF) | (((val) & 0xFF) << 8))

#define PUT_QWORD_LE(ptr, off, val) *(uint64_t *)((char *)(ptr) + (off)) = (uint64_t)(val)

#define PUT_DWORD_LE(ptr, off, val) *(uint32_t *)((char *)(ptr) + (off)) = (uint32_t)(val)

#define PUT_WORD_LE(ptr, off, val) *(uint16_t *)((char *)(ptr) + (off)) = (uint16_t)(val)
