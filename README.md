# BlueSCSIUI
UI for BlueSCSI on Amiga

Don't do anything before reading how to setup your BlueSCSI device and SD card. 
Basically you will need to setup a blank file on your SD card called NE4.hda.
https://bluescsi.com/docs/WiFi-DaynaPORT

Copy bsui and bsui.info to a directory on your Amiga (floppy or harddisk, it doesn't matter).
Edit Information against the bsui icon. Set your DEVICE to your SCSI driver. Opions can be
* scsi.device
* 1230scsi.device
* gvpscsi.device

The list goes on depending on your hardware device driver you have.
If you do not setup your driver properly then you will see an error in the bsui application and all buttons will be greyed out.
UNIT can be -1 for auto-detect or set to the unit you've configured for your BlueSCSI.

If you want to run from the command line then execute
> bsui <device name> <unit>

Run bsui and hopefully it reports a MAC address and all buttons are active. 
If this is your first time setting up then the app will ask you to configure the driver. 
Saying Yes will take you to the driver config which sets up Rob's dynaport driver. 
This replicates Rob's config on https://bluescsi.com/docs/WiFi-Amiga, but you don't need to open any files, just enter the values into the appication.
Your device and unit should auto populate to match the bsui config you set before starting up the app. Set the SSID and key to auto connect to your Wifi on startup of the driver. 
Set Auto to 1 to connect on startup. Note that mode can depend on your hardware. GVP is 2 and scsi.device is 1. Experiment if these are not working using values 0, 1 or 2.
Press SAVE to write configuration which will be permanent and USE for settings which will only apply until power cycle and restart of Workbench.

You can go back to the configuration using the menu (right mouse button) and select Configuration from the menu.

Pressing Scan in the main UI will show all discovered access points. If you don't see your Wifi then you cannot connect to it. Selecting an AP and entering a password key will connect you to the AP. Note that this is only useful if Dynaport isn't already connected via Roadshow, Miami or other network app.
ManualAP allows connection to hidden SSIDs and you will need to enter the SSID manually o connect. 

Active AP should show in UI and will periodically check this connection whilst the UI is running. 

You don't need this app to connect, but it is useful to diagnose your setup and ensure you are talking to your BlueSCSI. 
