#pragma once
/*��鳤���Ƿ񳬹�15*/
int checkIPLength(char * ipaddr);
/*����Ƿ��зǷ��ַ�*/
int checkCharInIpAddress(char * ipaddr, int &dotnum);
/*����Ƿ��������ķָ���*/
int checkContinuesSeperator(char * ipaddr);
/*����Ƿ��Էָ�����β*/
int checkLastSeperator(char * ipaddr, int len);
/*��ʼ�������������IP��ַ��4������*/
void initialTemp(char  temp[4][15]);
/*��IP��ַ��4���ִ���temp������*/
void partionIntoTemp(char * ipaddr, char  temp[4][15]);
/*���ÿ�����Ƿ񳤶ȳ���3*/
int checkLenOfEachPart(int &len, char  temp[4][15], int  ip[4]);
/*���ÿ���ֵ���ֵ�Ƿ񳬹�255*/
int checkValueBoundaryOfEachPart(int  ip[4]);
/*IP��ַ����*/
void classifyIpAddress(int  ip[4], char * inputIp);
