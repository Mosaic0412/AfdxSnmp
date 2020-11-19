#include <stdio.h>
#include <stdlib.h>
#define HAVE_REMOTE
#include "windows.h"
#include <wpdpack/pcap/pcap.h>
#include <afdx/afdx_api.h>
#pragma comment(lib,"netapi32.lib")

CRITICAL_SECTION cs;//Define the critical section
int myThreadCount = 0;//the number of Thread
pcap_if_t *d;//save network device
struct pcap_pkthdr *pktHd;//the header of packet
const unsigned char* pktDt;//the data of packet
pcap_t *pt;//Store open network connection handle

unsigned int iptosendn, iptosendh, localip, nlNetMask, HostNum;
unsigned char localmac[6] = { 0, 0, 0, 0, 0, 0 }; //local MAC



#pragma pack(1)
struct ethernet_head//Ethernet physical frame header
{
	unsigned char dest_mac[6];
	unsigned char source_mac[6];
	unsigned short eh_type;
};
struct arp_head//the data struct of ARP
{
	unsigned short hardware_type; 
	unsigned short protocol_type;  
	unsigned char add_len;         // hardware address length
	unsigned char pro_len;         // protocol address length
	unsigned short option;         // ARP operation type: 1 : request, 2 : reply
	unsigned char sour_addr[6];    // source mac
	unsigned long sour_ip;         // source ip
	unsigned char dest_addr[6];    // destination mac
	unsigned long dest_ip;         // destination ip
	unsigned char padding[18];     // Stuffing byte,Fill the minimum length
};
struct arp_packet
{
	struct ethernet_head eth;
	struct arp_head arp;
}myPacket;

#pragma pack()//Boundary address alignment back to OS

void MakePacket(int sourceip, int destip, struct arp_packet *parp_packet);//Package function
DWORD WINAPI threadofrcv(LPVOID);//Receiving thread
DWORD WINAPI threadofsnt(LPVOID);//Sending thread
int ddmac[6];
int tttDMac(char *dmac)
{

	InitializeCriticalSection(&cs);//Initialize critical section resources
	pcap_if_t *alldevs;
	struct pcap_addr *pAdr;
	char errbuf[PCAP_ERRBUF_SIZE];
	char packet_filter[] = "ip and udp";
	short usability[255];   //used to save the availability of a certain device
	memset(usability, 1, 255);//Initial value assumptions are all available

	pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf);
	d = alldevs;

	if ((pt = pcap_open(d->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)) == NULL)// open adapter
	{
		printf("打开网络接口失败！\n");
		pcap_freealldevs(alldevs);
		return -1;
	}

	//Search all network segments of the IP address and send an ARP request to the machine
	for (pAdr = d->addresses; pAdr; pAdr = pAdr->next)
	{
		localip = ((struct sockaddr_in *)pAdr->addr)->sin_addr.s_addr;//get ip
		nlNetMask = ((struct sockaddr_in *)(pAdr->netmask))->sin_addr.S_un.S_addr;//get subnet mask
	}

	HostNum = ~ntohl(nlNetMask);//the number of hosts in the LAN, obtained according to the mask
	/*
	The first IP address to send ARP request (after obtaining the local MAC,
	it will send arp requests to all possible IP addresses)
	*/
	iptosendh = ntohl(localip&nlNetMask);

	CreateThread(NULL, 0, threadofsnt, NULL, 0, NULL);//Receiving thread
	CreateThread(NULL, 0, threadofrcv, NULL, 0, NULL);//Sending thread
	Sleep(1000);

	while (myThreadCount > 0)
	{
		Sleep(1);
	}

	pcap_freealldevs(alldevs);//release device list
	
	sprintf(dmac, "%02x-%02x-%02x-%02x-%02x-%02x", ddmac[0], ddmac[1], ddmac[2], ddmac[3], ddmac[4], ddmac[5]);
	return 0;
}


