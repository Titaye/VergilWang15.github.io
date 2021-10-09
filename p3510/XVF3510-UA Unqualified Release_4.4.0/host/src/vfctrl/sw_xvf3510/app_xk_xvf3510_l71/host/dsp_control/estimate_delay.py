#!/usr/bin/env python3
# Copyright (c) 2019, XMOS Ltd, All rights reserved

import sys
if sys.version[0] != '3':
    print("Run this script with Python 3")
    sys.exit(1)

import subprocess
import argparse
import platform
import time
import os
import vfctrl
from vfctrl import do_command

delay_ms = 150
delay_samples = (16000 * delay_ms / 1000) - 1
delay_margin = 50

def parse_arguments():
    """ Parse command line arguments
    Args:
        None
    Returns:
        parsed arguments
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('interface', choices=['usb', 'i2c'], nargs=1,
                        help="Which interface the device is connected to")
    parser.add_argument('--set', action='store_true',
                        help="This option sets the estimated delay values on the device")
    parser.parse_args()
    parsed_args = parser.parse_args()
    return parsed_args


def mode(arr):
    vals = list(set(arr))
    largest = -1
    index = -1
    for i, v in enumerate(vals):
        matches = len([match for match in arr if match == v])
        if matches > largest:
            largest = matches
            index = i
    return vals[i]


def main(set_delay):
    delay_time = (float(delay_samples) / 16000) * 1000
    print("Set delay to -{:.1f}ms".format(delay_time))
    do_command('SET_DELAY_DIRECTION', 1)
    do_command('SET_DELAY_SAMPLES', delay_samples)

    print("Enable delay estimator")
    do_command('SET_DELAY_ESTIMATOR_ENABLED', 1)
    time.sleep(0.5)
    do_command('RESET_FILTER_AEC', 1)

    print("Wait 100ms...")
    time.sleep(0.1)

    delays = []
    for i in range(15):
        delay = int(do_command('GET_DELAY_ESTIMATE'))
        delays.append(delay)
        time.sleep(0.2)

    estimated_delay = (delays[-1] - delay_samples) - delay_margin

    print("Delays: {}".format(delays))
    print("Mode(Delay): {}".format(mode(delays)))

    set_delay_samples = abs(estimated_delay)
    if set_delay_samples > delay_samples:
        set_delay_samples = delay_samples

    set_delay_direction = 0
    if estimated_delay < 0:
        set_delay_direction = 1

    if set_delay:
        print("Setting delay to {}".format(estimated_delay))
        do_command('SET_DELAY_DIRECTION', set_delay_direction)
        do_command('SET_DELAY_SAMPLES', set_delay_samples)
    else:
        print("To set delay on the device, use the following control commands:")
        print('SET_DELAY_DIRECTION {}'.format(int(set_delay_direction)))
        print('SET_DELAY_SAMPLES {}'.format(int(set_delay_samples)))
        print("Or run this script again with the --set option\n")


    print("Switch back to AEC")
    do_command('SET_DELAY_ESTIMATOR_ENABLED', 0)


if __name__ == "__main__":
    args = parse_arguments()
    vfctrl.init(args.interface[0])
    success = do_command('GET_VERSION')
    if not success:
        sys.exit(1)
    main(args.set)
