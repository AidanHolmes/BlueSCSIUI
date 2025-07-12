#include "amigaui/app.h"
#include "amigaui/listgad.h"
#include "amigaui/stringgad.h"
#include "amigaui/txtgad.h"
#include "amigaui/intgad.h"
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

static const char __ver[] =
  "$VER: " BINNAME " " STR(VERSIONMAJOR) "." STR(VERSIONMINOR) " (" DEVDATE ")\n";

App myApp ;
char *g_device = NULL ;
int g_device_unit = 0;
struct blueWifi g_bw;
struct SCSIWifiEntry *g_lastSelected = NULL;
struct BlueScsiConfig g_conf;
BOOL g_Once = TRUE;

struct TextAttr topaz8 = {
   (STRPTR)"topaz.font", 8, 0, 1
};

ULONG tagDisable[] = {GA_Disabled, TRUE, TAG_END};
ULONG tagEnable[] = {GA_Disabled, FALSE, TAG_END};

ULONG strTags[] = 		{GTST_MaxChars, 64, TAG_DONE};
ULONG txtTags[] = 		{GTTX_Border, TRUE, TAG_DONE};
ULONG listTags[] = 		{GTLV_ShowSelected, NULL, GA_Disabled, FALSE, TAG_DONE};
AppGadget txtMAC = 				{TEXT_KIND,{120, 30, 170, 15, (UBYTE *)"MAC", &topaz8, 2, PLACETEXT_LEFT, NULL, NULL}};
AppGadget listCtrl = 			{LISTVIEW_KIND, {120, 50, 200, 70, (UBYTE *)"Access Points", &topaz8, 1, PLACETEXT_LEFT, NULL, NULL}};
AppGadget txtConnectedSSID = 	{TEXT_KIND,{120, 130, 120, 15, (UBYTE *)"Active AP", &topaz8, 2, PLACETEXT_LEFT, NULL, NULL}};
AppGadget btnScan = 			{BUTTON_KIND,{260, 130, 60, 25, (UBYTE *)"Scan", &topaz8, 3, PLACETEXT_IN, NULL, NULL}};
AppGadget btnDisconnect = 		{BUTTON_KIND,{50, 165, 85, 25, (UBYTE *)"Disconnect", &topaz8, 5, PLACETEXT_IN, NULL, NULL}};
AppGadget btnConnect = 			{BUTTON_KIND,{260, 165, 60, 25, (UBYTE *)"Connect", &topaz8, 4, PLACETEXT_IN, NULL, NULL}};
AppGadget btnManualConnect = 	{BUTTON_KIND,{165, 165, 70, 25, (UBYTE *)"ManualAP", &topaz8, 6, PLACETEXT_IN, NULL, NULL}};

// Access point connection
AppGadget strSSID = 			{STRING_KIND,{70, 50, 180, 15, (UBYTE *)"SSID", &topaz8, 1, PLACETEXT_LEFT, NULL, NULL}};
AppGadget strKey = 				{STRING_KIND,{70, 70, 180, 15, (UBYTE *)"Key", &topaz8, 2, PLACETEXT_LEFT, NULL, NULL}};
AppGadget btnCancelAP = 		{BUTTON_KIND,{50, 120, 60, 25, (UBYTE *)"Cancel", &topaz8, 3, PLACETEXT_IN, NULL, NULL}};
AppGadget btnConnectAP = 		{BUTTON_KIND,{190, 120, 60, 25, (UBYTE *)"Connect", &topaz8, 4, PLACETEXT_IN, NULL, NULL}};

