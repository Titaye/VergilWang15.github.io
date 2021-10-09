Data Partition Generator
========================

Typical usage
-------------

Typical usage when factory programming

1) To generate a JSON description out of a long binary file, you could do:

    xxd -i a.bin | grep 0x > a.json

where a.json will have the appropriate lines before and after added manually
such as:

    { "compatibility_version": "3.0.0",
      "items": [
        {"type": 2, "bytes": [
          ...
        ]}
      ]
    }

2) Run the generator utility on the JSON description:

    data_partition_generator --regular-sector-size 4096 \
      --hardware-build 0x12345678 --spi-spec-bin spispec.bin \
      --factory a.json -o data.bin

where spispec.bin was created from text specification using another utility

3) Program the flash by doing:

    xflash --factory app.xe --boot-partition-size 1048576 --data data.bin

Usage is similar for creating an upgrade image, except hardware build and SPI
specification are omitted
