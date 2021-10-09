#!/usr/bin/env python3
# Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
"""This module contains the script to plot the AEC filter coefficients
   for the range from the max negative delay to the max positive delay
   plus the length of the AEC filter.
   All the generated files, including the plots, are stored in the ./output folder

   The script requires one positional parameter (usb or i2c) to select
   the control interface and one optional parameter(--build-host)
   to add the steps of building the host application.
   The script requires a valid tools setup.
   The host app will be built by the script
   The script exits with the possible error codes:
       1: unsupported platform
       2: host app can't be built or it is missing
       3: control interface not available on the running platform
       4: device cannot be found
       5: command failed
"""

import sys
if sys.version[0] != '3':
    print("Run this script with Python 3")
    sys.exit(1)

import os
import time
import shutil
import argparse
import traceback
import numpy as np
import matplotlib.pyplot as plt
import vfctrl

DIVIDER_STRING = "\n---------------------------------------------------------\n"
CONVERGE_TIME_SEC = 10
MAX_DELAY_SAMPLES = 2399
SAMPLE_RATE = 16000
OUTPUT_DIR = os.path.normpath('./output/')
OUTPUT_DIR = os.path.join(OUTPUT_DIR, '')
CONTROL_RETRIES = 10
CONTROL_RETRY_WAIT_SEC = 0.1

def set_delay_parameters(delay_samples):
    """Configure the delay parameters to test

    Args:
        delay_samples: number of delay samples to test

    Returns:
        number of delay samples and the delay direction
    """

    delay_dir = 0
    if delay_samples < 0:
        delay_dir = 1
    if delay_samples > MAX_DELAY_SAMPLES:
        overlapping_samples = delay_samples-MAX_DELAY_SAMPLES
        delay_samples = MAX_DELAY_SAMPLES
    delay_samples = abs(delay_samples)
    print("Getting coefficients for delay samples {} and direction {}". \
           format(delay_samples, delay_dir))
    vfctrl.do_command("SET_DELAY_DIRECTION", delay_dir)
    vfctrl.do_command("SET_DELAY_SAMPLES", delay_samples)
    vfctrl.do_command("RESET_FILTER_AEC", 0)
    return (delay_samples, delay_dir)

def compute_all_h_hat(H_hat, h_hat_ir_all, overlapping_samples):
    """Compute the impulse responses for all the channels

    Args:
        H_hat: H_hat is an array internal to the aec with a shape as follows:
                    (y_channel_count, x_channel_count, max_phase_count, f_bin_count)
        h_hat_ir_all: impulse responses of all the channels
        overlapping_samples: samples of the h_hat_ir_all to discard since they are overlapping

    Returns:
        Impulse response of h_hat for all the channels and for the tested interval
    """

    y_channel_count = H_hat.shape[0]
    x_channel_count = H_hat.shape[1]

    for y_ch in range(y_channel_count):
        for x_ch in range(x_channel_count):
            h_hat_ir_all[y_ch][x_ch] = np.concatenate((h_hat_ir_all[y_ch][x_ch], \
                    get_h_hat_impulse_response(H_hat, y_ch, x_ch)[overlapping_samples:]), axis=None)
    return h_hat_ir_all

def get_h_hat_impulse_response(H_hat, y_channel, x_channel):
    """Gets the impulse response of h_hat.

    Args:
        H_hat: H_hat array
        y_channel: y_channel to plot
        x_channel: x_channel to plot

    Returns:
        Impulse response of h_hat for channel pair (y_channel, x_channel)
    """

    y_channel_count = H_hat.shape[0]
    x_channel_count = H_hat.shape[1]
    max_phase_count = H_hat.shape[2]
    f_bin_count     = H_hat.shape[3]
    frame_advance = 240
    h_hat_ir = np.zeros((max_phase_count * frame_advance,))

    for phase in range(max_phase_count):
        phase_ir = np.fft.irfft(H_hat[y_channel][x_channel][phase])
        start = frame_advance * phase
        end = frame_advance * (phase + 1)
        h_hat_ir[start:end] = phase_ir[:frame_advance]

    return h_hat_ir

