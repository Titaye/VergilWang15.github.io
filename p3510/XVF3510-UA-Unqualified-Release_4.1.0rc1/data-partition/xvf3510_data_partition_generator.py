#!/usr/bin/env python3
# Copyright (c) 2020, XMOS Ltd, All rights reserved

""" This package contains the functions to convert a JSON config file in the format as below:

    {
        "compatibility_version": "2.2.1",
        "regular_sector_size": "4096",
        "hardware_build": "0x12345678",
        "item_files": [
            "input/xmos_usb_params.txt",
            "input/dac_cmds.txt"
        ]
    }

    into the corresponding data partition image.
    The paths in 'item_files' are relative to this file.
"""

import argparse
import platform
import json
import sys
import os
import binascii
import subprocess
import shutil
import filecmp
import re

from contextlib import contextmanager
from pathlib import Path

import flash_specification_serialiser

TAB_IN_SPACES = "    "
NUM_OF_BYTES_PER_LINE = 16

if "XMOS_ROOT" not in os.environ:
    os.environ["XMOS_ROOT"] = str(Path(__file__).resolve().parents[3])

ROOT_PATH = Path(os.environ["XMOS_ROOT"])

SW_XVF3510_PATH = ROOT_PATH / "sw_xvf3510"
DSP_HOST_PATH = SW_XVF3510_PATH / "app_xk_xvf3510_l71/host/dsp_control/"
DP_GEN_HOST_PATH = ROOT_PATH / "lib_flash_data_partition/host/data_partition_generator/"
DP_JSON_OUTPUT_PATH = SW_XVF3510_PATH / "app_xk_xvf3510_l71/data-partition/json"
DP_IMAGES_PATH = SW_XVF3510_PATH / "app_xk_xvf3510_l71/data-partition/images"
LINE_SEPARATOR = "\n------------------------------------------------------------------------------------------------------\n\n"

data_partition_gen_params = {
    "compatibility_version": "",
    "regular_sector_size" : "",
    "hardware_build" : "",
    "spispec_path": ""
}

ERROR_INVALID_CONFIG_FILE_NAME = 1
ERROR_JSON_CONFIG_FILE_NOT_FOUND = 2
ERROR_INVALID_VFCTRL_FILE_NAME = 3
ERROR_VFCTRL_HOST_APP_NOT_FOUND = 4
ERROR_DATA_PARTITION_GEN_HOST_APP_NOT_FOUND = 5
ERROR_VFCTRL_HOST_APP_VERSION_NOT_FOUND = 6
ERROR_MISSING_REGULAR_SECTOR_SIZE_FIELD = 7
ERROR_MISSING_HARDWARE_BUILD_FIELD = 8
ERROR_MISSING_SPISPEC_PATH_FIELD = 9
ERROR_SPISPEC_FILE_NOT_FOUND = 10
ERROR_INPUT_FILE_NOT_FOUND = 11
ERROR_INVALID_INPUT_FILE_EXTENSION = 12
ERROR_INVALID_CONTROL_COMMAND = 13
ERROR_VFCTRL_APP_FAILED = 14
ERROR_BUILD_COMMAND_FAILED = 15
ERROR_DATA_PARTITION_GENERATOR_FAILED = 16
ERROR_INVALID_VERSION_FORMAT = 17

def config_file_check(file_name):
    """ Lambda function to check config file name
        Args:
            file_name:  config file name
        Returns:
            config file name if check is successful
        """

    config_file_ending = ".json"
    if not file_name.endswith(config_file_ending):
        print(f"Error: config file name must end with \"{config_file_ending}\". Given file is \"{file_name}\"", file=sys.stderr)
        exit(ERROR_INVALID_CONFIG_FILE_NAME)
    return file_name

def vfctrl_file_check(file_name):
    """ Lambda function to check vfctrl host app file name
        Args:
            file_name:  config file name
        Returns:
            config file name if check is successful
        """

    vfctrl_name = "vfctrl_json"
    if vfctrl_name not in str(Path(file_name).name):
        print(f"Error: vfctrl host file name must contain \"{vfctrl_name}\". Given file is \"{file_name}\"", file=sys.stderr)
        exit(ERROR_INVALID_VFCTRL_FILE_NAME)
    return file_name

