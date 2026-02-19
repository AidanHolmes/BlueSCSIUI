#include "bsconfig.h"
#include <string.h>
#include <stdio.h>
#include <dos/dos.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <proto/exec.h>
#include <clib/dos_protos.h>
#include <proto/utility.h>
#include <stdlib.h>

#define DOSBase conf->dosbase
#define UtilityBase conf->utilitybase

static char* CONFIG_TOKENS[NUM_TOKENS] = {"DEVICE","DEVICEID","PRIORITY","MODE","AUTOCONNECT","SSID","KEY","DATASIZE","DEBUG"};

void muldiv(USHORT num, USHORT divide, USHORT* result, USHORT* mod) {
    *result = 0;
    while (num >= divide) {
        (* result)++;
        num -= divide;
    }
    *mod = num;
}

void _stoa(WORD num, char* str) 
{
    char buffer[16];
	LONG neg = 0;
    UWORD divres, divmod, number;
	char* s = buffer;
    
    neg = num < 0;
    if (neg) number = -num; else number = num;
    do {
        muldiv(number, 10, &divres, &divmod);
        *s++ = '0' + divmod;
        number = divres;
    } while (number);
    s--;

    if (neg) *str++ = '-';

    while (s>=buffer) *str++ = *s--;        
    *str++ = '\0';
}

void _ustoa(UWORD num, char* str) 
{
    char buffer[16];
    char* s = buffer;
    UWORD divres, divmod;
    do {
        muldiv(num, 10, &divres, &divmod);
        *s++ = '0' + divmod;
        num = divres;
    } while (num);
    s--;

    while (s>=buffer) *str++ = *s--;        
    *str++ = '\0';
}



LONG tokeniseSetting(char* data, char** value) {
    *value = NULL;
    while (*data) {
        if (*data == '=') {
            *data = '\0';
            *value = data+1;
            return 1;
        } 
        data++;
    }
    return 0;
}

////////////////////////////////////////////////////////
//
// Public

void strcpy_s(char* dest, char* src, UWORD maxLength) 
{
    UWORD i = strlen(src)+1;
    if (i > maxLength){ 
		i = maxLength;
	}
    memcpy(dest, src, i);
    dest[i] = '\0';
}

void rtrimValue(char *s)
{
	char *c =NULL;
	
	for (c=s;*c!='\0';c++);
	for (;c>=s;c--){
		switch(*c){
			case '\0':
			case '\n':
			case '\r':
			case ' ':
			case '\t':
				*c = '\0';
				break;
			default:
				c=s;
				break;
		}
	}
}

void applyDefaults(struct BlueScsiConfig *conf, char *device, int unit)
{
	if (device){
		strcpy(conf->deviceName, device);
	}else{
		strcpy(conf->deviceName, "scsi.device");
	}
    conf->deviceID = unit;  // 
    conf->taskPriority = 0;  // -128 to 127  - probably should be 0 but works faster set as 1!
    conf->scsiMode = 1;      // Driver mode. 0=DynaPORT, 1=24 Byte Patch (scsi.device), 2=Single Write Mode (gvpscsi.device)
    conf->autoConnect = 0;   // auto connect to the WIFI?
	conf->datasize = 8192;
	conf->debug = 0;
    strcpy(conf->ssid, "");
    strcpy(conf->key, "");
}

BOOL initBlueSCSIConfig(struct BlueScsiConfig *conf)
{	
	memset(conf, 0, sizeof(struct BlueScsiConfig));
	
	conf->dosbase = (struct Library*)OpenLibrary("dos.library", 36);
	if (!conf->dosbase){
		return FALSE ;
	}
	
	conf->utilitybase = (struct Library*)OpenLibrary("utility.library", 37);
	if (!conf->utilitybase){
		closeBlueSCSIConfig(conf);
		return FALSE ;
	}
	
	return TRUE;
}

void closeBlueSCSIConfig(struct BlueScsiConfig *conf)
{
	if (conf){
		if (conf->dosbase){
			CloseLibrary(conf->dosbase);
			conf->dosbase = NULL;
		}
		
		if (conf->utilitybase){
			CloseLibrary(conf->utilitybase);
			conf->utilitybase = NULL;
		}
		
		memset(conf, 0, sizeof(struct BlueScsiConfig));
	}
}

