// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
package com.example.vfctrl;

import android.app.AlertDialog;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;

import com.xmos.XVF3510;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import android.util.Log;
import android.content.Context;

import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.widget.Toast;

import java.util.Arrays;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "XMOS Vfctrl";
    private static PopupMenu menuCommands;

    /**
     * USB permission to handle
     */
    private static final String ACTION_USB_PERMISSION = "com.android.example.USB_PERMISSION";

    private static PendingIntent permissionIntent = null;

    /*
     * USB variables
     */
    private static UsbManager manager = null;
    private static UsbDevice device = null;
    private static UsbDeviceConnection connection = null;

    /*
     * Show an error dialog
     *
     * @param   ErrorPrint      Error message to show
     */
    public void openErrorDialog(String ErrorPrint) {
        AlertDialog.Builder alert = new AlertDialog.Builder(this);
        Log.i(TAG, "Create error dialog");
        Log.e(TAG, ErrorPrint);
        alert.setTitle("ERROR");
        alert.setMessage(ErrorPrint);

        alert.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                finishAffinity();
            }
        });

        alert.show();
    }

    /**
     * onCreate() function
     *
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {


        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        manager = (UsbManager)getApplicationContext().getSystemService(Context.USB_SERVICE);
        assert manager != null;
        permissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);
        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        registerReceiver(usbReceiver, filter);

        Button select_cmd_button = findViewById(R.id.select_cmd);
        select_cmd_button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (connection == null) {
                    connectDevice();
                    if (connection == null) {
                        return;
                    }
                }
                //Creating the instance of PopupMenu
                if (menuCommands == null) {
                    Log.w(TAG, "Command list is empty");
                }

                //registering popup with OnMenuItemClickListener
                menuCommands.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    public boolean onMenuItemClick(MenuItem item) {
                        Toast.makeText(MainActivity.this,"You Clicked : " + item.getTitle(), Toast.LENGTH_SHORT).show();
                        EditText et = findViewById(R.id.enter_cmd);
                        et.setText(item.getTitle());
                        return true;
                    }
                });

                menuCommands.show();//showing popup menu
            }
        });

        Button enter_cmd_button = findViewById(R.id.execute_cmd);
        enter_cmd_button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (connection == null) {
                    connectDevice();
                }
                EditText editText = findViewById(R.id.print_val);
                editText.setText("", TextView.BufferType.EDITABLE);

                EditText et = findViewById(R.id.enter_cmd);
                String val = XVF3510.sendCommand(et.getText().toString());

                editText = findViewById(R.id.print_val);
                editText.setText(val, TextView.BufferType.EDITABLE);
            }
        });

    }
    @Override
    protected void  onStop() {
        super.onStop();
        menuCommands = null;
        connection = null;
        unregisterReceiver(usbReceiver);
    }

    private void connectDevice() {
        int vendorId = 0;
        int productId = 0;
        EditText editTextPid = null;
        EditText editTextVid = null;
        if (device == null) {
            try {
                editTextPid = findViewById(R.id.pid);
                productId = Integer.parseInt(editTextPid.getText().toString().replace("0x", ""), 16);

                editTextVid = findViewById(R.id.vid);
                vendorId = Integer.parseInt(editTextVid.getText().toString().replace("0x", ""), 16);
            }
            catch (NumberFormatException e) {
                openErrorDialog(String.format("Invalid string, found Vendor ID %s and Product ID %s, expected 0xAB10 format",
                        editTextVid.getText().toString(), editTextPid.getText().toString()));
                e.printStackTrace();
                return;
            }
            XVF3510.setProductId(productId);
            XVF3510.setVendorId(vendorId);

            Log.i(TAG, String.format("Selected parameters: vendor ID 0x%04x, product ID 0x%04x", vendorId, productId));
            device = XVF3510.getUsbDevice(manager);
            if (device == null) {
                openErrorDialog(String.format("Device with vendor ID 0x%04x and product ID 0x04%x not found",
                        vendorId, productId));
                return;
            }
        }

        if (connection == null) {
            if(!manager.hasPermission(device)) {
                Log.i(TAG, "Requesting permission");
                manager.requestPermission(device, permissionIntent);
            }
            else {
                Log.i(TAG, "Already have permission");
                connection = manager.openDevice(device);
                Log.i(TAG, "Opened device");
                if (connection == null) {
                    Log.e(TAG, "Cannot open XMOS USB device");
                } else {
                    Log.i(TAG, "Opened XMOS USB device");
                }

                int fileDescriptor = connection.getFileDescriptor();
                XVF3510.connect(fileDescriptor);
            }
            if (menuCommands == null) {
                Log.i(TAG, "Populating command list");
                menuCommands = new PopupMenu(MainActivity.this, findViewById(R.id.select_cmd));
                String[] cmds = XVF3510.sendCommand("--help").split("\\s+");
                Arrays.sort(cmds);
                Log.i(TAG, String.format("Commands: %s",  Arrays.deepToString(cmds)));
                for (String cmd : cmds) {
                    menuCommands.getMenu().add(cmd);
                }
            }
        }
    }

    private final BroadcastReceiver usbReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {
                    device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        // do nothing here, the device can't be selected yet
                        // the product and vendor IDs may not be updated at this stage
                        Log.i(TAG, String.format("Permission granted for device with product ID 0x%04X and vendor ID 0x04%X",
                                XVF3510.getVendorId(), XVF3510.getProductId()));
                    }
                    else {
                        openErrorDialog(String.format("Permission denied for device with product ID 0x%04X and vendor ID 0x04%X",
                                XVF3510.getVendorId(), XVF3510.getProductId()));
                        return;
                    }
                }
            }
        }
    };
}





