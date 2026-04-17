#include "syscalls.h"

#define NUM_OPERATIONS 0x40000
#define PAGE_SIZE 4096

void main(syslib_t *syslib) {
    syslib->initialize(syslib);

    syslib->logf("\n--- Start of Page Fault test ---");
    uint64_t start_time = syslib->get_time_in_microseconds();

    uint8_t *ptr = (uint8_t *) syslib->userspace_start;
    for(int i = 0; i < NUM_OPERATIONS; i++) {
		*ptr = 0xAA;
        ptr += PAGE_SIZE;
	}

    uint64_t end_time = syslib->get_time_in_microseconds();
    uint32_t total_time_us = end_time - start_time;

    syslib->logf("\n--- Results ---");
    syslib->logf("\nTime: %d us\nOne page fault: %d ns", total_time_us, total_time_us * 1000 / NUM_OPERATIONS);

    return;
}