//******************************************************************************************************************
//*****************************************Receiving thread*********************************************************
//******************************************************************************************************************
DWORD WINAPI threadofrcv(char *dmac)
{
	int inip[4];
	printf("\nIP地址：");
	scanf("%d.%d.%d.%d", &inip[0], &inip[1], &inip[2], &inip[3]);
	/*
	In order to prevent two processes from changing the value of myThreadCount at the same time, 
	treat myThreadCount as a critical resource and enter the critical area
	*/
	EnterCriticalSection(&cs);
	myThreadCount++;
	LeaveCriticalSection(&cs);//leaving critical zone
	char errorBuffer[PCAP_ERRBUF_SIZE];

	pcap_t* session = pcap_open(d->name, 60, PCAP_OPENFLAG_PROMISCUOUS, 10, NULL, errorBuffer);//build a arp connect

	while (1)
	{
		int res = pcap_next_ex(session, &pktHd, &pktDt);
		if (res == 1) //receive success
		{
			struct arp_packet *recv = (struct arp_packet *)pktDt;
			if (htons(0x0806) == recv->eth.eh_type)//if it is not an ARP packet, discard it and receive it again
			{
				if (recv->arp.option == htons(0x0002) && recv->arp.dest_ip == localip)
				{
					if ((inip[0] == (int)(recv->arp.sour_ip & 255)) && (inip[1] == (int)(recv->arp.sour_ip >> 8 & 255)) && (inip[2] == (int)(recv->arp.sour_ip >> 16 & 255)) && (inip[3] == (int)(recv->arp.sour_ip >> 24 & 255)))
					{
						printf("\n*******************************find result*******************************\n");
						printf("\nIP: %d.%d.%d.%d --------> MAC:", recv->arp.sour_ip & 255, recv->arp.sour_ip >> 8 & 255, recv->arp.sour_ip >> 16 & 255, recv->arp.sour_ip >> 24 & 255);

						for (int i = 0; i < 6; i++)
						{
							printf("%02x ", recv->arp.sour_addr[i]);
							ddmac[i] = recv->arp.sour_addr[i];
						}
						printf("\n");
						myThreadCount = -1;
						break;
					}
				}
			}
		}

		if (myThreadCount < 2) break;//thread synchronization
	}
	EnterCriticalSection(&cs);
	myThreadCount--;
	LeaveCriticalSection(&cs);
	return 0;
}

//******************************************************************************************************************
//*****************************************Sending thread*************************************************************
//******************************************************************************************************************
DWORD WINAPI  threadofsnt()
{
	EnterCriticalSection(&cs);//Critical resource control only writes and reads the variable myThreadCount to maintain data consistency
	myThreadCount++;
	LeaveCriticalSection(&cs);

	while (myThreadCount < 2)//Thread synchronization
	{
		Sleep(50);
	}

	char errorBuffer[PCAP_ERRBUF_SIZE];
	pcap_t* session = pcap_open(d->name, 60, PCAP_OPENFLAG_PROMISCUOUS, 10, NULL, errorBuffer);//build connect to send arp data
	while (1)
	{
		iptosendh++;
		iptosendn = htonl(iptosendh);
		MakePacket(localip, iptosendn, &myPacket);//Package arp packet

		if (pcap_sendpacket(session, (unsigned char*)&myPacket, 60) != 0)//sned
			printf("发送数据时出错!\n");
		HostNum--;//The number of the remaining hosts that have not yet sent packets will be reduced by 1
		if (0 == HostNum)  break;
	}
		Sleep(5000);
	EnterCriticalSection(&cs);
	myThreadCount--;
	LeaveCriticalSection(&cs);
	return 0;
}

//******************************************************************************************************************
//**************************************Package ARP request packet function*****************************************
//******************************************************************************************************************
void MakePacket(int sourceip, int destip, struct arp_packet *parp_packet)
{
	memset(parp_packet->eth.dest_mac, 0xFF, sizeof(parp_packet->eth.dest_mac));//set the destination MAC of the header of the physical frame FF-FF-FF-FF
	memcpy(parp_packet->eth.source_mac, localmac, sizeof(localmac));
	parp_packet->eth.eh_type = htons(0x0806);//Ethernet network type, host sequence

	parp_packet->arp.hardware_type = htons(0x0001);//host sequence, hardware type
	parp_packet->arp.protocol_type = htons(0x0800);//host sequence, IP protocol
	parp_packet->arp.add_len = 0x06;
	parp_packet->arp.pro_len = 0x04;
	parp_packet->arp.option = ntohs(0x0001);//Operation type : 1
	memcpy(parp_packet->arp.sour_addr, localmac, sizeof(localmac));
	parp_packet->arp.sour_ip = sourceip;
	memset(parp_packet->arp.dest_addr, 0, 6);
	parp_packet->arp.dest_ip = destip;

	memset(parp_packet->arp.padding, 0, 18);
}