def plot_h_hat(h_hat_ir, x_ch_num, y_ch_num, delay=0):
    """Plot the impulse responses of all the channels

    Args:
        h_hat_ir: impulse responses to plot
        x_ch_num: number of x channels
        y_ch_num: number of y channels

    Returns:
        List of max values and respective sample indexes
    """

    plt.figure(figsize=(10, 10))
    max_values = [[[] for x in range(x_ch_num)] for y in range(y_ch_num)]
    for y_ch in range(y_ch_num):
        for x_ch in range(x_ch_num):
            #h_hat_ir = get_h_hat_impulse_response(h_hat, y_ch, x_ch)
            h_hat_ir_abs = np.absolute(h_hat_ir[y_ch][x_ch])
            max_values[y_ch][x_ch] = (h_hat_ir_abs.argmax(), h_hat_ir_abs.max())
            print("Max value for ch_y {} and ch_x {} are ({}, {})".format(y_ch, x_ch,
                                                                  max_values[y_ch][x_ch][0],
                                                                  max_values[y_ch][x_ch][1]))
            N = len(h_hat_ir[y_ch][x_ch])
            start_x = 1000 * delay / SAMPLE_RATE
            x = np.linspace(start_x, 1000 * N / SAMPLE_RATE - abs(start_x), N)

            #plt.subplot(y_ch_num, x_ch_num, 1 + y_ch * x_ch_num + x_ch)
            plt.plot(x, h_hat_ir[y_ch][x_ch], label='mic {} ref {}'.format(y_ch, x_ch))
    plt.title("AEC filter coefficients")
    plt.ylabel("Amplitude")
    plt.xlabel("ms")
    plt.legend()

    plt.tight_layout()
    output_filename = OUTPUT_DIR+"aec_coeffs.png"
    plt.savefig(output_filename)
    print("Plot saved to {}".format(output_filename))
    plt.show()

    return max_values

def estimate_delay(max_values, x_ch_num, y_ch_num, set_delay=False,
                   print_delay_info=True):
    """Analyze the max values and print the recommended delay values

    Args:
        max_values: list of max values and respective samples indexes

    Returns:
        None
    """

    recommended_delay_samples = 0
    for y_ch in range(y_ch_num):
        for x_ch in range(x_ch_num):
            recommended_delay_samples += max_values[y_ch][x_ch][0]
    recommended_delay_samples /= (y_ch_num*x_ch_num)

    for y_ch in range(y_ch_num):
        for x_ch in range(x_ch_num):
            m_x = max_values[y_ch][x_ch][0]
            if max_values[y_ch][x_ch][1] == 0:
                print("Error: max value is 0, check that the reference is correctly played into the device")
                print_delay_info = False


            if abs(m_x - recommended_delay_samples) > 0.1*abs(recommended_delay_samples):
                print("Warning: delay for y_ch{}, x_ch{} differs from the average: {} vs {} relative to {} samples". \
                        format(y_ch, x_ch, m_x, recommended_delay_samples, -MAX_DELAY_SAMPLES))

    recommended_delay_samples -= MAX_DELAY_SAMPLES + 40
    if recommended_delay_samples < -MAX_DELAY_SAMPLES:
        recommended_delay_samples = MAX_DELAY_SAMPLES
    recommended_delay_dir = 0
    if recommended_delay_samples < 0:
        recommended_delay_samples = -recommended_delay_samples
        recommended_delay_dir = 1
    if set_delay:
        print("Setting optimal delay values on the device:")
        vfctrl.do_command('SET_DELAY_DIRECTION', recommended_delay_dir)
        vfctrl.do_command('SET_DELAY_SAMPLES', recommended_delay_samples)
    elif print_delay_info:
        print("Recommended delay settings are:")
        print("\tSET_DELAY_DIRECTION {}".format(recommended_delay_dir))
        print("\tSET_DELAY_SAMPLES {:.0f}".format(recommended_delay_samples))


def parse_arguments():
    parser = argparse.ArgumentParser(description='Script to fetch and plot the AEC filter coefficients')
    parser.add_argument('interface', choices=['usb', 'i2c'], nargs=1,
                        help="Which interface the device is connected to")
    parser.add_argument('--build-host', action='store_true',
                        help='flag to build the host app automatically')
    parser.add_argument('--full', action='store_true',
                        help='Plot the AEC at all possible delay values')
    parser.add_argument('--set-delay', action='store_true',
                        help='Use the AEC coefficients to set the delay on the device')
    args = parser.parse_args()
    if args.set_delay and not args.full:
        args.full = True
        print("--set-delay option used, automatically setting --full")
    return args


