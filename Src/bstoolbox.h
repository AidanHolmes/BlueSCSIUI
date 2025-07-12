#ifndef __BSTOOLBOX_HDR
#define __BSTOOLBOX_HDR

#include <proto/exec.h>
#include <exec/types.h>
#include <exec/devices.h>

#define MAX_SCAN_ENTRIES 10

struct SCSIWifiEntry
{
	char SSID[64];
	UBYTE BSSID[6];
	BYTE RSSI;
	UBYTE channel;
	UBYTE flags;
	UBYTE _padding;
};

struct SCSIWifiInfo 
{
	UWORD length;
	char SSID[64]; 
	UBYTE BSSID[6]; 
	UBYTE rssi;     
	UBYTE channel;
	UBYTE flags;
	UBYTE _padding;
};

struct SCSIWifiEntryList
{
	UWORD size;
	struct SCSIWifiEntry results[MAX_SCAN_ENTRIES];
};

struct blueWifi
{
	struct IOStdReq *ior;
	struct MsgPort *port;
	BOOL enabled;
	UBYTE mac[8];
	struct SCSIWifiEntryList scanresults ;
	struct SCSIWifiInfo info;
};

BOOL query_inquiry_dayna(struct blueWifi *bw);
BOOL openBlueWifi(struct blueWifi *bw, char *deviceName, UWORD port);
void closeBlueWifi(struct blueWifi *bw);
BOOL waitTimeOut(ULONG sec);
BOOL enable_wifi(struct blueWifi *bw, BOOL enable);
BOOL query_macaddress(struct blueWifi *bw);
BOOL wifi_scan(struct blueWifi *bw);
BOOL wifi_networkinfo(struct blueWifi *bw);
BOOL wifi_connect(struct blueWifi *bw, char *szSSID, char *szKey);








#endif