#include "app.h"
#include "listgad.h"
#include "stringgad.h"
#include "txtgad.h"
#include "intgad.h"
#include "intuition/gadgetclass.h"
#include <exec/types.h>
#include <stdio.h>
#include <clib/icon_protos.h>
#include <clib/dos_protos.h>
#include <string.h>
#include <stdlib.h>
#include "bstoolbox.h"
#include "bsconfig.h"

#define _STR(A) #A
#define STR(A) _STR(A)

struct Library *IntuitionBase = NULL;
struct Library *IconBase = NULL;

static const char __ver[] =
  "$VER: " STR(BINNAME) " " STR(VERSIONMAJOR) "." STR(VERSIONMINOR) " (" DEVDATE ")\n";

App myApp ;
char *g_device = NULL ;
int g_device_unit = 0;
struct blueWifi g_bw;
struct SCSIWifiEntry *g_lastSelected = NULL;
struct BlueScsiConfig g_conf;
BOOL g_Once = TRUE;
BOOL g_config_changed = FALSE;

struct TextAttr topaz8 = {
   (STRPTR)"topaz.font", 8, 0, 1
};

ULONG tagDisable[] = {GA_Disabled, TRUE, TAG_END};
ULONG tagEnable[] = {GA_Disabled, FALSE, TAG_END};

ULONG strTags[] = 		{GTST_MaxChars, 64, TAG_DONE};
ULONG txtTags[] = 		{GTTX_Border, TRUE, TAG_DONE};
ULONG listTags[] = 		{GTLV_ShowSelected, 0, GA_Disabled, FALSE, TAG_DONE};
AppGadget txtMAC = 				{TEXT_KIND,{120, 30, 170, 15, (UBYTE *)"MAC", &topaz8, 2, PLACETEXT_LEFT, NULL, NULL}};
AppGadget listCtrl = 			{LISTVIEW_KIND, {120, 50, 200, 70, (UBYTE *)"Access Points", &topaz8, 1, PLACETEXT_LEFT, NULL, NULL}};
AppGadget txtConnectedSSID = 	{TEXT_KIND,{120, 130, 120, 15, (UBYTE *)"Active AP", &topaz8, 2, PLACETEXT_LEFT, NULL, NULL}};
AppGadget txtFirmware = 		{TEXT_KIND,{120, 150, 120, 15, (UBYTE *)"SCSI Device", &topaz8, 2, PLACETEXT_LEFT, NULL, NULL}};
AppGadget btnScan = 			{BUTTON_KIND,{260, 130, 60, 25, (UBYTE *)"Scan", &topaz8, 3, PLACETEXT_IN, NULL, NULL}};
AppGadget btnDisconnect = 		{BUTTON_KIND,{50, 165, 85, 25, (UBYTE *)"Disconnect", &topaz8, 5, PLACETEXT_IN, NULL, NULL}};
AppGadget btnConnect = 			{BUTTON_KIND,{260, 185, 60, 25, (UBYTE *)"Connect", &topaz8, 4, PLACETEXT_IN, NULL, NULL}};
AppGadget btnManualConnect = 	{BUTTON_KIND,{165, 185, 70, 25, (UBYTE *)"ManualAP", &topaz8, 6, PLACETEXT_IN, NULL, NULL}};
AppGadget btnConfig = 			{BUTTON_KIND,{70, 185, 70, 25, (UBYTE *)"Config", &topaz8, 7, PLACETEXT_IN, NULL, NULL}};

// Access point connection
AppGadget strSSID = 			{STRING_KIND,{70, 50, 180, 15, (UBYTE *)"SSID", &topaz8, 1, PLACETEXT_LEFT, NULL, NULL}};
AppGadget strKey = 				{STRING_KIND,{70, 70, 180, 15, (UBYTE *)"Key", &topaz8, 2, PLACETEXT_LEFT, NULL, NULL}};
AppGadget btnCancelAP = 		{BUTTON_KIND,{50, 120, 60, 25, (UBYTE *)"Cancel", &topaz8, 3, PLACETEXT_IN, NULL, NULL}};
AppGadget btnConnectAP = 		{BUTTON_KIND,{190, 120, 60, 25, (UBYTE *)"Connect", &topaz8, 4, PLACETEXT_IN, NULL, NULL}};