// Configuration
AppGadget strDevice = 			{STRING_KIND,{80, 50, 180, 15, (UBYTE *)"Device", &topaz8, 1, PLACETEXT_LEFT, NULL, NULL}};
AppGadget intUnit = 			{INTEGER_KIND,{80, 70, 180, 15, (UBYTE *)"Unit", &topaz8, 2, PLACETEXT_LEFT, NULL, NULL}};
AppGadget intTaskPriority = 	{INTEGER_KIND,{80, 90, 60, 15, (UBYTE *)"Priority", &topaz8, 3, PLACETEXT_LEFT, NULL, NULL}};
AppGadget intMode = 			{INTEGER_KIND,{80, 110, 60, 15, (UBYTE *)"Mode", &topaz8, 4, PLACETEXT_LEFT, NULL, NULL}};
AppGadget intAutoConnect = 		{INTEGER_KIND,{80, 130, 60, 15, (UBYTE *)"Auto", &topaz8, 5, PLACETEXT_LEFT, NULL, NULL}};
AppGadget strSSIDConfig = 		{STRING_KIND,{80, 150, 180, 15, (UBYTE *)"SSID", &topaz8, 6, PLACETEXT_LEFT, NULL, NULL}};
AppGadget strKeyConfig = 		{STRING_KIND,{80, 170, 180, 15, (UBYTE *)"Key", &topaz8, 7, PLACETEXT_LEFT, NULL, NULL}};
AppGadget btnSave = 			{BUTTON_KIND,{10, 195, 60, 20, (UBYTE *)"Save", &topaz8, 10, PLACETEXT_IN, NULL, NULL}};
AppGadget btnUse = 				{BUTTON_KIND,{120, 195, 60, 20, (UBYTE *)"Use", &topaz8, 9, PLACETEXT_IN, NULL, NULL}};
AppGadget btnCancel = 			{BUTTON_KIND,{230, 195, 60, 20, (UBYTE *)"Cancel", &topaz8, 8, PLACETEXT_IN, NULL, NULL}};

MenuSelect msEdit ;

struct NewMenu appmenu[] = {
		{ NM_TITLE, "Configuration", 0,0,0,0},
		{ NM_ITEM, "Edit...", "E",0,0,&msEdit},
		{ NM_END, 0,0,0,0,0}
};

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
	struct SCSIWifiInfo *cache = NULL;
	UWORD i=0, entries =0;
	
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
	saveBlueSCSIConfig(&g_conf, TRUE) ;
	if (!UseOnly){
		saveBlueSCSIConfig(&g_conf, FALSE) ;
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
	setStringValue(strSSIDConfig, g_conf.ssid) ;
	setStringValue(strKeyConfig, g_conf.key) ;
	
	openAppWindow(childWnd,NULL);
	
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
			menuEdit(myWnd,0);
		}
	}
}


void handleSigs(struct App *myApp, ULONG sigs)
{
	if (g_Once){
		checkConfig(getAppWnd(myApp));
		g_Once=FALSE;
	}
	// wakes 5 sec
	if(wifi_networkinfo(&g_bw)){
		setTextValue(txtConnectedSSID, g_bw.info.SSID) ;
		//printf("Net info: %s, %02X:%02X:%02X:%02X:%02X:%02X, channel - %u, flags 0x%02X\n", g_bw.info.SSID, g_bw.info.BSSID[0],g_bw.info.BSSID[1],g_bw.info.BSSID[2],g_bw.info.BSSID[3],g_bw.info.BSSID[4],g_bw.info.BSSID[5],g_bw.info.channel, g_bw.info.flags);
	}
}