def version_format_check(version_str):
    """ Lambda function to check version format
        Args:
            version_str:  string containing version
        Returns:
            version if check is successful
        """

    if re.match(r"\d+\.\d+\.\d+", version_str):
        return version_str
    else:
        print(f"Error: forced compatibility version must have format 1.2.3. Given version is \"{version_str}\"", file=sys.stderr)
        exit(ERROR_INVALID_VERSION_FORMAT)

def parse_arguments():
    """ Parse command line arguments
        Args:
            None
        Returns:
            parsed arguments
    """

    parser = argparse.ArgumentParser()
    parser.add_argument('config_file', type=lambda s: config_file_check(s),
                        help="Config json file containing list of input files")
    parser.add_argument("--output-json-file", "-o", default=None,
                        help="Path and file name of the generated JSON file")
    parser.add_argument("--vfctrl-host-bin-path", "-c", type=lambda s: vfctrl_file_check(s), default=None,
                        help="Path to the vfctrl host app binary, if not set the host app from the release package will be used")
    parser.add_argument("--use-sandbox", "-r", action='store_true',
                        help="Do not use default binaries from release package: they will be built from the sandbox")
    parser.add_argument("--dpgen-host-bin-path", "-d", default=None,
                        help="path to the data-partition-generator host app binary, if not set the host app from the release package will be used")
    parser.add_argument("--verbose", "-v", action='store_true',
                        help="Print debug information")
    parser.add_argument('--force-compatibility-version', '-f', type=lambda s: version_format_check(s),
                        default=None, help="DEBUG argument to force a compatibility version different from the one of the VfCtrl host app. Version format is 1.2.3")


    parser.parse_args()
    parsed_args = parser.parse_args()
    return parsed_args

def get_vfctrl_app_version(bin_path, verbose):
    cmd = [bin_path, "--help"]
    if verbose:
        print("Running command: {}".format(" ".join(cmd)))
    output = ""
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if result.returncode:
        print(f"Error: VfCtrl binary returned {result.returncode}.\nOutput:\n{result.stdout}", file=sys.stderr)
        exit(ERROR_VFCTRL_APP_FAILED)
    output = result.stdout.decode("utf-8")
    for line in output.split("\n"):
        s = re.match(r".*Host app version: v(\d+\.\d+\.\d+).*", line)
        if s:
            return s.group(1)

    print(f"Error: VfCtrl app version not available.\n Command output: {output}", file=sys.stderr)
    exit(ERROR_VFCTRL_HOST_APP_VERSION_NOT_FOUND)

def set_output_json_file(input_file, compatibility_version):
    """ Create output json file name if name not given
        Args:
            input_file:             input file name with path
            compatibility_version   version of the data partition, it must match the firmware version
        Returns:
            output file name with path
    """

    output_dir = Path(__file__).resolve().parents[0] / "output"
    output_dir.mkdir(parents=True, exist_ok=True)
    input_file_stem = Path(input_file).stem
    compatibility_version_str = compatibility_version.replace(".", "_")
    output_file_name = f"output_{input_file_stem}_v{compatibility_version_str}.json"
    output_file = Path(output_dir) / output_file_name
    return output_file

@contextmanager
def pushd(new_dir):
    """ Move to a directory and return to the previous one afterwards
        Args:
            new_dir:     directory to move to
        Returns:
            None
    """

    last_dir = os.getcwd()
    os.chdir(new_dir)
    try:
        yield
    finally:
        os.chdir(last_dir)

def run_cmd(cmd):
    """ Run a command with subprocess.call and exit if return error

        Args:
            cmd:      command to run
        Returns:
            None
    """

    print("Running command: {}".format(" ".join(cmd)))
    ret = subprocess.call(cmd)
    if ret != 0:
        print("Failed to run \"{}\", error %d".format(" ".join(cmd), ret), file=sys.stderr)
        exit(ERROR_BUILD_COMMAND_FAILED)

