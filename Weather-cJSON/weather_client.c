/*******************************************************************************************************
** ���ƣ�����HTTP�ͻ���
** ���ߣ������
** ����΢�Ź��ںţ�Ƕ��ʽ���ӻ�
** ˵����һ��Ҫ�޸�KEY��ֵΪ�Լ��ģ������ܻ�ȡ�õ��������ݣ�
********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "cJSON.h"
#include "utf8togbk.h"

/* ���Կ��� */
#define  DEBUG   0		// 1�����԰汾�������printf������䣩   0�������汾

/* ��֪������www.seniverse.com��IP���˿� */
#define  WEATHER_IP_ADDR   "116.62.81.138"
#define  WEATHER_PORT	   80

/* ��Կ��ע�⣡�����Ҫ����һ�ݴ��룬���һ��Ҫ��Ϊ�Լ��ģ���Ϊ������Ѿ�����Ĵ��ˣ���ֹ�������ҹ���һ��KEY */
#define  KEY    "SOCW-9F59deKyPETA"		// ��������֪����ע���ÿ���û��Լ���һ��key

/* GET����� */
#define  GET_REQUEST_PACKAGE     \
         "GET https://api.seniverse.com/v3/weather/%s.json?key=%s&location=%s&language=zh-Hans&unit=c\r\n\r\n"
	
/* JSON���ݰ� */	
#define  NOW_JSON     "now"
#define  DAILY_JSON   "daily"
//....���ø����������������ݰ��ɲ�����֪����

/* �������ݽṹ�� */
typedef struct
{
	/* ʵ���������� */
	char id[32];				//id
	char name[32];				//����
	char country[32];			//����
	char path[32];				//��������·��
	char timezone[32];			//ʱ��
	char timezone_offset[32];   //ʱ��
	char text[32];				//����Ԥ������
	char code[32];				//����Ԥ������
	char temperature[32];   	//����
	char last_update[32];		//���һ�θ��µ�ʱ��
	
	
	/* ���졢���졢������������ */
	char date[3][32];			//����
	char text_day[3][64];	    //����������������
	char code_day[3][32];		//���������������
	char code_night[3][64]; 	//��������������
	char high[3][32];			//�����
	char low[3][32];			//�����
	char wind_direction[3][64]; //����
	char wind_speed[3][32];  	//���٣���λkm/h����unit=cʱ��
	char wind_scale[3][32];  	//�����ȼ�
}Weather;

/* cmd�������� */
struct cmd_windows_config
{
	int width;
	int high;
	int color;
};

/* cmd����Ĭ������ */
struct cmd_windows_config cmd_default_config =
{
	60,
	40,
	0xf0
};

// ��������
static void GetWeather(char *weather_json, char *location, Weather *result);
static int cJSON_NowWeatherParse(char *JSON, Weather *result);
static int cJSON_DailyWeatherParse(char *JSON, Weather *result);
static void DisplayWeather(Weather *weather_data);
static void cmd_window_set(struct cmd_windows_config *config);
static void printf_topic(void);
	
/*******************************************************************************************************
** ����: main��������
**------------------------------------------------------------------------------------------------------
** ����: void
** ����: void
********************************************************************************************************/
int main(void)
{
	Weather weather_data = {0};
	char location[32] = {0};
	
	cmd_window_set(&cmd_default_config); // ����cmd����
	printf_topic();
	
	while ((1 == scanf("%s", location))) // �������ƴ��
	{
		system("cls");	// ����
		memset(&weather_data, 0, sizeof(weather_data));  // weather_data���� 
		GetWeather(NOW_JSON, location, &weather_data);   // GET ������ʵ����������
		GetWeather(DAILY_JSON, location, &weather_data); // GET ��������������������
		DisplayWeather(&weather_data);					 // ��ʾ�������
		printf("\n������Ҫ��ѯ�����ĳ������Ƶ�ƴ�����磺beijing����");
	}
	
	return 0;
}