BOOL existsBlueSCSIConfig(void)
{
	BPTR fh1=0, fh2=0;
	BOOL ret = TRUE ;
	
	fh1=Open("ENV:scsidayna.prefs",MODE_OLDFILE);
	fh2=Open("ENVARC:scsidayna.prefs",MODE_OLDFILE);

	if (!fh1 && !fh2){
		ret=FALSE;
	}
	if (fh1){
		Close(fh1);
	}
	if (fh2){
		Close(fh2);
	}
	return ret;
}

BOOL loadBlueSCSIConfig(struct BlueScsiConfig *conf, char *device, int unit) 
{
	char buffer[128];
	UWORD matches = 0;
    BOOL modeConfigured = FALSE;
    BPTR fh;
	char* value;
	UWORD token = 0;
	
	// Apply defaults before reading config.
	// Config will then override any settings in the file and leave others as default
	applyDefaults(conf,device,unit);
    
	if (fh = Open("ENV:scsidayna.prefs",MODE_OLDFILE)) {
        while (FGets(fh, buffer, 128)) {
            if (tokeniseSetting(buffer, &value)) {
                // find a match
                for (token = 0; token < NUM_TOKENS; token++) {
                    matches++;
                    if (Stricmp(CONFIG_TOKENS[token], buffer) == 0) {
                        switch (token) {
                            case 0: strcpy_s(conf->deviceName, value,108); rtrimValue(conf->deviceName); break;
                            case 1: conf->deviceID = atoi(value); break;
                            case 2: conf->taskPriority = atoi(value); 
                                    if (conf->taskPriority>127) conf->taskPriority = 127;
                                    if (conf->taskPriority<-128) conf->taskPriority = -128;
                                    break;
                            case 3: conf->scsiMode = (UWORD)atoi(value); 
                                    if (conf->scsiMode>2) conf->scsiMode=2;
                                    modeConfigured = TRUE;
                                    break;
                            case 4: conf->autoConnect = atoi(value); break;
                            case 5: strcpy_s(conf->ssid, value, 64);rtrimValue(conf->ssid); break;
                            case 6: strcpy_s(conf->key, value, 64); rtrimValue(conf->key); break;
							case 7: conf->datasize = atoi(value); break;
							case 8: conf->debug = atoi(value); break;
                            default: matches--; break;
                        }
                        break;
                    }
                }
            }
        }
        Close(fh);
    }

    // If no mode was set, but a GVP device was specified then jump to mode 2. It will default to 1 anyway
    if (!modeConfigured) {
		if ((ToUpper(conf->deviceName[0]) == 'G') && (ToUpper(conf->deviceName[1]) == 'V') && (ToUpper(conf->deviceName[2]) == 'P')){ 
			conf->scsiMode = 2;
		}
    }

    return (BOOL)((matches > 0)?TRUE:FALSE);
}

// Saves settings back to ENV or ENVARC
BOOL saveBlueSCSIConfig(struct BlueScsiConfig *conf, BOOL saveToENV) 
{
	UWORD token = 0;
	BOOL good = FALSE ;
	char tmp[20];  // temp buffer
    BPTR fh;
	
    if (fh = Open(saveToENV ? "ENV:scsidayna.prefs" : "ENVARC:scsidayna.prefs",MODE_NEWFILE)) {
        // Save each setting in turn
        good = TRUE;
        for (token = 0; token < NUM_TOKENS; token++) {
            if (!FPuts(fh, CONFIG_TOKENS[token])) good = FALSE;
            if (!FPuts(fh, "=")) good = FALSE;
            switch (token) {
                case 0:  if (!FPuts(fh, conf->deviceName)) good = FALSE; break;
                case 1:  _stoa(conf->deviceID, tmp);  if (!FPuts(fh, tmp)) good = FALSE; break;
                case 2:  _stoa(conf->taskPriority, tmp);  if (!FPuts(fh, tmp)) good = FALSE; break;
                case 3:  _ustoa(conf->scsiMode, tmp);  if (!FPuts(fh, tmp)) good = FALSE; break;
                case 4:  _ustoa(conf->autoConnect, tmp);  if (!FPuts(fh, tmp)) good = FALSE; break;
                case 5:  if (!FPuts(fh, conf->ssid)) good = FALSE; break;
                case 6:  if (!FPuts(fh, conf->key)) good = FALSE; break;
				case 7:  _ustoa(conf->datasize, tmp);  if (!FPuts(fh, tmp)) good = FALSE; break;
				case 8:  _ustoa(conf->debug, tmp);  if (!FPuts(fh, tmp)) good = FALSE; break;
            }
            if (!FPuts(fh, "\n")) good = FALSE;
        }

        Close(fh);
        return good;
    }
    return FALSE;
}