// Configuration
AppGadget strDevice = 			{STRING_KIND,{85, 20, 180, 15, (UBYTE *)"Device", &topaz8, 1, PLACETEXT_LEFT, NULL, NULL}};
AppGadget intUnit = 			{INTEGER_KIND,{85, 40, 180, 15, (UBYTE *)"Unit", &topaz8, 2, PLACETEXT_LEFT, NULL, NULL}};
AppGadget intTaskPriority = 	{INTEGER_KIND,{85, 60, 60, 15, (UBYTE *)"Priority", &topaz8, 3, PLACETEXT_LEFT, NULL, NULL}};
AppGadget intMode = 			{INTEGER_KIND,{85, 80, 60, 15, (UBYTE *)"Mode", &topaz8, 4, PLACETEXT_LEFT, NULL, NULL}};
AppGadget intAutoConnect = 		{INTEGER_KIND,{85, 100, 60, 15, (UBYTE *)"Auto", &topaz8, 5, PLACETEXT_LEFT, NULL, NULL}};
AppGadget strSSIDConfig = 		{STRING_KIND,{85, 120, 180, 15, (UBYTE *)"SSID", &topaz8, 6, PLACETEXT_LEFT, NULL, NULL}};
AppGadget strKeyConfig = 		{STRING_KIND,{85, 140, 180, 15, (UBYTE *)"Key", &topaz8, 7, PLACETEXT_LEFT, NULL, NULL}};
AppGadget intDataSize = 		{INTEGER_KIND,{85, 160, 180, 15, (UBYTE *)"Data Size", &topaz8, 8, PLACETEXT_LEFT, NULL, NULL}};
AppGadget intDebug = 			{INTEGER_KIND,{85, 180, 180, 15, (UBYTE *)"Debug", &topaz8, 9, PLACETEXT_LEFT, NULL, NULL}};
AppGadget btnSave = 			{BUTTON_KIND,{10, 200, 60, 20, (UBYTE *)"Save", &topaz8, 10, PLACETEXT_IN, NULL, NULL}};
AppGadget btnUse = 				{BUTTON_KIND,{120, 200, 60, 20, (UBYTE *)"Use", &topaz8, 11, PLACETEXT_IN, NULL, NULL}};
AppGadget btnCancel = 			{BUTTON_KIND,{230, 200, 60, 20, (UBYTE *)"Cancel", &topaz8, 12, PLACETEXT_IN, NULL, NULL}};

MenuSelect msEdit ;
struct NewMenu appmenu[] = {
		{ NM_TITLE, "Configuration", 0,0,0,0},
		{ NM_ITEM, "Edit...", "E",0,0,&msEdit},
		{ NM_END, 0,0,0,0,0}
};

struct Device *findDevice(char *deviceName)
{
	struct Node *n = NULL ;
	struct Device *found = NULL;
	struct Library *l = NULL;
	struct ExecBase *eb = *(struct ExecBase **)4;
	
	Forbid();
	// It shouldn't be empty, but check anyway
	if (!IsListEmpty(&eb->DeviceList)){
		for (n=eb->DeviceList.lh_Head; n->ln_Succ; n = n->ln_Succ){
			l = (struct Library*)n;
			//printf("Device %s, List Priority %d, Version %u, Revision %u, Lib IdString %s, Open Count %u\n", n->ln_Name, n->ln_Pri, l->lib_Version, l->lib_Revision, l->lib_IdString, l->lib_OpenCnt);
			if (strcmp(deviceName, n->ln_Name) == 0){
				found = (struct Device*)n;
			}
		}
	}
	Permit();
	return found;
}

