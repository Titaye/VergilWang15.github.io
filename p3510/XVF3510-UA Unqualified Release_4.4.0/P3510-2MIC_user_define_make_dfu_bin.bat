@Rem //Just add the vfctrl command in the data-partition/input/user_define_xvf3510.txt file and save. Use the cmd file directly without modifying any file name and path

cd data-partition
@Rem //Need to install python,require the use of Python3 (v3.7 onward is recommended),then set python environment or path on Win-OS.
python xvf3510_data_partition_generator.py user_define_xvf3510.json
cd ../host/Win32/bin
dfu_suffix_generator.exe 0x20B1 0x0014 ../../../data-partition/output/data_partition_upgrade_user_define_xvf3510_v4_4_0.bin app_xvf3510_usb_user_define.dfu

pause


@Rem readme& note------------------------------------------------------