def build_host_app(dir_path,
                   bin_name):
    """ Build a host app in the given path using cmake

        Args:
            dir_path:      directory containing the host app source files
            bin_name:      name of the binary file to build
        Returns:
            path to the newly built binary file
    """


    uname = platform.uname()[0]

    # cmakecmd must be used as a list of strings since on Windows
    # one of the argument of the command is a string with spaces
    cmakecmd = ["cmake", "."]
    makecmd = "make"
    if uname == "Windows":
        cmakecmd = ["cmake", "-G", "NMake Makefiles", "-S", ".", "-Wno-dev"]
        makecmd = "nmake"
        bin_name += ".exe"
    cmakecmd_full = cmakecmd
    if dir_path == DSP_HOST_PATH:
        cmakecmd_full += ["-DJSON=ON"]
    with pushd(dir_path):
        # remove CMakeCache.txt and CMakeFiles
        cmakecache_path = dir_path / "CMakeCache.txt"
        if cmakecache_path.is_file():
            os.remove(cmakecache_path)
        cmakefiles_path = dir_path / "CMakeFiles"
        if cmakefiles_path.is_dir():
            shutil.rmtree(cmakefiles_path, ignore_errors=True)
        bin_path = dir_path / "bin"
        if bin_path.is_dir():
            shutil.rmtree(bin_path, ignore_errors=True)

        # Make binaries
        run_cmd(cmakecmd_full)
        cmd = makecmd.split()
        run_cmd(cmd)
    bin_path_str = f"{dir_path}/bin/{bin_name}"
    return Path(bin_path_str).resolve()

def prepare_host_apps(vfctrl_host_bin_path, dpgen_host_bin_path, use_sandbox):
    """ Set the host apps paths and do checks in the following order:
        1. if paths are set just check if files exist
        2. if "use_sandbox" is true, build the host apps from the sandbox
        3. if no argument is set, use the binaries from the release package 

        Args:
            vfctrl_host_bin_path:       path to the vfctrl host app binary, if not set the host app will be built inside the sandbox
            dpgen_host_bin_path:        path to the data-partition-generator host app binary, if not set the host app will be built inside the sandbox
            use_sandbox:                 set to true if we are not running script from the release package
        Returns:
            Strings containing the paths to the VfCtrl and the data partition generator host apps
    """

    if dpgen_host_bin_path:
        dpgen_host_bin_path = Path(dpgen_host_bin_path).resolve()
    if vfctrl_host_bin_path:
        vfctrl_host_bin_path = Path(vfctrl_host_bin_path).resolve()
    # Use host apps from release package if the paths are not set
    if not use_sandbox:
        uname = platform.uname()[0]
        bin_extension = ""
        if uname == "Darwin":
            build_name = "MAC"
        elif uname == "Windows":
            build_name = "Win32"
            bin_extension = ".exe"
        elif uname == "Linux" and platform.uname()[-2] == "x86_64":
            build_name = "Linux"
        elif uname == "Linux" and platform.uname()[-2] == "armv7l":
            build_name = "Pi"
        if not vfctrl_host_bin_path:
            vfctrl_host_bin_path = Path(__file__).resolve().parents[0] / f"../host/{build_name}/bin/vfctrl_json{bin_extension}"
        if not dpgen_host_bin_path:
            dpgen_host_bin_path = Path(__file__).resolve().parents[0] / f"../host/{build_name}/bin/data_partition_generator{bin_extension}"

    # Build host apps if the paths are not set
    if not dpgen_host_bin_path:
        dpgen_host_bin_path = build_host_app(DP_GEN_HOST_PATH, "data_partition_generator")

    if not vfctrl_host_bin_path:
        vfctrl_host_bin_path = build_host_app(DSP_HOST_PATH, "vfctrl_json")

    if not vfctrl_host_bin_path.is_file():
        print(f"Error: {vfctrl_host_bin_path} not found", file=sys.stderr)
        exit(ERROR_VFCTRL_HOST_APP_NOT_FOUND)

    if not dpgen_host_bin_path.is_file():
        print(f"Error: {dpgen_host_bin_path} not found", file=sys.stderr)
        exit(ERROR_DATA_PARTITION_GEN_HOST_APP_NOT_FOUND)

    # Convert paths into strings for Windows compatibility
    return [str(Path(vfctrl_host_bin_path).absolute()), str(Path(dpgen_host_bin_path).absolute())]

