# Copyright (c) 2019-2020, XMOS Ltd, All rights reserved

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import platform
import os
import sys
import subprocess
from subprocess import Popen, PIPE
import time

TIMEOUT = 3

bin_path = None

usb_bin = 'vfctrl_usb'
i2c_bin = 'vfctrl_i2c'

system = platform.uname()[0]
machine = platform.uname()[4]



control_retries = 10
control_retry_wait = 0.1


def init(interface, custom_bin=None, build=False):
    """ Sets the path to the binary

    Args:
        interface: control interface to use
        custom_bin: optional parameter to use a custom build

    Returns:
        None
    """

    if build:
        build_host_app()

    global bin_path
    if custom_bin is not None:
        bin_path = custom_bin
        return
    bin_path = 'bin/{}'
    if interface == 'usb':
        bin_path = bin_path.format(usb_bin)
    elif interface == 'i2c':
        if system != 'Linux' or machine != 'armv7l':
            print("Error: i2c control interface not supported on this machine")
            exit(3)
        bin_path = bin_path.format(i2c_bin)
    if not os.path.isfile(bin_path):
        bin_path = bin_path[len('bin/'):]
        if not os.path.isfile(bin_path):
            print("Error: Host app binary not found")
            exit(2)
    bin_path = os.path.abspath(bin_path)
    ret = do_command('GET_VERSION', quiet=True)
    if not ret:
        print("Error: Device not responding")
        sys.exit(1)
    print("Using the host app binary {}".format(bin_path))


def command_timeout(done_event):
    time.sleep(TIMEOUT)
    if not done_event.isSet():
        print("This command is taking a while...")
        print("NOTE: Control will hang if I2S audio is not playing/recording.");


def do_command(cmd_id, *args, **kwargs):
    """Run a control parameter and return the values in case of GET_ commands

    Args:
        cmd: parameter command
        args: additional values

    Returns:
        None or values of GET_ commands
    """

    quiet = False
    try:
        if kwargs['quiet']:
            quiet = True
    except KeyError:
        pass

    if bin_path is None:
        raise Exception("vfctrl not initialised.")
    cmd = [bin_path, cmd_id] + [str(a) for a in args]
    for i in range(control_retries):
        try:
            output = b''
            p = Popen(cmd, stdout=PIPE, bufsize=1)
            with p.stdout:
                for line in iter(p.stdout.readline, b''):
                    line = line
                    output += line
                    if not quiet:
                        print(line.decode(), end='')
            p.wait() # wait for the subprocess to exit
            break
        except subprocess.CalledProcessError:
            print("ERROR")
            output = None
            time.sleep(control_retry_wait)
    if output is None:
        print("Error executing command: {}".format(cmd_id))
        return False
    return output[len(cmd_id)+1:]

def build_host_app():
    """Build the host app for the given platform

    Args:
        None

    Returns:
        None
    """
    i2c = False
    #on rpi, build for I2C. TODO: this needs to change to enable linux build on RPi.
    if system == 'Linux':
        if machine == 'armv7l':
            i2c = True

    if system == 'Windows':
        cmake_cmd = "cmake -G \"NMake Makefiles\" -S . -Wno-dev"
    elif i2c == False:
        cmake_cmd = "cmake ."
    else:
        cmake_cmd = "cmake . -DI2C=ON"
    make_cmd = 'make'

    if system == 'Windows':
        assert False, "cmake based vfctrl app build not supported on windows" 

    cmd = cmake_cmd
    proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    if proc.returncode != 0:
        print("Error running: {}".format(cmd))
        print(stdout, stderr)
        exit(5)

    cmd = make_cmd
    proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    if proc.returncode != 0:
        print("Error running: {}".format(cmd))
        print(stdout, stderr)
        exit(5)
