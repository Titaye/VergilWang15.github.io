# Copyright (c) 2020, XMOS Ltd, All rights reserved

"""This module contains the script to unpack the packed 16kHz stereo data on one, 48kHz channel
   of a stereo wav file.
   The script requires three positional parameters, stereo 48kHz wav file, channel to unpack and output wav file name.
"""
import argparse
import os
import sys

import numpy as np
import scipy.io.wavfile

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", nargs='?', help="48kHz stereo recorded wav file containing packed data", default='pack_pi.wav')
    parser.add_argument("output", nargs='?', help="16kHz stereo unpacked output wav file", default='unpacked.wav')
    parser.add_argument("bitres", type=int, nargs='?', help="bit-resolution of packed data (32, 24 or 16)", default=32)

    args = parser.parse_args()
    return args

def unpack(input_wav, output_wav, bitres=32):
    fs, indata = scipy.io.wavfile.read(input_wav)
    if fs != 48000:
        print(f"Packed data is always 48kHz, this wav is {fs/1000:.1f}kHz")
        sys.exit(1)

    shift = 0
    if bitres == 24:
        if indata.dtype != np.int32:
            print("24bit res file can only be packed as 32bit wav file")
            sys.exit(1)
        shift = 8
    elif bitres == 16:
        if(indata.dtype == np.int16):
            indata = indata.astype(np.int32)
            indata = indata << 16
        shift = 16

    packer_data_0 = indata[:,0]
    packer_data_1 = indata[:,1]
    mask = 1 << shift

    frame_start_idxs_0 = np.argwhere(indata[:,0] & mask)
    frame_start_idxs_1 = np.argwhere(indata[:,1] & mask)
    # Check that ch0 frame starts are the same as ch1
    if frame_start_idxs_0.shape != frame_start_idxs_1.shape or not np.all(frame_start_idxs_0 == frame_start_idxs_1):
        print("Frame indices between channels 0 and 1 don't match, creating a union...")
        # If not, create a union of the two
        frame_start_idxs = np.union1d(
            frame_start_idxs_0.flatten(),frame_start_idxs_1.flatten()
        )
    else:
        frame_start_idxs = frame_start_idxs_0
    frame_idx_diffs = np.diff(frame_start_idxs.flatten())
    incorrect_idxs = np.argwhere(frame_idx_diffs != 3)
    for i in incorrect_idxs:
        print(f"Frame corrupted @ {i[0]/fs:.2f}s (frame_num: {i[0]}), diff: {frame_idx_diffs[i][0]}")

    # Make sure frame_start_idxs never exceeds audio length
    if frame_start_idxs[-1] + 2 >= packer_data_0.shape[0]:
        frame_start_idxs = frame_start_idxs[:-1]

    out_mic_0 = packer_data_0[frame_start_idxs]
    out_ref_0 = packer_data_0[frame_start_idxs + 1]
    proc_out_0 = packer_data_0[frame_start_idxs + 2]

    out_mic_1 = packer_data_1[frame_start_idxs]
    out_ref_1 = packer_data_1[frame_start_idxs + 1]
    proc_out_1 = packer_data_1[frame_start_idxs + 2]

    out_mic = np.column_stack((out_mic_0, out_mic_1))
    out_ref = np.column_stack((out_ref_0, out_ref_1))
    proc_out = np.column_stack((proc_out_0, proc_out_1))
    min_len = min(len(out_mic), len(out_ref), len(proc_out))
    out_all = np.column_stack((out_mic[:min_len,:], out_ref[:min_len,:], proc_out[:min_len,:]))

    scipy.io.wavfile.write(output_wav, 16000, out_all)


if __name__ == "__main__":
    args = parse_arguments()
    unpack(args.input, args.output, args.bitres);
