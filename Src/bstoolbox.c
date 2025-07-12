#include "bstoolbox.h"

#include <exec/devices.h>
#include <devices/serial.h>
#include <proto/exec.h>
#include <exec/exec.h>
#include <exec/errors.h>
#include <stdio.h>
#include <devices/scsidisk.h>
#include <devices/trackdisk.h>
#include <string.h>
#include <proto/timer.h>
#include <devices/timer.h>
#include <stdlib.h>

#define MAC_TO_STR(err) err: return #err; break

#define SCSI_NETWORK_WIFI_READFRAME         0x08
#define SCSI_NETWORK_WIFI_GETMACADDRESS     0x09  // gvpscsi.device doesn't like this command code
#define SCSI_NETWORK_WIFI_WRITEFRAME        0x0A
#define SCSI_NETWORK_WIFI_ADDMULTICAST      0x0D
#define SCSI_NETWORK_WIFI_ENABLE            0x0E
// These are custom extra SCSI commands specific to this device
#define SCSI_NETWORK_WIFI_CMD				0x1c	
// Sub commands
#define SCSI_NETWORK_WIFI_OPT_SCAN			0x01	 
#define SCSI_NETWORK_WIFI_OPT_COMPLETE		0x02
#define SCSI_NETWORK_WIFI_OPT_SCAN_RESULTS	0x03
#define SCSI_NETWORK_WIFI_OPT_INFO			0x04
#define SCSI_NETWORK_WIFI_OPT_JOIN			0x05
#define SCSI_NETWORK_WIFI_OPT_ALTREAD       0x08    
#define SCSI_NETWORK_WIFI_OPT_GETMACADDRESS 0x09

#define NSCMD_DEVICEQUERY 0x4000
#define NSCMD_TD_READ64	   0xc000
#define NSCMD_TD_WRITE64   0xc001
#define     NSDEVTYPE_TRACKDISK     5

struct SCSIWifiJoin 
{
	char SSID[64];
	char key[64];
	UBYTE channel;
	UBYTE _padding;
};

struct NSDeviceQueryResult
{
    /*
    ** Standard information
    */
    ULONG   DevQueryFormat;         /* this is type 0               */
    ULONG   SizeAvailable;          /* bytes available              */
 
    /*
    ** Common information (READ ONLY!)
    */
    UWORD   DeviceType;             /* what the device does         */
    UWORD   DeviceSubType;          /* depends on the main type     */
    UWORD   *SupportedCommands;     /* 0 terminated list of cmd's   */
 
    /* May be extended in the future! Check SizeAvailable! */
};

char *senseKeyStr[] = {"No-Sense", 
						"Recovered Error", 
						"Not Ready", 
						"Medium Error", 
						"Hardware Error", 
						"Illegal Request", 
						"Unit Attention", 
						"Data Protect", 
						"Blank Check", 
						"Vendor-Specific", 
						"Copy Aborted", 
						"Aborted Command", 
						"Equal", 
						"Volume Overflow", 
						"Miscompare", 
						"Reserved"};

