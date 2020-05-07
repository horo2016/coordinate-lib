
/**
 *Created by Wandergis on 2015/7/8.
 * 提供了百度坐标（BD09）、国测局坐标（火星坐标，GCJ02）、和WGS84坐标系之间的转换
 */

#include "coordinate_sys.h"
#include <math.h>
#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#define true 1
#define false 0
_SaveData Save_Data;
Location LocationMake(double lng, double lat) {
    Location loc; loc.lng = lng, loc.lat = lat; return loc;
}



const double pi = 3.14159265358979324;

//
// Krasovsky 1940
//
// a = 6378245.0, 1/f = 298.3
// b = a * (1 - f)
// ee = (a^2 - b^2) / a^2;
const double a = 6378245.0;
const double ee = 0.00669342162296594323;

int outOfChina(double lat, double lon)
{
    if (lon < 72.004 || lon > 137.8347)
        return 1;
    if (lat < 0.8293 || lat > 55.8271)
        return 1;
    return 0;
}

double transformLat(double x, double y)
{
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(x > 0 ? x:-x);
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 *sin(2.0 * x * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
    ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi / 30.0)) * 2.0 / 3.0;
    return ret;
}

double transformLon(double x, double y)
{
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(x > 0 ? x:-x);
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
    ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;
    return ret;
}
/**
 * WGS84转GCj02
 * @param lng
 * @param lat
 * @returns {*[]}
 */
 ///
///  Transform WGS-84 to GCJ-02 (Chinese encrypted coordination system)
///
Location transformFromWGSToGCJ(Location wgLoc)
{
    Location mgLoc;
    if (outOfChina(wgLoc.lat, wgLoc.lng))
    {
        mgLoc = wgLoc;
        return mgLoc;
    }
    double dLat = transformLat(wgLoc.lng - 105.0, wgLoc.lat - 35.0);
    double dLon = transformLon(wgLoc.lng - 105.0, wgLoc.lat - 35.0);
    double radLat = wgLoc.lat / 180.0 * pi;
    double magic = sin(radLat);
    magic = 1 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
    dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
    mgLoc.lat = wgLoc.lat + dLat;
    mgLoc.lng = wgLoc.lng + dLon;
    
    return mgLoc;
}
 /**
 * GCJ02 转换为 WGS84
 * @param lng
 * @param lat
 * @returns {*[]}
 */
///
///  Transform GCJ-02 to WGS-84
///  Reverse of transformFromWGSToGC() by iteration.
///
///  Created by Fengzee (fengzee@fengzee.me).
///
Location transformFromGCJToWGS(Location gcLoc)
{
    Location wgLoc = gcLoc;
    Location currGcLoc, dLoc;
    while (1) {
        currGcLoc = transformFromWGSToGCJ(wgLoc);
        dLoc.lat = gcLoc.lat - currGcLoc.lat;
        dLoc.lng = gcLoc.lng - currGcLoc.lng;
        if (fabs(dLoc.lat) < 1e-7 && fabs(dLoc.lng) < 1e-7) {  // 1e-7 ~ centimeter level accuracy
          // Result of experiment:
          //   Most of the time 2 iterations would be enough for an 1e-8 accuracy (milimeter level).
          //
            return wgLoc;
        }
        wgLoc.lat += dLoc.lat;
        wgLoc.lng += dLoc.lng;
    }
    
    return wgLoc;
}
/**
 * 火星坐标系 (GCJ-02) 与百度坐标系 (BD-09) 的转换
 * 即谷歌、高德 转 百度
 * @param lng
 * @param lat
 * @returns {*[]}
 */
///
///  Transform GCJ-02 to BD-09
///
const double x_pi = 3.14159265358979324 * 3000.0 / 180.0;
Location bd_encrypt(Location gcLoc)
{
    double x = gcLoc.lng, y = gcLoc.lat;
    double z = sqrt(x * x + y * y) + 0.00002 * sin(y * x_pi);
    double theta = atan2(y, x) + 0.000003 * cos(x * x_pi);
    return LocationMake(z * cos(theta) + 0.0065, z * sin(theta) + 0.006);
}
/**
 * 百度坐标系 (BD-09) 与 火星坐标系 (GCJ-02)的转换
 * 即 百度 转 谷歌、高德
 * @param bd_lon
 * @param bd_lat
 * @returns {*[]}
 */
///
///  Transform BD-09 to GCJ-02
///
Location bd_decrypt(Location bdLoc)
{
    double x = bdLoc.lng - 0.0065, y = bdLoc.lat - 0.006;
    double z = sqrt(x * x + y * y) - 0.00002 * sin(y * x_pi);
    double theta = atan2(y, x) - 0.000003 * cos(x * x_pi);
    return LocationMake(z * cos(theta), z * sin(theta));
}
///
///  ublox nmea gprmc  角度转角   11613.57409---->116.1957409
///
double degree_minute2dec_degrees(double deg)
{
    deg = (deg / 100.0 - (int)deg / 100) * 100.0 / 60.0 + (int)deg / 100 ;
    return deg;
}

Location WGS84tobaidu(double longti,double lati)
{
	Location my_wgLoc,out_wgloc;
	//my_wgLoc.lng = degree_minute2dec_degrees(11613.57409);
	//my_wgLoc.lat = degree_minute2dec_degrees(4005.20173);

	my_wgLoc.lng = degree_minute2dec_degrees(longti);
	my_wgLoc.lat = degree_minute2dec_degrees(lati);

	out_wgloc= bd_encrypt(transformFromWGSToGCJ(my_wgLoc));
	printf("%.6f\t  %.6f\t",out_wgloc.lng,out_wgloc.lat); 
	
	return out_wgloc;
	
}

