// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
package com.xmos;

import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.util.Log;

import java.util.HashMap;

/**
 *  Class containing the XMOS device API methods and variables
 */
public class XVF3510 {
    /**
     *  Print tag for this class
     */
    private static final String TAG = "XMOS XVF3510";

    public static native int connect(int fd);
    public static native String xmos_run_command(String[] cmd);

    static {
        System.loadLibrary("native-lib");
    }

    /**
     * XVF3510 USB Product and vendor IDs
     */
    private static int vendorId = 0x20B1;
    private static int productId = 0x0014;

    public static void setVendorId(int vid) {
        vendorId = vid;
    }

    public static void setProductId(int pid) {
        productId = pid;
    }

    public static int getProductId() {
        return productId;
    }

    public static int getVendorId() {
        return vendorId;
    }
    /**
     * Find USB XMOS device and set connection class variable
     *
     * @param       usbManager     USB manager of the activity
     * @return                     UsbDevice if device found, null otherwise
     */
    public static UsbDevice getUsbDevice(UsbManager usbManager)
    {
        HashMap<String, UsbDevice> deviceList = usbManager.getDeviceList();
        Log.i(TAG, String.format("Looking for device with vendor ID 0x%04x and product ID 0x%04x",
                vendorId, productId));
        for (UsbDevice usbDevice : deviceList.values()) {
            Log.d(TAG, deviceList.size() + " devices");
            Log.d(TAG, usbDevice.getDeviceName());
            Log.d(TAG, String.format("0x%04x 0x%04x", usbDevice.getVendorId(), usbDevice.getProductId()));

            if (usbDevice.getVendorId() == vendorId && usbDevice.getProductId() == productId) {
                Log.i(TAG, "XMOS Device found");
                return usbDevice;
            }
        }
        Log.e(TAG, "XMOS Device not found");
        return null;
    }

    /**
     * Function to check if version is correct.
     * Version is retrieved using UsbDevice.getVersion()
     *
     * @param       expectedVersion     string with expected version in format A.BC
     * @return      true if version of the device is the same as expectedVersion,
     *              false if versions are different or if the device is not set
     */
    private static boolean isVersionCorrect(UsbDevice device, String expectedVersion) {
        if (device == null) {
            Log.e(TAG, "USB device not set");
            return false;
        }
        String version = device.getVersion();
        if (expectedVersion.equals(version)) {
            Log.i(TAG, String.format("Correct firmware version: %s", expectedVersion));
        } else {
            Log.w(TAG, String.format("Unexpected firmware version: %s, expected %s", version, expectedVersion));
            return false;
        }
        return true;
    }

    static public String sendCommand(String cmd) {
        String[] params = cmd.trim().split("\\s+");

        Log.i(TAG, String.format("Send command: %s", cmd.trim()));
        return xmos_run_command(params);
    }
}