char *errorToStr(LONG error)
{
	switch(error){
		case MAC_TO_STR(IOERR_OPENFAIL) ;
		case MAC_TO_STR(IOERR_ABORTED) ;
		case MAC_TO_STR(IOERR_NOCMD) ;
		case MAC_TO_STR(IOERR_BADLENGTH) ;
		case MAC_TO_STR(IOERR_UNITBUSY) ;
		case MAC_TO_STR(IOERR_SELFTEST) ;
		case MAC_TO_STR(TDERR_NotSpecified) ;
		case MAC_TO_STR(TDERR_NoSecHdr) ;
		case MAC_TO_STR(TDERR_BadSecPreamble) ;
		case MAC_TO_STR(TDERR_BadSecID) ;
		case MAC_TO_STR(TDERR_BadHdrSum) ;
		case MAC_TO_STR(TDERR_BadSecSum) ;
		case MAC_TO_STR(TDERR_TooFewSecs) ;
		case MAC_TO_STR(TDERR_BadSecHdr) ;
		case MAC_TO_STR(TDERR_SeekError) ;
		case MAC_TO_STR(TDERR_PostReset) ;
		case MAC_TO_STR(TDERR_WriteProt) ;
		case MAC_TO_STR(TDERR_DiskChanged) ;
		case MAC_TO_STR(TDERR_NoMem) ;
		case MAC_TO_STR(TDERR_BadUnitNum) ;
		case MAC_TO_STR(TDERR_DriveInUse) ;
		case MAC_TO_STR(HFERR_SelfUnit) ;
		case MAC_TO_STR(HFERR_DMA) ;
		case MAC_TO_STR(HFERR_Phase) ;
		case MAC_TO_STR(HFERR_Parity) ;
		case MAC_TO_STR(HFERR_SelTimeout) ;
		case MAC_TO_STR(HFERR_BadStatus) ;
		case MAC_TO_STR(HFERR_NoBoard) ;
		default:
			break;
	}
	return "Unknown error" ;
}

char *cmdToStr(UWORD cmdid)
{
	switch(cmdid){
		case MAC_TO_STR(NSCMD_DEVICEQUERY) ;
		case MAC_TO_STR(NSCMD_TD_READ64) ;
		case MAC_TO_STR(NSCMD_TD_WRITE64) ;
		case MAC_TO_STR(HD_SCSICMD) ;
		case MAC_TO_STR(CMD_RESET) ;
		case MAC_TO_STR(CMD_READ) ;
		case MAC_TO_STR(CMD_WRITE) ;
		case MAC_TO_STR(CMD_UPDATE) ;
		case MAC_TO_STR(CMD_CLEAR) ;
		case MAC_TO_STR(CMD_STOP) ;
		case MAC_TO_STR(CMD_START) ;
		case MAC_TO_STR(CMD_FLUSH) ;
		case MAC_TO_STR(TD_MOTOR) ;
		case MAC_TO_STR(TD_SEEK) ;
		case MAC_TO_STR(TD_FORMAT) ;
		case MAC_TO_STR(TD_REMOVE) ;
		case MAC_TO_STR(TD_RAWREAD) ;
		case MAC_TO_STR(TD_RAWWRITE) ;
		case MAC_TO_STR(TD_GETNUMTRACKS) ;
		case MAC_TO_STR(TD_CHANGENUM) ;
		case MAC_TO_STR(TD_CHANGESTATE) ;
		case MAC_TO_STR(TD_PROTSTATUS) ;
		case MAC_TO_STR(TD_GETDRIVETYPE) ;
		case MAC_TO_STR(TD_ADDCHANGEINT) ;
		case MAC_TO_STR(TD_REMCHANGEINT) ;
		case MAC_TO_STR(TD_GETGEOMETRY) ;
		case MAC_TO_STR(TD_EJECT) ;
		case MAC_TO_STR(TD_LASTCOMM) ;
		case MAC_TO_STR(ETD_READ) ;
		case MAC_TO_STR(ETD_WRITE) ;
		case MAC_TO_STR(ETD_UPDATE) ;
		case MAC_TO_STR(ETD_MOTOR) ;
		case MAC_TO_STR(ETD_SEEK) ;
		case MAC_TO_STR(ETD_FORMAT) ;
		case MAC_TO_STR(ETD_CLEAR) ;
		case MAC_TO_STR(ETD_RAWREAD) ;
		case MAC_TO_STR(ETD_RAWWRITE) ;
		default:
			break;
	}
	return "Unknown command" ;
}

