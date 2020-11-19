#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <httpext.h> //supports HTTP requests
#include <windef.h>     //All basic data types of Windows
#include <Nb30.h>    //All functions of netbios are declared

#include <afdx/afdx_api.h>
#include <afdx/afdx_api.h>

#pragma comment(lib,"netapi32.lib") //connect Netapi32.lib, used to get mac


/*
 * Winsock programming on the Win32 platform has to go through the following basic steps:
 * Define variable -> get Winsock version -> load Winsock library -> initialize -> create socket
 * -> set socket option -> close socket -> unload Winsock library, release all resources.
 */

int getIP(char *ip2)//used to get local ip
{
	WSADATA wsaData; char name[155];//used to save hostname
	char *ip;
	PHOSTENT hostinfo; //the correct value of the Winsocl version, used to load the Winscok library

	//load Winsock library
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) == 0)  //load 2.0socket library
	{
		if (gethostname(name, sizeof(name)) == 0)
		{
			if ((hostinfo = gethostbyname(name)) != NULL)
			{
				//Call the inet_ntoa() function to convert the h_addr_list to IP address (such as 202.197.11.12.)  
				ip = inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list); //inet_addr()�����ѵ�ַ��ת��ΪIP��ַ
				//printf(" IP��ַ: %s\n", ip);//���IP��ַ
			}
		}
		WSACleanup();   //ж��Winsock�⣬���ͷ�������Դ
	}
	sprintf(ip2, "%s", ip);
	return 0;
}



//ͨ��WindowsNT/Win2000�����õ�NetApi32.DLL�Ĺ�����ʵ�ֵġ�����ͨ������NCBENUM����,��ȡ������
//��Ŀ��ÿ���������ڲ����,Ȼ���ÿ��������ŷ���NCBASTAT�����ȡ��MAC��ַ��
int getMAC(char *mac)//��NetAPI����ȡ����MAC��ַ
{
	NCB ncb; //����һ��NCB(������ƿ�)���͵Ľṹ�����ncb
	typedef struct _ASTAT_  //�Զ���һ���ṹ��_ASTAT_
	{
		ADAPTER_STATUS adapt;
		NAME_BUFFER NameBuff[30];

	}ASTAT, *PASTAT;

	ASTAT Adapter;

	typedef struct _LANA_ENUM  //�Զ���һ���ṹ��_LANA_ENUM
	{
		UCHAR length;
		UCHAR lana[MAX_LANA];//�������MAC��ַ

	}LANA_ENUM;

	LANA_ENUM lana_enum;  // ȡ��������Ϣ�б�
	UCHAR uRetCode;
	memset(&ncb, 0, sizeof(ncb));  //���ѿ����ڴ�ռ�ncb ��ֵ����Ϊֵ 0
	memset(&lana_enum, 0, sizeof(lana_enum));   //���һ���ṹ���͵ı���lana_enum����ֵΪ0

	//�Խṹ�����ncb��ֵ
	ncb.ncb_command = NCBENUM; //ͳ��ϵͳ������������
	ncb.ncb_buffer = (unsigned char*)&lana_enum;  //ncb_buffer��Աָ����LANA_ENUM�ṹ���Ļ�����
	ncb.ncb_length = sizeof(LANA_ENUM);    //����������NCBENUM����Ի�ȡ��ǰ������������Ϣ�����ж��ٸ�������ÿ�������ı�ţ�MAC��ַ��
	uRetCode = Netbios(&ncb);   //����netbois(ncb)��ȡ�������к�

	if (uRetCode != NRC_GOODRET)
		return uRetCode;   //��ÿһ�������������������Ϊ�����ţ���ȡ��MAC��ַ

	for (int lana = 0; lana<lana_enum.length; lana++)
	{
		ncb.ncb_command = NCBRESET; //����������NCBRESET������г�ʼ��
		ncb.ncb_lana_num = lana_enum.lana[lana]; uRetCode = Netbios(&ncb);
	}

	if (uRetCode != NRC_GOODRET)
		return uRetCode;   // ׼��ȡ�ýӿڿ���״̬��ȡ��MAC��ַ

	memset(&ncb, 0, sizeof(ncb));

	ncb.ncb_command = NCBASTAT; //����������NCBSTAT�����ȡ������Ϣ
	ncb.ncb_lana_num = lana_enum.lana[0]; //ָ�������ţ��������ָ����һ��������ͨ��Ϊ��������
	strcpy((char*)ncb.ncb_callname, "*");//Զ��ϵͳ����ֵΪ*
	ncb.ncb_buffer = (unsigned char*)&Adapter; //ָ�����ص���Ϣ��ŵı���
	ncb.ncb_length = sizeof(Adapter); //���ŷ���NCBASTAT�����Ի�ȡ��������Ϣ
	uRetCode = Netbios(&ncb);   // ȡ����������Ϣ����������������������Ļ������ر�׼��ð�ŷָ���ʽ��

	if (uRetCode != NRC_GOODRET)
		return uRetCode; //������MAC��ַ��ʽת��Ϊ���õ�16������ʽ,������ַ���mac��

	sprintf(mac, "%02X-%02X-%02X-%02X-%02X-%02X", Adapter.adapt.adapter_address[0], Adapter.adapt.adapter_address[1], Adapter.adapt.adapter_address[2], Adapter.adapt.adapter_address[3], Adapter.adapt.adapter_address[4], Adapter.adapt.adapter_address[5]);

	return 0;
}

int getDMac(char *dmac) //����������������
{
	tttDMac(dmac);
	return 0;
}