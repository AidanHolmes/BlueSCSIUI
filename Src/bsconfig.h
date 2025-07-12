#ifndef __BLUESCSIv2_CONFIG_H
#define __BLUESCSIv2_CONFIG_H

#include <exec/types.h>


#define NUM_TOKENS 7

struct BlueScsiConfig
{
	struct Library *dosbase;
	struct Library *utilitybase;
	char deviceName[108];
	// Device ID  
	WORD deviceID;     // if this is <0 or >7 then it auto-detects
	// Priority for the READING task
	WORD taskPriority;
	// Driver mode
	UWORD scsiMode;
	// Auto-connect to this wifi network
	UWORD autoConnect;
	char ssid[64];
	char key[64];
};

BOOL initBlueSCSIConfig(struct BlueScsiConfig *conf);
void closeBlueSCSIConfig(struct BlueScsiConfig *conf);
BOOL loadBlueSCSIConfig(struct BlueScsiConfig *conf, char *device, int unit) ;
BOOL saveBlueSCSIConfig(struct BlueScsiConfig *conf, BOOL saveToENV) ;
void applyDefaults(struct BlueScsiConfig *conf, char *device, int unit);
BOOL existsBlueSCSIConfig(void);

void strcpy_s(char* dest, char* src, UWORD maxLength) ;


















#endif