BOOL directSCSI(struct IOStdReq *ior, UBYTE *cmd, UWORD cmdlen, UBYTE *buffer, ULONG *length, BOOL bWrite)
{
	LONG error = 0;
	UBYTE  Sense[20], scsiflags = SCSIF_AUTOSENSE ; 
	struct SCSICmd Cmd;
	
	memset(&Sense[0], 0, 20);
	
	Cmd.scsi_Data = (UWORD *)buffer;        /* where we put mode sense data */
	Cmd.scsi_Length = *length;                  /* how much we will accept      */
	Cmd.scsi_CmdLength = cmdlen;                 /* length of the command        */

	if (bWrite){
		scsiflags |= SCSIF_WRITE;
	}else{
		scsiflags |= SCSIF_READ;
	}
	Cmd.scsi_Flags = scsiflags;	
	Cmd.scsi_SenseData = (UBYTE *)Sense;    /* where sense data will go     */
	Cmd.scsi_SenseLength = 20;              /* how much we will accept      */
	Cmd.scsi_SenseActual = 0;               /* how much has been received   */

	Cmd.scsi_Command=cmd;/* issuing a command     */
	
	ior->io_Length  = sizeof(struct SCSICmd);
	ior->io_Data    = (APTR)&Cmd;
	ior->io_Command = HD_SCSICMD;
	
	// DoIO - it has an issue with quick commands, try with SendIO
	error = DoIO((struct IORequest *)ior);
	//SendIO((struct IORequest *)ior);
	//error = WaitIO((struct IORequest *)ior);
	
	*length = Cmd.scsi_Actual ;
	
	if (error != 0){
		switch(error){
			case IOERR_NOCMD:
				printf("HD_SCSICMD not supported for this device\n") ;
				break;
			case HFERR_BadStatus:
				printf("WaitIO failed with Bad Status\n") ;
				
				break;
			default:
				printf("HD_SCSICMD error executing request %02Xh. Error (%d) %s\n", Cmd.scsi_Command[0], error, errorToStr(error)) ;
				break;
		}
		return FALSE;
	}else if (Cmd.scsi_Status){
		// SCSI error 
		printf("Command 0x%02X failed...\n", Cmd.scsi_Command[0]) ;
		printf("WaitIO failed with Bad Status\n") ;

		return FALSE;
	}
	return TRUE ;
}

BOOL query_inquiry_dayna(struct blueWifi *bw)
{
	UBYTE buffer[64];  
	ULONG length = 64;
	static UBYTE Inquiry[]={ 0x12,0,0,0,64,0 };
	BOOL ret = TRUE ;
		
	if ((ret = directSCSI(bw->ior, Inquiry, sizeof(Inquiry), buffer, &length, FALSE))){
		if (memcmp(&buffer[8],"Dayna", 5) == 0){
			return TRUE ;
		}
	}
	return FALSE ;
}

BOOL enable_wifi(struct blueWifi *bw, BOOL enable)
{
	ULONG len = 0;
	BOOL ret = TRUE ;
	static UBYTE Wifi[]={ SCSI_NETWORK_WIFI_ENABLE, 0,0,0,0,0};
	Wifi[5] = enable?0x80:0;
	
	if ((ret=directSCSI(bw->ior, Wifi, sizeof(Wifi), NULL, &len, FALSE))){
		bw->enabled = enable ;
	}
	return ret ;
}

BOOL query_macaddress(struct blueWifi *bw)
{
	UBYTE MAC[18]; 
	BOOL ret = TRUE ;
	ULONG len =0;
	int i=0;

	static UBYTE MacAddress2[]={ SCSI_NETWORK_WIFI_CMD, SCSI_NETWORK_WIFI_OPT_GETMACADDRESS,0,0,0,0};
	static UBYTE MacAddress[]={ SCSI_NETWORK_WIFI_GETMACADDRESS, 0,0,0,0,0};
	
	len= 18;
	ret = directSCSI(bw->ior, MacAddress, sizeof(MacAddress), MAC, &len, FALSE);
	
	if (ret){
		// data ok
		memcpy(bw->mac, MAC, 6) ;
		return ret;
	}
	
	len= 6;
	ret = directSCSI(bw->ior, MacAddress2, sizeof(MacAddress2), MAC, &len, FALSE);
	
	if (ret){
		// data ok
		memcpy(bw->mac, MAC, 6) ;
	}
	
	return ret;
}

