// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __flash_simple_h__
#define __flash_simple_h__

#include <xccompat.h>
#include <stddef.h>
#include <quadflash.h>

/**
 * Fast simple QSPI flash read using standard 0xEB command
 *
 * Also computes a CRC-based checksum
 *
 * Restricted to XC so boundary check is enforced for safety
 *
 * \param address     Flash (byte) source address
 * \param words       Memory destination
 * \param num_words   Number of 32bit words to read
 * \param crc         Resulting checksum
 * \param ports       Ports and clock block (quadflash library struct)
 *
 * \return            Zero for success or a non-zero error code
 */
#ifdef __XC__
int flash_simple_read(unsigned address, unsigned words[],
                      unsigned num_words, unsigned &crc,
                      fl_QSPIPorts &ports);
#endif

/**
 * Helper function to read the fixed in flash that contains boot partition size
 * as written by xflash into stage 2 header during factory programming
 *
 * Does an additional nibble byte swap (swap nibbles of each byte) to
 * compensate for the fact that xflash writes into boot partition nibble swapped
 * like this (unlike data partition)
 *
 * \param boot_partition_size  Boot partition size as read from flash
 * \param ports                Ports and clock block (quadflash library struct)
 *
 * \return                     Zero for success or a non-zero error code
 */
int flash_simple_read_boot_partition_size(REFERENCE_PARAM(unsigned, boot_partition_size),
                                          REFERENCE_PARAM(fl_QSPIPorts, ports));

#endif
