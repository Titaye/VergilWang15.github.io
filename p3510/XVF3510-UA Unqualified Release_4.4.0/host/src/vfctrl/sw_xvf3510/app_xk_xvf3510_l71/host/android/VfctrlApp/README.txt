To build and run the XMOS Vfctrl App follow the steps below:
  1. Unzip the content of the XMOSVfctrlApp.zip archive
  2. The files extracted from the previous step contain an Android Studio project, 
     open the project from Android Studio
  3. Build and run the project as usual

On Android only one app can have access to a USB device, therefore if multiple apps require
access to the XMOS USB device, the desired app must be explicitly granted access. This can be achieved
by disconnecting and reconnecting the XMOS device and selecting the desired app when asked to:
  "Choose an app for the USB device"

The XMOS Vfctrl App waits for the KWD to boot and sends some control messages to set up the DAC, this
may take up to 12 seconds at the moment.
 
To run a command, do the following:
  1. wait for the initialization phase to complete (~12 seconds)
  2. press the button "SELECT COMMAND" and choose a command from the list
  3. if a command needs arguments, manually add them in the same text box with the command name
  4. press the button "EXECUTE COMMAND"
  5. if the command is a read command, the read value will be shown at the right of "EXECUTE COMMAND" button,
     if the command is a write command, "Command success" should be shown at the right of "EXECUTE COMMAND" button.

The command names can also be entered manually in the text box on the left, it is not required to use the "SELECT COMMAND" button.