BOOL waitTimeOut(ULONG sec)
{
	struct MsgPort *p;
	struct IORequest *io;
	struct Device *TimerBase ;
	BYTE error ;
	
	if ((p = CreateMsgPort())){
        if ((io = CreateIORequest(p, sizeof(struct timerequest)))){
            if (OpenDevice("timer.device", UNIT_MICROHZ, io, 0) > 0){
				DeleteIORequest(io);
				DeleteMsgPort(p);
				return FALSE ;
			}
        }else{
			DeleteMsgPort(p);
			return FALSE ;
        }
    }else{
		return FALSE ;
    }
	
	TimerBase = io->io_Device;   

	io->io_Command = TR_ADDREQUEST;
	((struct timerequest*)io)->tr_time.tv_secs = sec;
	((struct timerequest*)io)->tr_time.tv_micro = 0;
	SendIO(io) ;
	error = WaitIO(io); // wait
	
	CloseDevice(io);
	DeleteIORequest(io);
	DeleteMsgPort(p);
	if (error >0){
		return FALSE;
	}
	return TRUE ;
}

BOOL wifi_scan(struct blueWifi *bw)
{
	/*
	
struct SCSIWifiEntry{
	char SSID[64];
	char BSSID[64];
	char RSSI;
	UBYTE channel;
	UBYTE flags;
	UBYTE _padding;
};
*/ 
	BOOL ret = TRUE ;
	ULONG len =0;
	UWORD scanresults = 0;
	struct SCSIWifiEntry *wifientry;
	UBYTE WifiData[4];
	int i=0;
	const int retries = 5;
	static UBYTE WifiScan[]={ SCSI_NETWORK_WIFI_CMD, SCSI_NETWORK_WIFI_OPT_SCAN,0,0,0,0};
	static UBYTE WifiComplete[]={ SCSI_NETWORK_WIFI_CMD, SCSI_NETWORK_WIFI_OPT_COMPLETE,0,0,0,0};
	static UBYTE WifiResults[]={ SCSI_NETWORK_WIFI_CMD, SCSI_NETWORK_WIFI_OPT_SCAN_RESULTS,0,0,0,0};
	
	len= 4;
	if (!(ret = directSCSI(bw->ior, WifiScan, sizeof(WifiScan), WifiData, &len, FALSE))){
		//printf("WifiScan request failed\n");
		goto wifiexit;
	}
	if (len != 1 || (BYTE)WifiData[0] < 0){
		ret = FALSE ;
		//printf("WifiScan data len or data wrong. len %u, WifiData[0] 0x%02X\n", len, WifiData[0]);
		goto wifiexit;
	}

	i = 0 ;
	do{
		len= 4;
		waitTimeOut(2);
		if (!(ret = directSCSI(bw->ior, WifiComplete, sizeof(WifiComplete), WifiData, &len, FALSE))){
			//printf("WifiComplete request failed\n");
			goto wifiexit;
		}
		if (len != 1){
			ret = FALSE;
			//printf("WifiComplete len failed with value %u\n", len);
			goto wifiexit;
		}
		if (WifiData[0] == 1){
			break;
		}
		i++ ;
	}while(i < retries);
	
	if (i == retries){
		ret = FALSE;
		//printf("WifiComplete time out\n");
		goto wifiexit;
	}

	len = sizeof(struct SCSIWifiEntryList);
	WifiResults[3] = len >> 8;
	WifiResults[4] = len & 0xFF;
	if ((ret = directSCSI(bw->ior, WifiResults, sizeof(WifiResults), (UBYTE*)&bw->scanresults, &len, FALSE))){

		if (len < 2){
			ret = FALSE ;
			//printf("WifiResults len incorrect %u\n", len);
			goto wifiexit;
		}
		
		bw->scanresults.size = bw->scanresults.size / sizeof(struct SCSIWifiEntry);
	}else{
		//printf("WifiResults failed\n");
	}

wifiexit:
	return ret;
}