Location loc_coor;
long int get_lati;
long int  get_longti;//local get



#define MQTT_SIP "gpscar.xiaovdiy.cn"
int Creatstatejson(Location gpsval)
{
    char tmp_buf[0xff]={0};
	char id_buf[32]={0};
    char topic_buf[0xff]={0};
    char  value_buf[0xff]={0};
	char value_buff[0xff];
    memcpy(tmp_buf,gpsval.devid,16);
	memset(id_buf,0,32);
	memcpy(id_buf,gpsval.devid,16);
    memcpy(tmp_buf+16,"/state/gps",sizeof("/state/gps"));
  //  sprintf(tmp_buf,"%s/state/gps",chargename);
    cJSON * root =  cJSON_CreateObject();
   if(!root) {
          printf("get root faild !\n");
     }else printf( "get root success!\n");
    cJSON_AddItemToObject(root, "\"type\"", cJSON_CreateNumber(0));//?ù?úμ???ìí?ó
    cJSON_AddItemToObject(root, "\"devid\"", cJSON_CreateString(id_buf));
    cJSON_AddItemToObject(root, "\"isvalid\"", cJSON_CreateNumber(1));
  //  cJSON_AddItemToObject(root, "GPS", item);//root?úμ???ìí?ósemantic?úμ?  
    cJSON_AddItemToObject(root, "\"lonti\"", cJSON_CreateNumber(gpsval.lng));//ìí?óname?úμ?
    cJSON_AddItemToObject(root, "\"lati\"",cJSON_CreateNumber(gpsval.lat));//ìí?óname?úμ?
   // printf("%s\n", cJSON_Print(root));
   // DEBUG(LOG_DEBUG,"topic:%s ,mes:%s\n",tmp_buf,value_buf);
   // mqtt_publish(tmp_buf,cJSON_Print(root));
	memcpy(value_buf,cJSON_Print(root),strlen(cJSON_Print(root)));
	
  //  memcpy(publishstring.pTopic,tmp_buf,strlen(tmp_buf));//PUBLISH_STR publishstring;
 //    memcpy(publishstring.pMessage,value_buf,strlen(value_buf));
 //    publishstring.publish_sendflg =1;
	//mqtt_publish(tmp_buf,value_buf);
	cJSON_Delete(root);
	 sprintf(value_buff,"mosquitto_pub -t %s -h %s -m \"%s\"",tmp_buf,MQTT_SIP,value_buf);
	 system(value_buff);
//  cJSON_Delete(item);
    return 0;
}


//打印发送数据
Location printGpsBuffer(char *did)
{
	if (Save_Data.isParseData)
	{
		Save_Data.isParseData = false;
		
		printf("Save_Data.UTCTime = ");
		printf(Save_Data.UTCTime);
		printf("\r\n");
		
		if(Save_Data.isUsefull)
		{
		
			Save_Data.isUsefull = false;
			//upstate_inf.intf_work_sta =1 ;
			get_lati = atof(Save_Data.latitude) *100000;
			get_longti= atof(Save_Data.longitude)*100000;
			 // DEBUG(LOG_DEBUG, "local latitude:%d,longitude:%d  \n",get_lati,get_longti);
             //       upstate_inf.reserve0 = get_lati;
			//upstate_inf.reserve1 = get_longti;
			
			 loc_coor =  WGS84tobaidu(get_longti/100000,get_lati/100000);
			 memcpy(loc_coor.devid,did,16);
			 Creatstatejson(loc_coor);
			//  data_parse();
		

		}
		
		return loc_coor;
	}
	 return loc_coor;
}
void parseGpsBuffer()
{
	char *subString;
	char *subStringNext;
	char i = 0;
	if (Save_Data.isGetData)
	{
		Save_Data.isGetData = false;
	//	printf("**************\r\n");
	//	printf(Save_Data.GPS_Buffer);

		
		for (i = 0 ; i <= 6 ; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
					printf("error");	//解析错误
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2]; 
					switch(i)
					{
						case 1:memcpy(Save_Data.UTCTime, subString, subStringNext - subString);break;	//获取UTC时间
						case 2:memcpy(usefullBuffer, subString, subStringNext - subString);break;	//获取UTC时间
						case 3:memcpy(Save_Data.latitude, subString, subStringNext - subString);break;	//获取纬度信息
						case 4:memcpy(Save_Data.N_S, subString, subStringNext - subString);break;	//获取N/S
						case 5:memcpy(Save_Data.longitude, subString, subStringNext - subString);break;	//获取经度信息
						case 6:memcpy(Save_Data.E_W, subString, subStringNext - subString);break;	//获取E/W

						default:break;
					}

					subString = subStringNext;
					Save_Data.isParseData = true;
					if(usefullBuffer[0] == 'A')
						Save_Data.isUsefull = true;
					else if(usefullBuffer[0] == 'V')
						Save_Data.isUsefull = false;

				}
				else
				{
						//解析错误
				}
			}


		}
	}
}
#if 0
int main(int argc, char* argv[])
{
  Location my_wgLoc,out_wgloc;
  my_wgLoc.lng = degree_minute2dec_degrees(11613.57409);
  my_wgLoc.lat = degree_minute2dec_degrees(4005.20173);

  
  out_wgloc= bd_encrypt(transformFromWGSToGCJ(my_wgLoc));
  printf("%.6f\t  %.6f\t",out_wgloc.lng,out_wgloc.lat); 
}
#endif