char *MACtoStr(UBYTE *mac)
{
	// Format - 00:00:00:00:00:00
	static char szMac[18];
	sprintf(szMac,"%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	
	return szMac;
}

BOOL initBlueNetApp(void)
{	
	UWORD unit = 0;
	
	if (!initBlueSCSIConfig(&g_conf)){
		return FALSE;
	}
	
	if (g_device_unit < 0){
		// detect unit
		for (;unit < 20;unit++){
			if (openBlueWifi(&g_bw, g_device, unit)){
				if (query_inquiry_dayna(&g_bw)){
					g_device_unit = unit;
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
		if (!openBlueWifi(&g_bw, g_device, g_device_unit)){
			return FALSE ;
		}
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
	closeBlueSCSIConfig(&g_conf);
	closeBlueWifi(&g_bw);
}

int main(int argc, char **argv)
{
	Wnd *appWnd = NULL, *childWnd = NULL, *configChildWnd = NULL ;
	struct WBStartup *WBenchMsg;
	struct WBArg *wbarg;
	struct DiskObject *dobj;
	UWORD i = 0;
	LONG olddir = -1;
	char *strToolValue ;
	struct Library *IconBase = NULL;
	
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
		CloseLibrary(IconBase);
	}
	
	// Setup styles
	setGadgetTags(&txtMAC, txtTags);
	setGadgetTags(&txtConnectedSSID, txtTags);
	setGadgetTags(&strSSID, strTags);
	setGadgetTags(&strKey, strTags);
	setGadgetTags(&listCtrl, listTags);
	
	//setGadgetTags(&listCtrl, tagDisable);
	setGadgetTags(&btnScan, tagDisable);
	setGadgetTags(&btnDisconnect, tagDisable);
	setGadgetTags(&btnConnect, tagDisable);
	setGadgetTags(&btnManualConnect, tagDisable);
	
	////////////////////////////////////////////
	// Init main app
	
	initialiseApp(&myApp);

	appWnd = getAppWnd(&myApp);
	wndSetSize(appWnd, 350, 200);
    
	addAppGadget(appWnd, &txtMAC) ;
	addAppGadget(appWnd, &txtConnectedSSID) ;
	addAppGadget(appWnd, &btnScan);
	addAppGadget(appWnd, &btnConnect) ;
	//addAppGadget(appWnd, &btnDisconnect) ;
	addAppGadget(appWnd, &btnManualConnect) ;
	addAppGadget(appWnd, &listCtrl);
	
	lvInitialise(&listCtrl);
	
	lvUpdateList(appWnd, &listCtrl);
	listCtrl.fn_gadgetUp = lvSelected;
	btnConnect.fn_gadgetUp = btnConnectClicked;
	btnDisconnect.fn_gadgetUp = btnDisconnectClicked;
	btnScan.fn_gadgetUp = btnScanClicked;
	btnManualConnect.fn_gadgetUp = btnManualConnectClicked;
	msEdit.fn_menuselect = menuEdit ;
	
	appWnd->info.Title = "BlueSCSI Net Info";
	
	//BT_Initialise_Dynamic_Queue(&sdp_data, sizeof(struct SDPRecord), TRUE);
	
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
	addAppGadget(configChildWnd, &btnCancel);
	addAppGadget(configChildWnd, &btnUse);
	addAppGadget(configChildWnd, &btnSave);
	
	btnCancel.fn_gadgetUp = btnCancelClick;
	btnUse.fn_gadgetUp = btnUseClick;
	btnSave.fn_gadgetUp = btnSaveClick;
	
	
	////////////////////////////////////////////
	// Let's Go!
	
	openAppWindow(appWnd, NULL);
	createMenu(appWnd, appmenu);

	if (initBlueNetApp()){
		setGadgetTags(&listCtrl, listTags);
		setGadgetTags(&btnScan, tagEnable);
		setGadgetTags(&btnDisconnect, tagEnable);
		setGadgetTags(&btnConnect, tagEnable);
		setGadgetTags(&btnManualConnect, tagEnable);
		
		myApp.wake_sigs = 0;
		myApp.wake_secs = 2;
		myApp.fn_wakeSigs = handleSigs;
	}else{
		lvAddEntry(&listCtrl, -1, "Cannot initialise device", NULL) ;
		lvUpdateList(getAppWnd(&myApp), &listCtrl);
	}	
	dispatch(&myApp);
	
	lvFreeList(&listCtrl);
	//BT_Free_Queue(&sdp_data);
	appCleanUp(&myApp);
	
	closeBlueNetApp();
	
	if (dobj){
		FreeDiskObject(dobj);
		dobj = NULL;
	}

	return(0);
}