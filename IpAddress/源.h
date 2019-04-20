#pragma once
/*检查长度是否超过15*/
int checkIPLength(char * ipaddr);
/*检查是否有非法字符*/
int checkCharInIpAddress(char * ipaddr, int &dotnum);
/*检查是否有连续的分隔符*/
int checkContinuesSeperator(char * ipaddr);
/*检查是否以分隔符结尾*/
int checkLastSeperator(char * ipaddr, int len);
/*初始化缓冲区，存放IP地址的4个部分*/
void initialTemp(char  temp[4][15]);
/*将IP地址的4部分存入temp缓冲区*/
void partionIntoTemp(char * ipaddr, char  temp[4][15]);
/*检查每部分是否长度超过3*/
int checkLenOfEachPart(int &len, char  temp[4][15], int  ip[4]);
/*检查每部分的数值是否超过255*/
int checkValueBoundaryOfEachPart(int  ip[4]);
/*IP地址归类*/
void classifyIpAddress(int  ip[4], char * inputIp);