def convert_vfctrl_log_to_json(txt_file_name, compatibility_version, bin_path, verbose):
    """ Convert a .txt file containing the list of AP control commands to configure the device
        into a string with the corresponding JSON items.

        Args:
            txt_file_name:              .txt file with the list of I2C commands
            compatibility_version:      version given to the script, to be checked against VfCtrl app version
            bin_path:                   path to the binary of the vfctrl host app
            verbose:                set to true to turn debug prints on
        Returns:
            String with list of converted JSON items
    """

    with open(txt_file_name) as txt_file:
        lines = txt_file.readlines()
        json_data = ""
        for line in lines:
            line = line.strip()
            if not line or line.startswith("//") or line.startswith("#"):
                # skip comments or empty lines
                continue
            if line.startswith("GET_"):
                print(f"Error: Read command {line.strip()} cannot be added to data partition", file=sys.stderr)
                exit(ERROR_INVALID_CONTROL_COMMAND)
            cmd = [bin_path]
            line_split = line.split(' ')

            # handle the commands where one of the argument is a string:
            # combine strings between 2 quotation marks
            line_split_processed = []
            new_token = ""
            string_started = False
            for token in line_split:
                if token.count("\"") == 1:
                    if not string_started:
                        new_token = token
                        string_started = True
                    else:
                        new_token += " " + token
                        string_started = False
                else:
                    if string_started:
                        new_token += " " + token
                    else:
                        new_token = token
                if not string_started:
                    line_split_processed.append(new_token)

            cmd.extend(line_split_processed)
            if verbose:
                print("Running command: {}".format(" ".join(cmd)))
            output = ""
            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            if result.returncode:
                print(f"Error: VfCtrl binary returned {result.returncode}.\nOutput:\n{result.stdout}", file=sys.stderr)
                exit(ERROR_VFCTRL_APP_FAILED)
            output = result.stdout.decode("utf-8")
            json_item_split = output.split(LINE_SEPARATOR.strip())
            if len(json_item_split) < 2:
                print(f"Error: command {line} failed. Command output: {output}", file=sys.stderr)
                exit(ERROR_INVALID_CONTROL_COMMAND)

            json_item = json_item_split[2]
            # Remove extra newlines
            json_data += json_item[2:-1]
            if verbose:
                print(output)
    return json_data

def convert_kwd_log_to_json(bin_file_name):
    """ Convert a .bin file containing the log the keyword detector boot log
        into a string containing the corresponding JSON items.

        Args:
            bin_file_name:      .bin file with the KWD log
        Returns:
            String with list of converted JSON items
    """

    with open(bin_file_name, 'rb') as bin_file:
        json_data = f"{TAB_IN_SPACES}{TAB_IN_SPACES}{{\n{TAB_IN_SPACES}{TAB_IN_SPACES}{TAB_IN_SPACES}\"type\": 2, \"bytes\": [\n"
        hex_data = binascii.hexlify(bin_file.read())
        byte_lines = f"{TAB_IN_SPACES}{TAB_IN_SPACES}{TAB_IN_SPACES}{TAB_IN_SPACES}"
        num_val = 0
        for i in range(0, len(hex_data), 2):
            val = int(hex_data[i:i+2].decode("utf-8"), 16)
            byte_lines += f"{val:3}, "
            num_val += 1
            if num_val == NUM_OF_BYTES_PER_LINE:
                byte_lines = byte_lines[:-1]
                byte_lines += f"\n{TAB_IN_SPACES}{TAB_IN_SPACES}{TAB_IN_SPACES}{TAB_IN_SPACES}"
                num_val = 0
        json_data += byte_lines[:-2] + '\n'
        json_data += f"{TAB_IN_SPACES}{TAB_IN_SPACES}{TAB_IN_SPACES}]\n{TAB_IN_SPACES}{TAB_IN_SPACES}}},\n"
    return json_data