char *MACtoStr(UBYTE *mac)
{
	// Format - 00:00:00:00:00:00
	static char szMac[18];
	sprintf(szMac,"%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	
	return szMac;
}

void openConfiguration(void)
{
	Wnd *childWnd;
	
	childWnd = findAppWindowName(&myApp, "config");
	if (!childWnd){
		return ;
	}
	
	loadBlueSCSIConfig(&g_conf, g_device, g_device_unit);
	
	setStringValue(strDevice, g_conf.deviceName) ;
	setIntegerValue(intUnit, g_conf.deviceID) ;
	setIntegerValue(intTaskPriority, g_conf.taskPriority) ;
	setIntegerValue(intMode, g_conf.scsiMode) ;
	setIntegerValue(intAutoConnect, g_conf.autoConnect) ;
	setIntegerValue(intDataSize, g_conf.datasize);
	setIntegerValue(intDebug, g_conf.debug);
	setStringValue(strSSIDConfig, g_conf.ssid) ;
	setStringValue(strKeyConfig, g_conf.key) ;
	
	openAppWindow(childWnd,NULL);
}

void btnConnectClicked(AppGadget *lvg, struct IntuiMessage *m)
{
	Wnd *childWnd;
	
	childWnd = findAppWindowName(&myApp, "ap");
	if (!childWnd){
		return ;
	}
	
	if (g_lastSelected){
		setStringValue(strSSID, g_lastSelected->SSID) ;
		setStringValue(strKey, "") ;
		openAppWindow(childWnd,NULL);
	}
}

void btnDisconnectClicked(AppGadget *lvg, struct IntuiMessage *m)
{
	if(wifi_connect(&g_bw, "", "")){
		setTextValue(txtConnectedSSID, g_bw.info.SSID) ;
	}
}

void btnScanClicked(AppGadget *lvg, struct IntuiMessage *m)
{
	static struct EasyStruct es = {
		sizeof(struct EasyStruct),
		0,
		"Scan Error",
		"Cannot run a wifi scan",
		"OK",
	};
	UWORD i=0;
	
	lvFreeList(&listCtrl);
	
	if (wifi_scan(&g_bw)){
		//printf("scan %u\n", g_bw.scanresults.size);
		for(;i<g_bw.scanresults.size;i++){
			lvAddEntry(&listCtrl, -1, g_bw.scanresults.results[i].SSID, &g_bw.scanresults.results[i]) ;
		}
	}else{
		//printf("scan failed\n");
		EasyRequest(getAppWnd(&myApp)->appWindow, &es, NULL);
	}
	lvUpdateList(getAppWnd(&myApp), &listCtrl);
}

void btnManualConnectClicked(AppGadget *lvg, struct IntuiMessage *m)
{
	Wnd *childWnd;
	
	childWnd = findAppWindowName(&myApp, "ap");
	if (!childWnd){
		return ;
	}

	setStringValue(strSSID, "") ;
	setStringValue(strKey, "") ;
	openAppWindow(childWnd,NULL);
}

void btnConfigClicked(AppGadget *lvg, struct IntuiMessage *m)
{
	openConfiguration();
}


void btnAPCancel(AppGadget *lvg, struct IntuiMessage *m)
{
	Wnd *childWnd;
	
	childWnd = findAppWindowName(&myApp, "ap");
	if (!childWnd){
		return ;
	}
	closeAppWindow(childWnd);
}

void btnAPConnect(AppGadget *lvg, struct IntuiMessage *m)
{
	Wnd *childWnd;
	struct EasyStruct es = {
		sizeof(struct EasyStruct),
		0,
		"Connect Error",
		"Cannot connect to %s",
		"OK",
	};
	
	childWnd = findAppWindowName(&myApp, "ap");
	if (!childWnd){
		return ;
	}
	
	if (wifi_connect(&g_bw, getStringValue(strSSID), getStringValue(strKey))){
		setTextValue(txtConnectedSSID, g_bw.info.SSID) ;
	}else{
		//printf("Cannot connect to AP %s\n", getStringValue(strSSID));
		EasyRequest(getAppWnd(&myApp)->appWindow, &es, NULL, getStringValue(strSSID));
		
	}
	closeAppWindow(childWnd);
}

void lvSelected(AppGadget *lvg, struct IntuiMessage *m)
{
	g_lastSelected = (struct SCSIWifiEntry *)lvGetListData(lvg, m->Code) ;
}

void btnCancelClick(AppGadget *lvg, struct IntuiMessage *m)
{
	Wnd *childWnd;
	
	childWnd = findAppWindowName(&myApp, "config");
	if (!childWnd){
		return ;
	}
	closeAppWindow(childWnd);
}

void saveConfigUI(struct BlueScsiConfig *conf, BOOL UseOnly)
{
	static struct EasyStruct es = {
		sizeof(struct EasyStruct),
		0,
		"Config Setup",
		"Note that scsidayna.device is running. Reboot for new changes to take effect",
		"OK",
	};
	
	strcpy_s(g_conf.deviceName, getStringValue(strDevice), 108); 
	strcpy_s(g_conf.ssid, getStringValue(strSSIDConfig), 64); 
	strcpy_s(g_conf.key, getStringValue(strKeyConfig), 64); 
	g_conf.deviceID = getIntegerValue(intUnit);
	g_conf.taskPriority = getIntegerValue(intTaskPriority);
	g_conf.scsiMode = getIntegerValue(intMode);
	g_conf.autoConnect = getIntegerValue(intAutoConnect);
	if (g_conf.autoConnect > 0){
		g_conf.autoConnect = 1;
	}
	g_conf.datasize = getIntegerValue(intDataSize);
	if (g_conf.datasize > 8192){
		g_conf.datasize = 8192;
	}
	g_conf.debug = getIntegerValue(intDebug);
	if (g_conf.debug > 0){
		g_conf.debug = 1;
	}
	saveBlueSCSIConfig(&g_conf, TRUE) ;
	if (!UseOnly){
		saveBlueSCSIConfig(&g_conf, FALSE) ;
	}
	g_config_changed = TRUE ;
	
	if (findDevice("scsidayna.device")){
		EasyRequest(getAppWnd(&myApp)->appWindow, &es, NULL);
	}
}

void btnSaveClick(AppGadget *lvg, struct IntuiMessage *m)
{
	Wnd *childWnd;
	
	childWnd = findAppWindowName(&myApp, "config");
	if (!childWnd){
		return ;
	}
	saveConfigUI(&g_conf, FALSE);
	closeAppWindow(childWnd);
}

void btnUseClick(AppGadget *lvg, struct IntuiMessage *m)
{
	Wnd *childWnd;
	
	childWnd = findAppWindowName(&myApp, "config");
	if (!childWnd){
		return ;
	}
	saveConfigUI(&g_conf, TRUE);
	closeAppWindow(childWnd);
}

void menuEdit(struct Wnd *myWnd, UWORD code)
{
	openConfiguration();
}

void checkConfig(struct Wnd *myWnd)
{
	static struct EasyStruct es = {
		sizeof(struct EasyStruct),
		0,
		"Config Setup",
		"Cannot find a config file.\nDo you want to config now?",
		"Yes|No",
	};
	if (!existsBlueSCSIConfig()){
		if (EasyRequest(getAppWnd(&myApp)->appWindow, &es, NULL) == 1){
			openConfiguration();
		}
	}
}

BOOL initBlueNetApp(void)
{	
	UWORD unit = 0;
		
	loadBlueSCSIConfig(&g_conf, g_device, g_device_unit);
	
	if (g_conf.deviceID < 0 || g_conf.deviceID > 7){
		// detect unit
		for (;unit < 20;unit++){
			if (openBlueWifi(&g_bw, g_conf.deviceName, unit)){
				if (query_inquiry_dayna(&g_bw)){
					break;
				}
				closeBlueWifi(&g_bw);
			}
		}
		if (unit == 20){
			// cannot find
			return FALSE ;
		}
	}else{
		if (!openBlueWifi(&g_bw, g_conf.deviceName, g_conf.deviceID)){
			return FALSE ;
		}
	}
	
	if (g_bw.version == 1){
		setTextValue(txtFirmware, "AmigaNET");
	}else if (g_bw.version == 0){
		setTextValue(txtFirmware, "DynaPORT");
	}else{
		setTextValue(txtFirmware, "Unknown");
	}
	
	if (!enable_wifi(&g_bw, TRUE)){
		closeBlueWifi(&g_bw);
		return FALSE ;
	}
	
	if(wifi_networkinfo(&g_bw)){
		setTextValue(txtConnectedSSID, g_bw.info.SSID) ;
	}
	
	if(query_macaddress(&g_bw)){
		setTextValue(txtMAC, MACtoStr(g_bw.mac)) ;
	}
	
	return TRUE ;
}

void closeBlueNetApp(void)
{
	setTextValue(txtMAC, "") ;
	setTextValue(txtConnectedSSID, "") ;
	setTextValue(txtFirmware, "");
	closeBlueWifi(&g_bw);
}

void handleSigs(struct App *myApp, ULONG sigs)
{
	// Check for missing config and offer to open configuration window
	if (g_Once){
		checkConfig(getAppWnd(myApp));
		g_Once=FALSE;
	}
	
	// Close connection and try again with new settings
	if (g_config_changed){
		if (g_bw.port){
			closeBlueNetApp();
		}
		g_config_changed = FALSE ;
	}
	
	// Port is valid and non NULL if open. Try to connect if NULL
	if (!g_bw.port){
		// Try connecting to app
		if (initBlueNetApp()){
			setGadgetTags(&listCtrl, listTags);
			setGadgetTags(&btnScan, tagEnable);
			setGadgetTags(&btnDisconnect, tagEnable);
			setGadgetTags(&btnConnect, tagEnable);
			setGadgetTags(&btnManualConnect, tagEnable);	
			lvFreeList(&listCtrl);
			lvUpdateList(getAppWnd(myApp), &listCtrl);
		}
	}
	
	// Check status of Port to driver. Set UI with information
	if (g_bw.port){
		if(wifi_networkinfo(&g_bw)){
			setTextValue(txtConnectedSSID, g_bw.info.SSID) ;
			//printf("Net info: %s, %02X:%02X:%02X:%02X:%02X:%02X, channel - %u, flags 0x%02X\n", g_bw.info.SSID, g_bw.info.BSSID[0],g_bw.info.BSSID[1],g_bw.info.BSSID[2],g_bw.info.BSSID[3],g_bw.info.BSSID[4],g_bw.info.BSSID[5],g_bw.info.channel, g_bw.info.flags);
		}
	}else{
		setGadgetTags(&listCtrl, listTags);
		setGadgetTags(&btnScan, tagDisable);
		setGadgetTags(&btnDisconnect, tagDisable);
		setGadgetTags(&btnConnect, tagDisable);
		setGadgetTags(&btnManualConnect, tagDisable);	
		lvFreeList(&listCtrl);
		lvAddEntry(&listCtrl, -1, "Cannot initialise", NULL) ;
		lvAddEntry(&listCtrl, -1, "check configuration", NULL) ;
		lvUpdateList(getAppWnd(myApp), &listCtrl);
	}
}

int main(int argc, char **argv)
{
	Wnd *appWnd = NULL, *childWnd = NULL, *configChildWnd = NULL ;
	struct WBStartup *WBenchMsg = NULL;
	struct WBArg *wbarg = NULL;
	struct DiskObject *dobj = NULL;
	UWORD i = 0;
	LONG olddir = -1;
	char *strToolValue ;
	int ret = 0;
	
	g_device = "scsi.device";
	g_device_unit = 4;
	// crude parameter support to set driver
	if (argc >= 3){
		g_device = argv[1];
		g_device_unit = atoi(argv[2]);
	}
	if (argc ==0){
		// Started from Workbench
		if(!(IconBase = OpenLibrary("icon.library",33))){
			return 0;
		}
		WBenchMsg = (struct WBStartup *)argv;
		for(i=0, wbarg=WBenchMsg->sm_ArgList; i < WBenchMsg->sm_NumArgs; i++, wbarg++){
			olddir = -1;
			if (wbarg->wa_Lock && *wbarg->wa_Name){
				olddir = CurrentDir(wbarg->wa_Lock);
			}
			
			if((*wbarg->wa_Name) && (dobj=GetDiskObject(wbarg->wa_Name))){
				if ((strToolValue = (char *)FindToolType((char **)dobj->do_ToolTypes, "DEVICE"))){
					g_device = strToolValue;
				}
				if ((strToolValue = (char *)FindToolType((char **)dobj->do_ToolTypes, "UNIT"))){
					g_device_unit = atoi(strToolValue);
				}
			}
			
			if (olddir == -1){
				CurrentDir(olddir);
			}
		}
	}
	
	// Setup styles
	setGadgetTags(&txtMAC, txtTags);
	setGadgetTags(&txtConnectedSSID, txtTags);
	setGadgetTags(&txtFirmware, txtTags);
	setGadgetTags(&strSSID, strTags);
	setGadgetTags(&strKey, strTags);
	setGadgetTags(&listCtrl, listTags);
	
	//setGadgetTags(&listCtrl, tagDisable);
	setGadgetTags(&btnScan, tagDisable);
	setGadgetTags(&btnDisconnect, tagDisable);
	setGadgetTags(&btnConnect, tagDisable);
	setGadgetTags(&btnManualConnect, tagDisable);
	
	setGadgetTags(&btnConfig, tagEnable);
	
	////////////////////////////////////////////
	// Init main app
	
	if ((ret=initialiseApp(&myApp)) != 0){
		return ret;
	}
	
	IntuitionBase = myApp.intu;

	appWnd = getAppWnd(&myApp);
	wndSetSize(appWnd, 350, 220);
    
	addAppGadget(appWnd, &txtMAC) ;
	addAppGadget(appWnd, &txtConnectedSSID) ;
	addAppGadget(appWnd, &txtFirmware) ;
	addAppGadget(appWnd, &btnScan);
	addAppGadget(appWnd, &btnConnect) ;
	//addAppGadget(appWnd, &btnDisconnect) ;
	addAppGadget(appWnd, &btnManualConnect) ;
	addAppGadget(appWnd, &btnConfig) ;
	addAppGadget(appWnd, &listCtrl);
	
	lvInitialise(&listCtrl);
	
	lvUpdateList(appWnd, &listCtrl);
	listCtrl.fn_gadgetUp = lvSelected;
	btnConnect.fn_gadgetUp = btnConnectClicked;
	btnDisconnect.fn_gadgetUp = btnDisconnectClicked;
	btnScan.fn_gadgetUp = btnScanClicked;
	btnManualConnect.fn_gadgetUp = btnManualConnectClicked;
	btnConfig.fn_gadgetUp = btnConfigClicked;
	msEdit.fn_menuselect = menuEdit ;
	
	appWnd->info.Title = "BlueSCSI Net Info";
	
	///////////////////////////////////////////
	// Init AP access window
	
	childWnd = addChildWnd(appWnd, "ap", "Connect to AP");
	wndSetSize(childWnd, 300, 150);
	wndSetModal(childWnd, TRUE);
	
	addAppGadget(childWnd, &strSSID);
	addAppGadget(childWnd, &strKey );
	addAppGadget(childWnd, &btnConnectAP);
	addAppGadget(childWnd, &btnCancelAP);
	
	btnCancelAP.fn_gadgetUp = btnAPCancel;
	btnConnectAP.fn_gadgetUp = btnAPConnect;
	
	//////////////////////////////////////////
	// Init Config window
	configChildWnd = addChildWnd(appWnd, "config", "Configuration");
	wndSetSize(configChildWnd, 300, 230);
	wndSetModal(configChildWnd, TRUE) ;
		
	addAppGadget(configChildWnd, &strDevice);
	addAppGadget(configChildWnd, &intUnit);
	addAppGadget(configChildWnd, &intTaskPriority);
	addAppGadget(configChildWnd, &intMode);
	addAppGadget(configChildWnd, &intAutoConnect);
	addAppGadget(configChildWnd, &strSSIDConfig);
	addAppGadget(configChildWnd, &strKeyConfig);
	addAppGadget(configChildWnd, &intDataSize);
	addAppGadget(configChildWnd, &intDebug);
	addAppGadget(configChildWnd, &btnCancel);
	addAppGadget(configChildWnd, &btnUse);
	addAppGadget(configChildWnd, &btnSave);
	
	btnCancel.fn_gadgetUp = btnCancelClick;
	btnUse.fn_gadgetUp = btnUseClick;
	btnSave.fn_gadgetUp = btnSaveClick;
	
	if (!initBlueSCSIConfig(&g_conf)){
		goto exit;
	}
	
	////////////////////////////////////////////
	// Let's Go!
	
	openAppWindow(appWnd, NULL);
	createMenu(appWnd, appmenu);

	setWakeTimer(&myApp, 2, 0);
	myApp.wake_sigs = 0;
	myApp.fn_wakeSigs = handleSigs;
	
	dispatch(&myApp);
	
exit:
	lvFreeList(&listCtrl);
	
	closeBlueNetApp();
	closeBlueSCSIConfig(&g_conf);
	
	appCleanUp(&myApp);
	if (dobj){
		FreeDiskObject(dobj);
		dobj = NULL;
	}
	CloseLibrary(IconBase);

	return(0);
}