//
//  main.c
//  AnalyzingOid
//
//  Created by ³ÂÇì»ª on 2020/11/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <afdx/afdx_api.h>
// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

char sss[] = "0300000083a40200002ad12008004500009f000040000111f5d00a074d4d0a2a2200ee5a00a1008b000030818002010004086166647852656164a081700201000201000201003081643012060e2b0601040184670b040c0101030105003012060e2b0601040184670b040c0101020105003012060e2b0601040184670b040c0101010105003012060e2b0601040184670b040c0101110105003012060e2b0601040184670b040c01011001050024";
static int hex_table[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6,
7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10,
11, 12, 13, 14, 15, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11,
12, 13, 14, 15 };

int hex_to_decimal(char *hex_str){
	char ch;
	int iret = 0;
	while (ch = *hex_str++){
		if (ch != '-'&& ch != ':')
			iret = (iret << 4) | hex_table[ch];
	}
	return iret;
}

char* print(char *s, int start, int len){
	char snm[1024];
	int i, j;
	start = (start - 1) * 2;
	len = len * 2;
	for (i = start, j = 0; i < start + len; i++, j++){
		snm[j] = *(s + i);
	}
	snm[j] = '\0';
	return snm;
}

char * oidnameToOid(char *oid){
	//char *oidData;
	printf("/**********/\n");
	//printf("oid : %s\n",oid);
	int oidLen = hex_to_decimal(print(oid, 2, 1));
	printf("oid type : %s , length : %d\n", print(oid, 1, 1), oidLen);
	printf("oid number : %s\n", print(oid, 3, oidLen));
	printf("oid end : %s\n", print(oid, 3 + oidLen, 2));

	printf("Analyzing Oid : ");
	int x = 0, y = 0;
	x = hex_to_decimal(print(oid, 3, 1));
	y = x - (x / 40) * 40;
	x = x / 40;
	printf("%d.%d.", x, y);
	for (int i = 4; i < oidLen + 3; i++){
		int num = hex_to_decimal(print(oid, i, 1));
		if (num < 127){
			if (i + 1 < oidLen + 3)
				printf("%d.", num);
			else
				printf("%d", num);
		}
		else{
			int num2 = hex_to_decimal(print(oid, i + 1, 1));
			int j = 7, k = 0, delNum = 0;;
			int bigthan127[16] = { 0 };
			while (num){
				bigthan127[j--] = num % 2;
				num = num / 2;
			}
			j = 15;
			while (num2){
				bigthan127[j--] = num2 % 2;
				num2 = num2 / 2;
			}
			i++;
			for (j = 0; j < 15; ++j){
				if (j % 4 == 0 && (j / 4) % 2 == 0){
					delNum++;
				}
				else{
					bigthan127[k++] = bigthan127[j];
				}
			}
			while (delNum){
				for (j = 15 - delNum; j >= 0; --j)
					bigthan127[j + 1] = bigthan127[j];
				bigthan127[j + 1] = 0;
				--delNum;
			}
			char totalNum[5];
			num2 = 0;
			for (j = 0; j < 4; j++){
				num = 0;
				for (k = j * 4; k < (j + 1) * 4; k++){
					num = num + bigthan127[k] * pow(2.0, 4 * (j + 1) - k - 1);
				}
				num2 = num2 * 10 + num;
				totalNum[j] = num + '0';
			}
			totalNum[j] = '\0';
			printf("%d.", hex_to_decimal(totalNum));
		}
	}

	//printf("Analyzing Oid : %s\n",oidData);

	printf("\n/**********/\n");
	return oid;
}

void analyzingSnmp(){
	char *snmp = sss, ch;
	int i;
	printf("Analyzing Oid");

	printf("\nD Mac : ");
	for (int i = 0; i < 6; i++){
		printf("%s", print(snmp, i+1, 1));
		if (i < 5) printf(":");
	}

	printf("\nS Mac : ");
	for (int i = 0; i <6; i++){
		printf("%s", print(snmp, i+7, 1));
		if (i < 5) printf(":");
	}

	printf("\nsssS IP : ");
	for (int i = 0; i < 4; i++){
		printf("%d", hex_to_decimal(print(snmp, 27 + i, 1)));
		if (i < 3) printf(".");
	}

	printf("\nD IP : ");
	for (int i = 0; i < 4; i++){
		printf("%d", hex_to_decimal(print(snmp, 31 + i, 1)));
		if (i < 3) printf(".");
	}

	if (snmp[84] == '3' && snmp[85] == '0'){
		printf("\nThis is snmp packet\n");
		printf("length : %s\n", print(snmp, 45, 1));
		printf("snmpv1 version : %s (00 is v1 version)\n", print(snmp, 46+2, 1));

		int communityStart = 51;
		int communityLen = hex_to_decimal(print(snmp, 50, 1));
		//char *community = print(snmp, communityStart, communityLen);
		printf("community : ");
		for (int i = 0; i < communityLen;i++)
			printf("%c", hex_to_decimal(print(snmp, communityStart+i,1)));
		//printf("community : %s\n", community);

		int pduStart = 49 + 2 + communityLen;
		printf("\npdu type : %s\n", print(snmp, pduStart, 2));
		printf("pdu include length : %d\n", hex_to_decimal(print(snmp, pduStart + 2, 1)));

		int request_idStart = pduStart + 3;
		printf("request_id : %s\n", print(snmp, request_idStart, 3));
		int error_statusStart = request_idStart + 3;
		printf("error_status : %s\n", print(snmp, error_statusStart, 3));
		int error_indexStart = error_statusStart + 3;
		printf("error_index : %s\n", print(snmp, error_indexStart, 3));

		int dataStart = error_indexStart + 3;
		int dataLen = hex_to_decimal(print(snmp, dataStart + 2, 1));
		printf("data type : %s\n", print(snmp, dataStart, 2));
		printf("data length : %d\n", dataLen);

		int oidStart = dataStart + 3;

		//printf("%s\n",print(snmp,oidStart,20)); //first oid

		oidStart = (oidStart - 1) * 2;
		int no = 0;

		char oidname[100];
		for (i = oidStart; i < oidStart + dataLen * 2; i++){
			//printf("\n%s ",print(snmp,i/2+1,2));
			if (i % 2 == 0){
				if (snmp[i] == '3' && snmp[i + 1] == '0'){

					int oidStart = i / 2 + 1;
					int oidLen = hex_to_decimal(print(snmp, oidStart + 1, 1));
					printf("\noid %d, length : %d\n", no++, oidLen);

					oidname[0] = '\0';
					strcpy(oidname, print(snmp, oidStart + 2, oidLen));

					oidnameToOid(oidname);

					i = i + oidLen * 2 + 3;
					//break;
				}
			}
		}

		printf("\n");
	}
	else{
		printf("not snmp\n");
	}

}

/*
int main(int argc, char * argv[])
{
	analytical_snmp();
	system("pause");
	return 0;
}
*/