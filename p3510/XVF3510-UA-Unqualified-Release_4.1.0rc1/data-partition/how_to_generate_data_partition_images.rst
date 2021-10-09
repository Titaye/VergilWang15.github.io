How to generate data partition images
=====================================

To update a data partition image unzip the release package and follow the steps below:

    1. Collect the control commands and list them in a new .txt file inside *data-partition/input*, such as in the example below:

        ``SET_USB_VENDOR_ID 8369
          SET_USB_PRODUCT_ID 20
          SET_USB_START_STATUS 1``

        Single-line comments are created by beginning a line with the hash (#) or  double slash (//) characters.

    2. If you want to use a different keyword detector boot log, add the file into *data-partition/input*

    3. Open the desired *<build_type>.json* in *data-partition/* and add or remove the input files in *item_files*. The *item_files* list looks like the following:

        ``"item_files": [

            ...

            {
                "path": "input/i2s_rate_48k.txt",
                "comment": ""
            }
            ...
        ]``

       where the *path* is relative to the  *<build_type>.json* and the *comment* field is optional.
       Remember that the execution order of the commands can affect the behaviour of the device. For example commands to configure USB and I2S should be added at the beginning of the data partition.

    6. Open a terminal and go to the folder *data-partition*.

    7. Run the command:

        ``python3 xvf3510_data_partition_generator.py <build_type>.json``

        where *<build_type>.json* is the file edited in the previous steps.

**Note**: If you need to rebuild the host apps, please rebuild the ``VfCtrl JSON host app`` and the ``Data partition generator`` as described in *host/how_to_build_host_apps.rst.rst* and copy the newly generated binaries in the appropriate *host/<OS>/bin* directory.


    7. The newly generated data-partition images,one for factory and one for upgrade, will be available in the directory *data-partition/output* under the names *data_partition_factory_<build_type>.bin* and  *data_partition_upgrade_<build_type>.bin*.
       For debugging purposes you can examine the generated JSON file in the *output* directory. The name of the generated JSON file is output_*<build_type>.json*.
       This JSON file is useful to check if the new commands have been added correctly, JSON files are easier to compare than binary image files.

    8. To flash the firmware and the corresponding data partition, copy the generated  *data_partition_factory_<build_type>_v<release_version>.bin* and the desired *<build_type>.xe* file in the same directory and type from a terminal with the XMOS tools enabled :
        ``xflash --factory <build_type>.xe  --boot-partition-size 1048576 --data data_partition_factory_<build_type>_v<release_version>..bin``

       Remember to use the factory data partition binary and not the upgrade binary!

If you are running the script from a sw_xvf3510 sandbox in step 6. type the command:

        ``pipenv run python3 xvf3510_data_partition_generator.py <build_type>.json --use-sandbox``

If you prefer to use different host utilities in step 6. you run specify the paths of the desired apps via the command:

        ``python3 xvf3510_data_partition_generator.py <build_type>.json --vfctrl-host-bin-path vfctrl_bin_path --dpgen-host-bin-path dpgen_bin_path``

        *vfctrl_bin_path* and *dpgen_bin_path* are the paths to the VF control JSON app and the data-partition-generator apps included in the release package inside *host/<OS>/bin*. Modify the paths accordingly if the host app binaries are not located inside *data-partition* and add *.exe* extensions only on Windows.