/*******************************************************************************************************
** ����: GetWeather����ȡ�������ݲ�����
**------------------------------------------------------------------------------------------------------
** ����: weather_json����Ҫ������json��   location������   result�����ݽ����Ľ��
** ����: void
********************************************************************************************************/
static void GetWeather(char *weather_json, char *location, Weather *result)
{
	SOCKET ClientSock;
	WSADATA wd;
	char GetRequestBuf[256] = {0};
	char WeatherRecvBuf[2*1024] = {0};
	char GbkRecvBuf[2*1024] = {0};
	int  gbk_recv_len = 0;
	int  connect_status = 0;
	
	/* ��ʼ������sock��Ҫ��DLL */
	WSAStartup(MAKEWORD(2,2),&wd);  
	
	/* ����Ҫ���ʵķ���������Ϣ */
    SOCKADDR_IN  ServerSockAddr;
    memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));  		  // ÿ���ֽڶ���0���
    ServerSockAddr.sin_family = PF_INET;						  // IPv4
    ServerSockAddr.sin_addr.s_addr = inet_addr(WEATHER_IP_ADDR);  // ��֪����������IP
    ServerSockAddr.sin_port = htons(WEATHER_PORT);   			  // �˿�
	
	/* �����ͻ���socket */
	if (-1 == (ClientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)))
	{
		printf("socket error!\n");
		exit(1);
	}
	
	/* ���ӷ���� */
	if (-1 == (connect_status = connect(ClientSock, (SOCKADDR*)&ServerSockAddr, sizeof(SOCKADDR))))
	{
		printf("connect error!\n");
		exit(1);
	}
	
	/* ���GET����� */
	sprintf(GetRequestBuf, GET_REQUEST_PACKAGE, weather_json, KEY, location);
	
	/* �������ݵ������ */
	send(ClientSock, GetRequestBuf, strlen(GetRequestBuf), 0);
		
	/* ���ܷ���˵ķ������� */
	recv(ClientSock, WeatherRecvBuf, 2*1024, 0);
	
	/* utf-8תΪgbk */
	SwitchToGbk((const unsigned char*)WeatherRecvBuf, strlen((const char*)WeatherRecvBuf), (unsigned char*)GbkRecvBuf, &gbk_recv_len);	
#if DEBUG
	printf("����˷��ص�����Ϊ��%s\n", GbkRecvBuf);
#endif
	
	/* �����������ݲ����浽�ṹ�����weather_data�� */
	if (0 == strcmp(weather_json, NOW_JSON))		// ����ʵ��
	{
		cJSON_NowWeatherParse(GbkRecvBuf, result);	
	}
	else if(0 == strcmp(weather_json, DAILY_JSON)) // δ����������
	{
		cJSON_DailyWeatherParse(GbkRecvBuf, result);	
	}
	
	/* ��ջ����� */
	memset(GetRequestBuf, 0, 256);   
	memset(WeatherRecvBuf, 0, 2*1024);   
	memset(GbkRecvBuf, 0, 2*1024); 
	
	/* �ر��׽��� */
	closesocket(ClientSock);  
	
	/* ��ֹʹ�� DLL */
	WSACleanup();  
}

/*******************************************************************************************************
** ����: cJSON_NowWeatherParse����������ʵ������
**------------------------------------------------------------------------------------------------------
** ����: JSON���������ݰ�   result�����ݽ����Ľ��
** ����: void
********************************************************************************************************/
static int cJSON_NowWeatherParse(char *JSON, Weather *result)
{
	cJSON *json,*arrayItem,*object,*subobject,*item;
	
	json = cJSON_Parse(JSON); //����JSON���ݰ�
	if(json == NULL)		  //���JSON���ݰ��Ƿ�����﷨�ϵĴ��󣬷���NULL��ʾ���ݰ���Ч
	{
		printf("Error before: [%s]\n",cJSON_GetErrorPtr()); //��ӡ���ݰ��﷨�����λ��
		return 1;
	}
	else
	{
		if((arrayItem = cJSON_GetObjectItem(json,"results")) != NULL); //ƥ���ַ���"results",��ȡ��������
		{
			int size = cJSON_GetArraySize(arrayItem);     //��ȡ�����ж������
#if DEBUG
			printf("cJSON_GetArraySize: size=%d\n",size); 
#endif
			if((object = cJSON_GetArrayItem(arrayItem,0)) != NULL)//��ȡ����������
			{
				/* ƥ���Ӷ���1�����е������ */
				if((subobject = cJSON_GetObjectItem(object,"location")) != NULL)
				{
					// ƥ��id
					if((item = cJSON_GetObjectItem(subobject,"id")) != NULL)   
					{
						memcpy(result->id, item->valuestring,strlen(item->valuestring)); 		// �������ݹ��ⲿ����
					}
					// ƥ�������
					if((item = cJSON_GetObjectItem(subobject,"name")) != NULL) 
					{
						memcpy(result->name, item->valuestring,strlen(item->valuestring)); 		// �������ݹ��ⲿ����
					}
					// ƥ��������ڵĹ���
					if((item = cJSON_GetObjectItem(subobject,"country")) != NULL)
					{
						memcpy(result->country, item->valuestring,strlen(item->valuestring)); 	// �������ݹ��ⲿ����
					}
					// ƥ����������·��
					if((item = cJSON_GetObjectItem(subobject,"path")) != NULL)  
					{
						memcpy(result->path, item->valuestring,strlen(item->valuestring)); 		// �������ݹ��ⲿ����	
					}
					// ƥ��ʱ��
					if((item = cJSON_GetObjectItem(subobject,"timezone")) != NULL)
					{
						memcpy(result->timezone, item->valuestring,strlen(item->valuestring)); 	// �������ݹ��ⲿ����	
					}
					// ƥ��ʱ��
					if((item = cJSON_GetObjectItem(subobject,"timezone_offset")) != NULL)
					{
						memcpy(result->timezone_offset, item->valuestring,strlen(item->valuestring)); 	// �������ݹ��ⲿ����
					}
				}
				/* ƥ���Ӷ���2�������������� */
				if((subobject = cJSON_GetObjectItem(object,"now")) != NULL)
				{
					// ƥ��������������
					if((item = cJSON_GetObjectItem(subobject,"text")) != NULL)
					{
						memcpy(result->text, item->valuestring,strlen(item->valuestring));  // �������ݹ��ⲿ����
					}
					// ƥ�������������
					if((item = cJSON_GetObjectItem(subobject,"code")) != NULL)
					{
						memcpy(result->code, item->valuestring,strlen(item->valuestring));  // �������ݹ��ⲿ����
					}
					// ƥ������
					if((item = cJSON_GetObjectItem(subobject,"temperature")) != NULL) 
					{
						memcpy(result->temperature, item->valuestring,strlen(item->valuestring));   // �������ݹ��ⲿ����
					}	
				}
				/* ƥ���Ӷ���3�����ݸ���ʱ�䣨�ó��еı���ʱ�䣩 */
				if((subobject = cJSON_GetObjectItem(object,"last_update")) != NULL)
				{
					memcpy(result->last_update, subobject->valuestring,strlen(subobject->valuestring));   // �������ݹ��ⲿ����
				}
			} 
		}
	}
	
	cJSON_Delete(json); //�ͷ�cJSON_Parse()����������ڴ�ռ�
	
	return 0;
}