BOOL wifi_networkinfo(struct blueWifi *bw)
{
	/*
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
	*/
	BOOL ret = FALSE ;
	ULONG len =0;
	static UBYTE WifiInfo[]={ SCSI_NETWORK_WIFI_CMD, SCSI_NETWORK_WIFI_OPT_INFO,0,0,0,0};
	
	memset(&bw->info, 0, sizeof(struct SCSIWifiInfo));
	
	len = sizeof(struct SCSIWifiInfo);
	if (!(ret = directSCSI(bw->ior, WifiInfo, sizeof(WifiInfo), (UBYTE*)&bw->info, &len, FALSE))){
		goto wifiexit;
	}
	
	if ((bw->info.length +2) != sizeof(struct SCSIWifiInfo) && (bw->info.length +2) != len){
		goto wifiexit;
	}
	
	ret = TRUE ;
	
wifiexit:
	return ret;
}

BOOL wifi_connect(struct blueWifi *bw, char *szSSID, char *szKey)
{
	/*
	struct SCSIWifiJoin {
	char SSID[64];
	char key[64];
	UBYTE channel;
	UBYTE _padding;
	};
*/
	BOOL ret = TRUE ;
	ULONG len =0;
	int i = 0;
	struct SCSIWifiJoin wifiap;
	const int retries = 5;
	struct SCSIWifiInfo info ;
	static UBYTE WifiJoin[]={ SCSI_NETWORK_WIFI_CMD, SCSI_NETWORK_WIFI_OPT_JOIN,0,0,0,0};
	
	WifiJoin[3] = sizeof(struct SCSIWifiJoin) >> 8;
	WifiJoin[4] = sizeof(struct SCSIWifiJoin) & 0xFF;
	
	memset(&wifiap, 0, sizeof(struct SCSIWifiJoin));
	
	if (strlen(szSSID) > 63){
		goto wifiexit;
	}
	strcpy(wifiap.SSID, szSSID);

	if (strlen(szKey) > 63){
		goto wifiexit;
	}
	strcpy(wifiap.key, szKey);

	len= sizeof(struct SCSIWifiJoin) ;
	if (!(ret = directSCSI(bw->ior, WifiJoin, sizeof(WifiJoin), (UBYTE*)&wifiap, &len, TRUE))){
		goto wifiexit;
	}
	
	for (i=0; i<retries; i++){
		waitTimeOut(2);
		if (!(ret=wifi_networkinfo(bw))){
			goto wifiexit;
		}
		if (info.SSID[0] != '\0'){
			break;
		}
	}
	
	if (i == retries){
		ret = FALSE;
		goto wifiexit;
	}

wifiexit:
	return ret;
}

BOOL openBlueWifi(struct blueWifi *bw, char *deviceName, UWORD port)
{
	memset(bw,0,sizeof(struct blueWifi));

	if (!(bw->port = CreateMsgPort())){
		return FALSE;
	}
	if (!(bw->ior = (struct IOStdReq *)CreateIORequest(bw->port, sizeof(struct IOStdReq)+128))){
		DeleteMsgPort(bw->port);
		bw->port = NULL ;
		return FALSE ;
	}
	if (OpenDevice(deviceName,port,(struct IORequest*)bw->ior,0) != 0){
		DeleteIORequest((struct IORequest *)bw->ior);
		DeleteMsgPort(bw->port);
		bw->port = NULL;
		bw->ior = NULL;
		return FALSE ;
	}
	return TRUE ;
}

void closeBlueWifi(struct blueWifi *bw)
{
	if (bw->ior){
		CloseDevice((struct IORequest *)bw->ior);
		DeleteIORequest((struct IORequest *)bw->ior);
		DeleteMsgPort(bw->port);
	}
	memset(bw,0,sizeof(struct blueWifi));
}
