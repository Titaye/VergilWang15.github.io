// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __quadflash_extra_h__
#define __quadflash_extra_h__

#include <xccompat.h>
#ifdef __XC__
#define UNSAFE unsafe
#else
#define UNSAFE
#endif

#include <quadflash.h>

extern const fl_QuadDeviceSpec * UNSAFE g_flashAccess;

typedef fl_BootImageInfo fl_DataImageInfo;

/**
 * Connect function with optional bypass of manufacturer ID check
 *
 * Works the same as regular fl_connectToDevice except that:
 *
 * - only one specification is provided, for convenience where only one matters
 * - manufacturer ID check bypassed if device ID in specification is set to zero
 *
 * \param ports            Structure of ports to use
 * \param spec             Flash specification to use
 *
 * \return                 Zero for success or a non-zero error code
 */
int fl_connectToOneDevice(REFERENCE_PARAM(fl_QSPIPorts, ports),
                          REFERENCE_PARAM(fl_QuadDeviceSpec, spec));

/**
 * Send an erase command to the flash
 *
 * Normally used as an asynchronous operation that completes once status
 * bit of SR register is cleared (typically bit 0 out of SR read command 0x05).
 * Flash requires write enable be issued first.
 *
 * An asynchronous erase is useful simply because erase is slow (eg 4 seconds
 * to erase 256KB)
 *
 * \param cmd              Erase command (typically 0x20)
 * \param sectorAddress    Start of sector (byte address)
 */
void fl_int_eraseSector(unsigned char cmd, unsigned int sectorAddress);

int fl_getSectorContaining(unsigned address);

int fl_getSectorAtOrAfter(unsigned address);

unsigned fl_roundAddressUpToWholeSector(unsigned address);

/**
 * Take data partition base, skip hardware build section and read header
 * of a potential data image. Check tag word in header and validate checksum.
 * If all good, an image has been found and information will be returned in
 * a structure. Extract compatibility version from image header.
 *
 * \param dataImageInfo    Resulting image information - start byte adress,
 *                         byte size and compatibility version (factory
 *                         field set to 1)
 *
 * \return                 Zero for success or a non-zero error code
 */
int fl_getFactoryDataImageFullChecksum(REFERENCE_PARAM(fl_DataImageInfo,
                                       dataImageInfo));

/**
 * Start with image information passed in and look for another image.
 * Take the image passed in, start address add image size and sector align.
 * Read image header, check tag word and validate checksum. Continue searching
 * for images by sectory padding after each data image with a valid header but
 * bad checksum.
 *
 * \param dataImageInfo    Information about image to start searching from
 *                         as well as resulting image found, if any (factory
 *                         field set to 0)
 *
 * \return                 Zero for success or a non-zero error code
 */
int fl_getNextDataImageFullChecksum(REFERENCE_PARAM(fl_DataImageInfo,
                                    dataImageInfo));

/**
 * Version of standard fl_getFactoryImage that does a full image checksum
 * rather than a first page checksum only.
 *
 * This version is application specific, with the following differences to
 * standard fl_getFactoryImage:
 *
 * - no pre-13 tools handling
 * - no secure boot support
 *
 * \param bootImageInfo    Image information
 *
 * \return                 Zero for success or a non-zero error code
 */
int fl_getFactoryImageFullChecksum(REFERENCE_PARAM(fl_BootImageInfo,
                                   bootImageInfo));

/**
 * Version of standard fl_getNextBootImage that does a full image checksum
 * rather than a first page checksum only.
 *
 * Application specific, so secure boot not supported
 * This version is application specific, with the following differences to
 * standard fl_getFactoryImage:
 *
 * - no pre-13 tools handling
 * - no secure boot support
 * - simplified scanning, tested for single upgrade slot only
 *
 * \param bootImageInfo    Image information
 *
 * \return                 Zero for success or a non-zero error code
 */
int fl_getNextBootImageFullChecksum(REFERENCE_PARAM(fl_BootImageInfo,
                                    bootImageInfo));

/**
 * Take data partition base, skip hardware build section and read header
 * of a potential data image. Check tag word in header. Ignore checksum.
 * If tag is good, assume an image has been found and return information in
 * a structure. Extract compatibility version from image header.
 *
 * \param dataImageInfo    Resulting image information - start byte adress,
 *                         byte size and compatibility version (factory
 *                         field set to 1)
 *
 * \return                 Zero for success or a non-zero error code
 */
int fl_getFactoryDataImageNoChecksum(REFERENCE_PARAM(fl_DataImageInfo,
                                     dataImageInfo));

/**
 * Start with image information passed in and look for another image.
 * Take the image passed in, start address add image size and sector align.
 * Read image header and check tag word. Ignore checksum. 
 *
 * \param dataImageInfo    Information about image to start searching from
 *                         as well as resulting image found, if any (factory
 *                         field set to 0)
 *
 * \return                 Zero for success or a non-zero error code
 */
int fl_getNextDataImageNoChecksum(REFERENCE_PARAM(fl_DataImageInfo,
                                  dataImageInfo));

/**
 * Send a write command to the flash, along with data to write
 *
 * Normally used as an asynchronous operation that completes once status
 * bit of SR register is cleared (typically bit 0 out of SR read command 0x05).
 * Flash will apply special handling on non-page aligned writes (eg wrap inside
 * page), so most commonly this is used as a single page write. The area must
 * have been erased first. Flash requires write enable be issued first.
 *
 * An asynchronous write is useful simply because writing is much slower than
 * reading
 *
 * \param cmd              Write command (typically 0x02)
 * \param pageAddress      Start location (byte address)
 * \param data             Data bytes to send
 * \param num_bytes        Number of bytes to send
 */
void fl_int_write(unsigned char cmd,
                  unsigned int pageAddress,
#ifdef __XC__
                  const unsigned char data[num_bytes],
#else
                  const unsigned char data[],
#endif
                  unsigned int num_bytes);

/**
 * Read from flash
 *
 * Does not perform aditional nibble byte swapping so if you were to read boot
 * partition with it, you need to swap nibbles of each byte (like as various
 * quadflash functions). Remember the convention in xflash is to write boot
 * partition nibble swapped (but not the data partition).
 *
 * \param cmd              Read command (typically 0xEB)
 * \param pageAddress      Start location (byte address)
 * \param data             Memory destination
 * \param num_bytes        Number of bytes to read
 */
void fl_int_read(unsigned char cmd,
                 unsigned int address,
#ifdef __XC__
                 unsigned char destination[num_bytes],
#else
                 unsigned char destination[],
#endif
                 unsigned int num_bytes);

int fl_getSectorEndAddress(int sectorNum);

/**
 * Synchronous (block) programming of serial number in serial number section
 *
 * Maximum length is FLASH_SERIAL_MAX_CHARACTERS defined elsewhere. Function
 * will write to the appropriate location in flash, with any sector erase
 * required. For safety, any superfluous characters are trimmed off.
 *
 * Trimming is indicated by a different return code, 1.
 *
 * \param serial           Zero terminated string
 *
 * \return                 Zero or one for success, otherwise negative error
 *                         code
 */
int fl_programSerialNumber(const char serial[]);

#endif