/*******************************************************************************************************
** ����: cJSON_DailyWeatherParse��������������������
**------------------------------------------------------------------------------------------------------
** ����: JSON���������ݰ�   result�����ݽ����Ľ��
** ����: void
********************************************************************************************************/
static int cJSON_DailyWeatherParse(char *JSON, Weather *result)
{
	cJSON *json,*arrayItem,*object,*subobject,*item,*sub_child_object,*child_Item;
	
	json = cJSON_Parse(JSON); //����JSON���ݰ�
	if(json == NULL)		  //���JSON���ݰ��Ƿ�����﷨�ϵĴ��󣬷���NULL��ʾ���ݰ���Ч
	{
		printf("Error before: [%s]\n",cJSON_GetErrorPtr()); //��ӡ���ݰ��﷨�����λ��
		return 1;
	}
	else
	{
		if((arrayItem = cJSON_GetObjectItem(json,"results")) != NULL); //ƥ���ַ���"results",��ȡ��������
		{
			int size = cJSON_GetArraySize(arrayItem);     //��ȡ�����ж������
#if DEBUG
			printf("Get Array Size: size=%d\n",size); 
#endif
			if((object = cJSON_GetArrayItem(arrayItem,0)) != NULL)//��ȡ����������
			{
				/* ƥ���Ӷ���1------�ṹ��location */
				if((subobject = cJSON_GetObjectItem(object,"location")) != NULL)
				{
					if((item = cJSON_GetObjectItem(subobject,"name")) != NULL) //ƥ���Ӷ���1��Ա"name"
					{
						memcpy(result->name, item->valuestring,strlen(item->valuestring)); 		// �������ݹ��ⲿ����
					}
				}
				/* ƥ���Ӷ���2------����daily */
				if((subobject = cJSON_GetObjectItem(object,"daily")) != NULL)
				{
					int sub_array_size = cJSON_GetArraySize(subobject);
#if DEBUG
					printf("Get Sub Array Size: sub_array_size=%d\n",sub_array_size);
#endif
					for(int i = 0; i < sub_array_size; i++)
					{
						if((sub_child_object = cJSON_GetArrayItem(subobject,i))!=NULL)
						{
							// ƥ������
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"date")) != NULL)
							{
								memcpy(result->date[i], child_Item->valuestring,strlen(child_Item->valuestring)); 		// ��������
							}
							// ƥ�����������������
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"text_day")) != NULL)
							{
								memcpy(result->text_day[i], child_Item->valuestring,strlen(child_Item->valuestring)); 	// ��������
							}
							// ƥ����������������
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"code_day")) != NULL)
							{
								memcpy(result->code_day[i], child_Item->valuestring,strlen(child_Item->valuestring)); 	// ��������
							}
							// ƥ��ҹ�������������
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"code_night")) != NULL)
							{
								memcpy(result->code_night[i], child_Item->valuestring,strlen(child_Item->valuestring)); // ��������
							}
							// ƥ������¶�
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"high")) != NULL)
							{
								memcpy(result->high[i], child_Item->valuestring,strlen(child_Item->valuestring)); 		//��������
							}
							// ƥ������¶�
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"low")) != NULL)
							{
								memcpy(result->low[i], child_Item->valuestring,strlen(child_Item->valuestring)); 		// ��������
							}
							// ƥ�����
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"wind_direction")) != NULL)
							{
								memcpy(result->wind_direction[i],child_Item->valuestring,strlen(child_Item->valuestring)); //��������
							}
							// ƥ����٣���λkm/h����unit=cʱ��
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"wind_speed")) != NULL)
							{
								memcpy(result->wind_speed[i], child_Item->valuestring,strlen(child_Item->valuestring)); // ��������
							}
							// ƥ������ȼ�
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"wind_scale")) != NULL)
							{
								memcpy(result->wind_scale[i], child_Item->valuestring,strlen(child_Item->valuestring)); // ��������
							}
						}
					}
				}
				/* ƥ���Ӷ���3------���һ�θ��µ�ʱ�� */
				if((subobject = cJSON_GetObjectItem(object,"last_update")) != NULL)
				{
					//printf("%s:%s\n",subobject->string,subobject->valuestring);
				}
			} 
		}
	}
	
	cJSON_Delete(json); //�ͷ�cJSON_Parse()����������ڴ�ռ�
	
	return 0;
}

