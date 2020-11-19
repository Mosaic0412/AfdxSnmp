/******************************************************************************
*  Includes
******************************************************************************/
#include <afdx/afdx_api.h>
#include <stdio.h>
#include <regex.h>

/************ Start of Code ************/ 
int l_Mac_to_VL(){
	int status = 0, i = 0;
	int flag = REG_EXTENDED;
	regmatch_t pmatch[1];
	const size_t nmatch = 1;
	regex_t reg;
	const char *pattern = "([0]{1}[3]{1}-)(([0]{2}-){3})([0-9a-fA-F]{2}-)([0-9a-fA-F]{2})";
	char *buf = "03-00-00-00-22-33";//success

	regcomp(&reg, pattern, flag);
	status = regexec(&reg, buf, nmatch, pmatch, 0);
	if (status == REG_NOMATCH){
		printf("no match\n");
	}
	else if (status == 0){
		printf("match success\n");
		for (i = 12; i < pmatch[0].rm_eo; i++){
			putchar(buf[i]);
		}
		putchar('\n');
	}
	regfree(&reg);
	system("pause");
	return 0;
}
void l_VL_to_Mac(){

}