def get_filter_coefficients(delay_samples=None):
    if not delay_samples is None:
        (delay_samples, delay_dir) = set_delay_parameters(delay_samples)
        print("Wait {} seconds to allow AEC to converge".format(CONVERGE_TIME_SEC))
        time.sleep(CONVERGE_TIME_SEC)

    try:
        os.remove("aec_coefficients.py")
    except OSError:
        pass

    print("Running GET_FILTER_COEFFICIENTS_AEC...")
    vfctrl.do_command("GET_FILTER_COEFFICIENTS_AEC")

    # H_hat values are stored in coefficients.py, reload them with exec()
    # Defines the following values:
    #   frame_advance
    #   y_channel_count
    #   x_channel_count
    #   max_phase_count
    #   f_bin_count
    #   H_hat
    namespace = {}
    with open('aec_coefficients.py') as f:
        exec(f.read(), globals(), namespace)

    if not delay_samples is None:
        output_filename = OUTPUT_DIR + "coefficients_w_delay_samples_{}_dir_{}.py"\
                            .format(delay_samples, delay_dir)
        shutil.copy('aec_coefficients.py', output_filename)

    return namespace['H_hat']


def main(args):
    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    print("Run script on {} interface".format(args.interface[0]))

    vfctrl.init(args.interface[0], build=args.build_host)

    print(DIVIDER_STRING)
    print("Get AEC parameters")
    start_delay = -MAX_DELAY_SAMPLES
    try:
        frame_advance = int(vfctrl.do_command("GET_FRAME_ADVANCE_AEC"))
    except Exception as e:
        print(str(e))
        print("Error: check that the device is connected and correctly flashed")
        exit(4)

    x_ch_num = int(vfctrl.do_command("GET_X_CHANNELS_AEC"))
    y_ch_num = int(vfctrl.do_command("GET_Y_CHANNELS_AEC"))
    phases_num = int(vfctrl.do_command("GET_X_CHANNEL_PHASES_AEC").decode().lstrip().split(" ")[0])
    filter_len_samples = frame_advance*phases_num
    h_hat_ir = [[[np.zeros(0)] for x in range(x_ch_num)] for y in range(y_ch_num)]
    print(DIVIDER_STRING)

    if args.full:
        original_delay_samples = int(vfctrl.do_command('GET_DELAY_SAMPLES'))
        original_delay_direction = int(vfctrl.do_command('GET_DELAY_DIRECTION'))
        for delay_samples in range(-MAX_DELAY_SAMPLES,
                                   MAX_DELAY_SAMPLES+filter_len_samples,
                                   filter_len_samples):
            H_hat = get_filter_coefficients(delay_samples)
            h_hat_ir = compute_all_h_hat(H_hat, h_hat_ir, overlapping_samples=0)
            print(DIVIDER_STRING)
    else:
        H_hat = get_filter_coefficients(None)
        h_hat_ir = compute_all_h_hat(H_hat, h_hat_ir, overlapping_samples=0)

    max_values = [[[] for x in range(x_ch_num)] for y in range(y_ch_num)]
    for y_ch in range(y_ch_num):
        for x_ch in range(x_ch_num):
            h_hat_ir_abs = np.absolute(h_hat_ir[y_ch][x_ch])
            max_values[y_ch][x_ch] = (h_hat_ir_abs.argmax(), h_hat_ir_abs.max())

    if args.full:
        print(DIVIDER_STRING)
        estimate_delay(max_values, x_ch_num, y_ch_num, set_delay=args.set_delay)

    if args.full and not args.set_delay:
        # Reset delay to original values and reset filter
        vfctrl.do_command('SET_DELAY_SAMPLES', original_delay_samples, quiet=True)
        vfctrl.do_command('SET_DELAY_DIRECTION', original_delay_direction, quiet=True)
        vfctrl.do_command("RESET_FILTER_AEC", 0)

    print("Plot AEC filter coefficients")
    delay = 0
    if args.full:
        delay = -MAX_DELAY_SAMPLES
    plot_h_hat(h_hat_ir, x_ch_num, y_ch_num, delay)


if __name__ == "__main__":
    args = parse_arguments()
    main(args)