/*******************************************************************************************************
** ����: DisplayWeather����ʾ��������
**------------------------------------------------------------------------------------------------------
** ����: weather_data����������
** ����: void
********************************************************************************************************/
static void DisplayWeather(Weather *weather_data)
{
	printf("===========%s��ʱ�������������===========\n",weather_data->name);
	printf("������%s\n",weather_data->text);		
	printf("���£�%s��\n",weather_data->temperature);	
	printf("ʱ����%s\n",weather_data->timezone);	
	printf("ʱ�%s\n",weather_data->timezone_offset);
	printf("��������ʱ�䣺%s\n",weather_data->last_update);
	printf("===========%s������������������===========\n",weather_data->name);
	printf("��%s��\n",weather_data->date[0]);
	printf("������%s\n",weather_data->text_day[0]);
	printf("����£�%s��\n",weather_data->high[0]);
	printf("����£�%s��\n",weather_data->low[0]);
	printf("����%s\n",weather_data->wind_direction[0]);
	printf("���٣�%skm/h\n",weather_data->wind_speed[0]);
	printf("�����ȼ���%s\n",weather_data->wind_scale[0]);
	printf("\n");
	printf("��%s��\n",weather_data->date[1]);
	printf("������%s\n",weather_data->text_day[1]);
	printf("����£�%s��\n",weather_data->high[1]);
	printf("����£�%s��\n",weather_data->low[1]);
	printf("����%s\n",weather_data->wind_direction[1]);
	printf("���٣�%skm/h\n",weather_data->wind_speed[1]);
	printf("�����ȼ���%s\n",weather_data->wind_scale[1]);
	printf("\n");
	printf("��%s��\n",weather_data->date[2]);
	printf("������%s\n",weather_data->text_day[2]);
	printf("����£�%s��\n",weather_data->high[2]);
	printf("����£�%s��\n",weather_data->low[2]);
	printf("����%s\n",weather_data->wind_direction[2]);
	printf("���٣�%skm/h\n",weather_data->wind_speed[2]);
	printf("�����ȼ���%s\n",weather_data->wind_scale[2]);
}

/*******************************************************************************************************
** ����: cmd_window_set������cmd����
**------------------------------------------------------------------------------------------------------
** ����: weather_data����������
** ����: void
********************************************************************************************************/
static void cmd_window_set(struct cmd_windows_config *config)
{
	char cmd[50];
	
	// ����cmd���ڱ���
	system("title �����"); 
	// ����cmd���ڿ���
	sprintf((char*)cmd, "mode con cols=%d lines=%d", 
			config->width, config->high);
	system(cmd);
	memset(cmd, 0, 50);
	// ����cmd���ڱ���ɫ
	sprintf((char*)cmd, "color %x", config->color);
    system(cmd);  	
	memset(cmd, 0, 50);				
}

/*******************************************************************************************************
** ����: printf_topic, ��ӡ����
**------------------------------------------------------------------------------------------------------
** ����: void
** ����: void
********************************************************************************************************/
static void printf_topic(void)
{
    system("date /T");	// �������
    system("time /T");	// ���ʱ��	
	
	printf("=================HTTP�����ͻ���==================\n");
	printf("������Ҫ��ѯ�����ĳ������Ƶ�ƴ�����磺beijing����");
}