def are_files_identical(file_src, file_dst):
    """ Compare two files and copy the first to the second if different

        Args:
            file_src:       source fie
            file_dst:       destination fie
        Returns:
            True if files are identical
    """

    ret = filecmp.cmp(file_src, file_dst)
    if not ret:
        print(f"Error: file {file_dst} must be manually updated. Check the differences with \"git diff\"")
        shutil.copyfile(file_src, file_dst)
    return ret


def run_data_partition_generator(dp_json_file,
                                 dpgen_host_bin_path,
                                 verbose):
    """ Generate a data-partition image from the JSON files containing the list of items such AP control commands and Keyword detector boot log

        Args:
            dp_json_file:           JSON file containing the list of item to convert
            dpgen_host_bin_path:    path to the data-partition-generator host app binary, if not set the host app will be built inside the sandbox
            verbose:                set to true to turn debug prints on
        Returns:
            String with list of converted JSON items
    """
    output_images_name = Path(dp_json_file).name.replace("output_", "").replace("_config", "").replace(".json", ".bin")
    output_images_path = Path(dp_json_file).parent
    output_factory_image_file = output_images_path / f"data_partition_factory_{output_images_name}"
    output_upgrade_image_file = output_images_path / f"data_partition_upgrade_{output_images_name}"

    regular_sector_size = data_partition_gen_params["regular_sector_size"]
    hardware_build = data_partition_gen_params["hardware_build"]
    spispec_path = data_partition_gen_params["spispec_path"]
    spispec_bin_path = spispec_path.replace(".spispec", ".bin")

    # convert the spispec file into a binary file
    flash_specification_serialiser.serialize(spispec_path, spispec_bin_path)

    factory_cmd =  [str(dpgen_host_bin_path), "--regular-sector-size", regular_sector_size, "--hardware-build", hardware_build, "--spi-spec-bin", spispec_bin_path, "--factory", str(dp_json_file), "-o", str(output_factory_image_file)]

    if verbose:
        factory_cmd.append("--verbose")

    if verbose:
        print("Run command {}".format(' '.join(factory_cmd)))

    result = subprocess.run(factory_cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output = result.stdout.decode("utf-8")

    if result.returncode:
        print(f"Error: data_partition_generator binary for factory image returned {result.returncode}.\nOutput:\n{output}", file=sys.stderr)
        exit(ERROR_DATA_PARTITION_GENERATOR_FAILED)
    if verbose:
        print(output)

    version_split = [int(v, 16) for v in data_partition_gen_params["compatibility_version"].split(".")]
    upgrade_version = f"0x{version_split[0]:02}{version_split[1]}{version_split[2]}"
    upgrade_cmd = [ str(dpgen_host_bin_path), "--regular-sector-size", regular_sector_size, "--upgrade", upgrade_version, str(dp_json_file), "-o", str(output_upgrade_image_file)]

    if verbose:
        upgrade_cmd.append("--verbose")

    if verbose:
        print("Run command {}".format(' '.join(upgrade_cmd)))

    result = subprocess.run(upgrade_cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output = result.stdout.decode("utf-8")

    if result.returncode:
        print(f"Error: data_partition_generator binary for upgrade image returned {result.returncode}.\nOutput:\n{output}", file=sys.stderr)
        exit(ERROR_DATA_PARTITION_GENERATOR_FAILED)
    if verbose:
        print(output)

    print(f"{LINE_SEPARATOR}Data image generation successful for {args.config_file}.\n Created files:")
    print(f" - { str(Path(dp_json_file).absolute())}")
    print(f" - { str(Path(output_factory_image_file).absolute())}")
    print(f" - { str(Path(output_upgrade_image_file).absolute())}")
    print(f"{LINE_SEPARATOR}")

def generate_data_partitions(input_config_file,
                             output_json_file,
                             vfctrl_host_bin_path,
                             dpgen_host_bin_path,
                             verbose,
                             force_compatibility_version):
    """ Generate a data-partition image for factory and one for upgrade from a JSON config file containing the input files to convert

        Args:
            input_config_file:          JSON file containing the list of input files
            output_file:                generated JSON file
            vfctrl_host_bin_path:       path to the vfctrl host app binary, if not set the host app will be built inside the sandbox
            dpgen_host_bin_path:        path to the data-partition-generator host app binary, if not set the host app will be built inside the sandbox
            verbose:                set to true to turn debug prints on
        Returns:
            String with list of converted JSON items
    """

    with open(input_config_file) as config_f:
        data = json.load(config_f)
        if not force_compatibility_version:
            compatibility_version = get_vfctrl_app_version(vfctrl_host_bin_path, verbose)
        else:
            compatibility_version = force_compatibility_version
        data_partition_gen_params["compatibility_version"] = compatibility_version
        if not output_json_file:
            output_json_file = set_output_json_file(args.config_file, compatibility_version)

    with open(output_json_file, 'w') as output_json_f:

        output_json_f.write(f"{{\n{TAB_IN_SPACES}\"compatibility_version\": \"{compatibility_version}\"")

        if "regular_sector_size" not in data:
            print(f"Error: No regular sector size in {input_config_file}", file=sys.stderr)
            exit(ERROR_MISSING_REGULAR_SECTOR_SIZE_FIELD)
        data_partition_gen_params["regular_sector_size"] = data["regular_sector_size"]

        if "hardware_build" not in data:
            print(f"Error: No hardware build in {input_config_file}", file=sys.stderr)
            exit(ERROR_MISSING_HARDWARE_BUILD_FIELD)

        data_partition_gen_params["hardware_build"] = data["hardware_build"]

        if "spispec_path" not in data:
            print(f"Error: No spispec path in {input_config_file}", file=sys.stderr)
            exit(ERROR_MISSING_SPISPEC_PATH_FIELD)
        spispec_file_path = Path(input_config_file).parent / data["spispec_path"]
        if not spispec_file_path.is_file():
            print(f"Error: Spispec file {spispec_file_path} not found", file=sys.stderr)
            exit(ERROR_SPISPEC_FILE_NOT_FOUND)
        data_partition_gen_params["spispec_path"] = str(spispec_file_path)

        if "item_files" in data:
            output_json_f.write(f",\n{TAB_IN_SPACES}\"items\": [\n")
            json_data = ""
            for item_file in data["item_files"]:
                item_file = str(Path(input_config_file).resolve().parents[0] / item_file["path"])
                print(f"{LINE_SEPARATOR}Parsing file {item_file}\n{LINE_SEPARATOR}")
                if not Path(item_file).is_file():
                    print(f"Error: Input file {item_file} not found", file=sys.stderr)
                    exit(ERROR_INPUT_FILE_NOT_FOUND)

                file_extension = item_file[-4:]
                if file_extension == '.txt':
                    json_data += convert_vfctrl_log_to_json(item_file, compatibility_version,
                                                            vfctrl_host_bin_path, verbose)
                elif file_extension == '.bin':
                    json_data += convert_kwd_log_to_json(item_file)
                else:
                    print("Error: wrong file extension {file_extension}, .txt or .bin expected", file=sys.stderr)
                    exit(ERROR_INVALID_INPUT_FILE_EXTENSION)

            # Remove last comma and trailing newlines
            json_data = json_data[:-2]
            output_json_f.write(json_data)

        output_json_f.write(f"\n{TAB_IN_SPACES}]\n}}\n")

    run_data_partition_generator(output_json_file,
                                 dpgen_host_bin_path,
                                 verbose)

if __name__ == "__main__":

    args = parse_arguments()

    if not Path(args.config_file).is_file():
        print(f"Error: {args.config_file} not found", file=sys.stderr)
        exit(ERROR_JSON_CONFIG_FILE_NOT_FOUND)

    [args.vfctrl_host_bin_path, args.dpgen_host_bin_path] = prepare_host_apps(args.vfctrl_host_bin_path, args.dpgen_host_bin_path, args.use_sandbox)

    generate_data_partitions(args.config_file,
                             args.output_json_file,
                             args.vfctrl_host_bin_path,
                             args.dpgen_host_bin_path,
                             args.verbose,
                             args.force_compatibility_version)
