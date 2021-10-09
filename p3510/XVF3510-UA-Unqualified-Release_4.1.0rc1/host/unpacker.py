# Copyright (c) 2020, XMOS Ltd, All rights reserved

"""This module contains the script to unpack the packed 16kHz stereo data on one, 48kHz channel
   of a stereo wav file.
   The script requires three positional parameters, stereo 48kHz wav file, channel to unpack and output wav file name.
"""
import scipy.io.wavfile
import numpy as np
import argparse

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", nargs='?', help="48kHz stereo recorded wav file containing packed data", default='pack_pi.wav')
    parser.add_argument("channel_index", nargs='?', help="the channel index (0 or 1) to perform unpacking on", default='0')
    parser.add_argument("output", nargs='?', help="16kHz stereo unpacked output wav file", default='unpacked_output.wav')

    args = parser.parse_args()
    return args

def unpack(input_wav, packer_channel, output_wav):
    assert((packer_channel == 0) or (packer_channel == 1)), "Incorrect channel index. Needs to be either 0 or 1"
    fs, indata = scipy.io.wavfile.read(input_wav)
    packer_data = indata[:,packer_channel]
    #print(fs)
    #print(indata.dtype)
    
    start_index = 0
    found = False
    while start_index < len(packer_data)-3:
        if ((packer_data[start_index] & 0x0ff == 0) and (packer_data[start_index+1] & 0x0ff == 1) and (packer_data[start_index+2] & 0x0ff == 2)):
            found = True
            break
        start_index = start_index + 1
    if(found == False):
        print("packed data looks incorrect. Exiting\n")
        assert(False),"no packed data found in the file"

    out_data = []
    #print("start_index = {}".format(start_index))
    #print(len(packer_data))
    #round length to the nearest multiple of 3
    process_length = len(packer_data) - start_index
    process_length = int((int(process_length)/3)*3)
    for i in range(start_index, process_length, 3):
        assert (packer_data[i] & 0x0ff == 0), "marker 0 not found for index %r" % i
        assert (packer_data[i + 1] & 0x0ff == 1), "marker 1 not found for index %r" % i
        assert (packer_data[i + 2] & 0x0ff == 2), "marker 2 not found for index %r" % i
        d0 = 0
        d1 = 0
        ch0_upper24 = packer_data[i] & 0xffffff00
        ch1_upper24 = packer_data[i+1] & 0xffffff00
        ch0_lower8 = (packer_data[i + 2] >> 24) & 0x0ff
        ch1_lower8 = (packer_data[i + 2] >> 16) & 0x0ff
        d0 = ch0_upper24 | ch0_lower8
        d1 = ch1_upper24 | ch1_lower8
        out_data.append([d0,d1])
    
    nd_out = np.array(out_data, dtype=np.int32)
    #print nd_out.shape
    scipy.io.wavfile.write(output_wav, 16000, nd_out)


if __name__ == "__main__":
    args = parse_arguments()
    unpack(args.input, int(args.channel_index), args.output);
