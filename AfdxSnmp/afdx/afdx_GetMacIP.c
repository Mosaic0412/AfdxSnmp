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
				ip = inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list); //inet_addr()函数把地址串转换为IP地址
				//printf(" IP地址: %s\n", ip);//输出IP地址
			}
		}
		WSACleanup();   //卸载Winsock库，并释放所有资源
	}
	sprintf(ip2, "%s", ip);
	return 0;
}



//通过WindowsNT/Win2000中内置的NetApi32.DLL的功能来实现的。首先通过发送NCBENUM命令,获取网卡的
//数目和每张网卡的内部编号,然后对每个网卡标号发送NCBASTAT命令获取其MAC地址。
int getMAC(char *mac)//用NetAPI来获取网卡MAC地址
{
	NCB ncb; //定义一个NCB(网络控制块)类型的结构体变量ncb
	typedef struct _ASTAT_  //自定义一个结构体_ASTAT_
	{
		ADAPTER_STATUS adapt;
		NAME_BUFFER NameBuff[30];

	}ASTAT, *PASTAT;

	ASTAT Adapter;

	typedef struct _LANA_ENUM  //自定义一个结构体_LANA_ENUM
	{
		UCHAR length;
		UCHAR lana[MAX_LANA];//存放网卡MAC地址

	}LANA_ENUM;

	LANA_ENUM lana_enum;  // 取得网卡信息列表
	UCHAR uRetCode;
	memset(&ncb, 0, sizeof(ncb));  //将已开辟内存空间ncb 的值均设为值 0
	memset(&lana_enum, 0, sizeof(lana_enum));   //清空一个结构类型的变量lana_enum，赋值为0

	//对结构体变量ncb赋值
	ncb.ncb_command = NCBENUM; //统计系统中网卡的数量
	ncb.ncb_buffer = (unsigned char*)&lana_enum;  //ncb_buffer成员指向由LANA_ENUM结构填充的缓冲区
	ncb.ncb_length = sizeof(LANA_ENUM);    //向网卡发送NCBENUM命令，以获取当前机器的网卡信息，如有多少个网卡，每个网卡的编号（MAC地址）
	uRetCode = Netbios(&ncb);   //调用netbois(ncb)获取网卡序列号

	if (uRetCode != NRC_GOODRET)
		return uRetCode;   //对每一个网卡，以其网卡编号为输入编号，获取其MAC地址

	for (int lana = 0; lana<lana_enum.length; lana++)
	{
		ncb.ncb_command = NCBRESET; //对网卡发送NCBRESET命令，进行初始化
		ncb.ncb_lana_num = lana_enum.lana[lana]; uRetCode = Netbios(&ncb);
	}

	if (uRetCode != NRC_GOODRET)
		return uRetCode;   // 准备取得接口卡的状态块取得MAC地址

	memset(&ncb, 0, sizeof(ncb));

	ncb.ncb_command = NCBASTAT; //对网卡发送NCBSTAT命令，获取网卡信息
	ncb.ncb_lana_num = lana_enum.lana[0]; //指定网卡号，这里仅仅指定第一块网卡，通常为有线网卡
	strcpy((char*)ncb.ncb_callname, "*");//远程系统名赋值为*
	ncb.ncb_buffer = (unsigned char*)&Adapter; //指定返回的信息存放的变量
	ncb.ncb_length = sizeof(Adapter); //接着发送NCBASTAT命令以获取网卡的信息
	uRetCode = Netbios(&ncb);   // 取得网卡的信息，并且如果网卡正常工作的话，返回标准的冒号分隔格式。

	if (uRetCode != NRC_GOODRET)
		return uRetCode; //把网卡MAC地址格式转化为常用的16进制形式,输出到字符串mac中

	sprintf(mac, "%02X-%02X-%02X-%02X-%02X-%02X", Adapter.adapt.adapter_address[0], Adapter.adapt.adapter_address[1], Adapter.adapt.adapter_address[2], Adapter.adapt.adapter_address[3], Adapter.adapt.adapter_address[4], Adapter.adapt.adapter_address[5]);

	return 0;
}

int getDMac(char *dmac) //主函数，程序的入口
{
	tttDMac(dmac);
	return 0;
}