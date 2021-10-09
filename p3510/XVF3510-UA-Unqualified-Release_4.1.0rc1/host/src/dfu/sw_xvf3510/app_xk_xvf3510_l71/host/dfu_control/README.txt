DFU Control Utility
===================

Operations, commands and requests
---------------------------------

DFU utility performs *operations* such as get-version and write-upgrade

USB DFU specification defines *requests* such as DETACH, DNLOAD or UPLOAD

A DFU utility operation break down into a sequence of commands, most of which
are requests (eg DNLOAD and GETSTATUS) with a few non-request commands (eg
GET_VERSION)

USB and I2C transport
---------------------

Two separate binary executables are provided, dfu_usb and dfu_i2c

The USB version looks up device on the bus by vendor ID and product ID, having
first checked that those match DFU suffix embedded in supplied upgrade images

The I2C version uses address to target the correct device on the bus, and
issues explicit control commands to read vendor and product ID for suffix
verification
