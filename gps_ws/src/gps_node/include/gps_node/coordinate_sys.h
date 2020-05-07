#ifndef _COORDINATE_SYS_H
#define _COORDINATE_SYS_H


#ifdef __cplusplus
extern "C"
{
#endif
#include <math.h>
#include "stdio.h"
#include <stdlib.h>


//定义数组长度
#define GPS_Buffer_Length 80
#define UTCTime_Length 11
#define latitude_Length 11
#define N_S_Length 2
#define longitude_Length 12
#define E_W_Length 2 
typedef struct {
    double lng;
    double lat;
	char   devid[36];
} Location;
typedef struct SaveData 
{
	char GPS_Buffer[GPS_Buffer_Length];
	char isGetData;		//是否获取到GPS数据
	char isParseData;	//是否解析完成
	char UTCTime[UTCTime_Length];		//UTC时间
	char latitude[latitude_Length];		//纬度
	char N_S[N_S_Length];		//N/S
	char longitude[longitude_Length];		//经度
	char E_W[E_W_Length];		//E/W
	char isUsefull;		//定位信息是否有效
} _SaveData;


extern Location transformFromWGSToGCJ(Location wgLoc);
extern Location transformFromGCJToWGS(Location gcLoc);
extern Location bd_encrypt(Location gcLoc);
extern Location bd_decrypt(Location bdLoc);
extern Location WGS84tobaidu(double longti,double lati);
extern void parseGpsBuffer();
extern Location printGpsBuffer(char *did);
_SaveData Save_Data;
#ifdef __cplusplus
}
#endif
#endif
