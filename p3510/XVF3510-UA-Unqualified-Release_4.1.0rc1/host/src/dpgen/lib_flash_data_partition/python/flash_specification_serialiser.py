#!/usr/bin/env python3
# Copyright (c) 2020, XMOS Ltd, All rights reserved

""" This package contains the functions to convert a spispec file into a binary file.
    The binary file can be used in a data partition.

"""

import argparse
from pathlib import Path

def parse_arguments():
    """ Parse command line arguments
        Args:
            None
        Returns:
            parsed arguments
    """

    parser = argparse.ArgumentParser()
    parser.add_argument('spispec_file',
                        help="Spispec file to serialize")
    parser.add_argument("spispec_bin_file",
                        help="Output bin file")
    parser.add_argument("--verbose", action="store_true",
                        help="Print debug info")

    parser.parse_args()
    parsed_args = parser.parse_args()
    return parsed_args

WORD = 0
BYTE = 1
ENUM = 2
BYTES_32 = 3

ENUM_DICT = {
    "PROT_TYPE_NONE": 0,
    "PROT_TYPE_SR": 1,
    "PROT_TYPE_SECS": 2,
    "PROT_TYPE_SR_2X": 3,
    "SECTOR_LAYOUT_REGULAR": 0,
    "SECTOR_LAYOUT_IRREGULAR": 1
}

SPISPEC_ENCODINGS = [[WORD], [WORD], [WORD], [WORD],\
                     [WORD], [BYTE], [BYTE], [BYTE], \
                     [WORD], [BYTE], [WORD], [BYTE], \
                     [BYTE], [ENUM], [BYTE, BYTE, BYTE, BYTE],\
                     [WORD], [BYTE], [BYTE], [ENUM],\
                     [WORD, BYTE, BYTES_32], [BYTE],\
                     [WORD], [BYTE]]

def pad_bytes(f_handle, num_bytes, verbose):
    """ Add padding bytes to the file

        Args:
            f_handle:       handle of the file to be written
            num_bytes:      number of bytes to write
            verbose:        set to true to turn debug prints on
        Returns:
            None
    """

    if verbose:
        print(f"Padding {num_bytes} bytes")
    value = 0
    f_handle.write(value.to_bytes(num_bytes, byteorder='little'))

def serialize(spispec_file, bin_file, verbose=0):
    """ Convert the spispec file into a binary file which can included in the data parttion

        Args:
            spispec_file:   spispec file to be converted
            bin_file:       output binary file
            verbose:        set to true to turn debug prints on
        Returns:
            one
    """

    if not Path(spispec_file).is_file():
        raise ValueError(f"{spispec_file} not found")

    with open(spispec_file) as s_handle, open(bin_file, 'wb') as b_handle:
        lines = s_handle.readlines()
        line_idx = 0
        alignment = 4
        for line in lines:
            line = line.strip()
            if line and not line.startswith("//") and not line.startswith("#"):
                if verbose:
                    print(f"Parsing line: \"{line.strip()}\"")
                values_str = line.replace(" ","")
                values_str = values_str.split(",")
                value_idx = 0
                for encoding in SPISPEC_ENCODINGS[line_idx]:
                    value_str = values_str[value_idx].replace("{", "").replace("}", "")
                    bytes_to_write = 4
                    if encoding in (WORD, BYTES_32):
                        pad_bytes(b_handle, (4-alignment) % 4, verbose)
                        if encoding == BYTES_32:
                            bytes_to_write = 32
                        if value_str.startswith("0x"):
                            value = int(value_str, 16)
                        else:
                            value = int(value_str)
                        if encoding == BYTES_32 and value != 0:
                            raise ValueError("SECTOR_LAYOUT_IRREGULAR is not supported")
                        alignment = 4
                    elif encoding == BYTE:
                        if value_str.startswith("0x"):
                            value = int(value_str, 16)
                        else:
                            value = int(value_str)
                        alignment = (alignment+1) % 4
                        bytes_to_write = 1
                    elif encoding == ENUM:
                        if value_str == "SECTOR_LAYOUT_IRREGULAR":
                            raise ValueError(f"SECTOR_LAYOUT_IRREGULAR is not supported")
                        pad_bytes(b_handle, (4-alignment) % 4, verbose)
                        value = ENUM_DICT[value_str]
                        alignment = 4
                    else:
                        raise ValueError(f"Unknown encoding type {encoding}")
                    if verbose:
                        print(f"Write {bytes_to_write} byte(s) for value {value}")
                    b_handle.write(value.to_bytes(bytes_to_write, byteorder='little'))

                    value_idx += 1
                line_idx += 1
        pad_bytes(b_handle, (4-alignment) % 4, verbose)

if __name__ == "__main__":

    args = parse_arguments()
    serialize(args.spispec_file,
              args.spispec_bin_file,
              args.verbose)
