Flash Specification Serialiser
==============================

Typical usage
-------------

Starting with a text file such as:

    0,                      /* AT25SF161 - Just specify 0 as flash_id */
    256,                    /* page size */
    8192,                   /* num pages */
    3,                      /* address size */
    4,                      /* log2 clock divider */
    0x9F,                   /* QSPI_RDID */
    0,                      /* id dummy bytes */
    3,                      /* id size in bytes */
    0x1F8601,               /* device id */
    0x20,                   /* QSPI_SE */
    4096,                   /* Sector erase is always 4KB */
    0x06,                   /* QSPI_WREN */
    0x04,                   /* QSPI_WRDI */
    PROT_TYPE_SR,           /* Protection via SR */
    {{0x18,0x00},{0,0}},    /* QSPI_SP, QSPI_SU */
    0x02,                   /* QSPI_PP */
    0xEB,                   /* QSPI_READ_FAST */
    1,                      /* 1 read dummy byte */
    SECTOR_LAYOUT_REGULAR,  /* mad sectors */
    {4096,{0,{0}}},         /* regular sector sizes */
    0x05,                   /* QSPI_RDSR */
    0x01,                   /* QSPI_WRSR */
    0x01,                   /* QSPI_WIP_BIT_MASK */

The text file format is similar to that used by standard tools such as xflash,
that is a C header file populating one instance of fl_QuadDeviceSpec structure

The following creates a binary dump of the C structure using the XCore cross
compiler:

  python3 flash_specification_serialiser.py spispec.txt spispec.bin
