#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

int main(int argc, char **argv)
{

	//-----------------------------------------
	// Declare and initialize variables
	WSADATA wsaData;
	int iResult;

	DWORD dwError;
	int i = 0;

	struct hostent *remoteHost;
	char host_name[128];
	struct in_addr addr;

	char **pAlias;

	
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	gethostname(host_name, 100);

	printf("Calling gethostbyname with %s\n", host_name);
	remoteHost = gethostbyname(host_name);

	if (remoteHost == NULL) {
		dwError = WSAGetLastError();
		if (dwError != 0) {
			if (dwError == WSAHOST_NOT_FOUND) {
				printf("Host not found\n");
				return 1;
			}
			else if (dwError == WSANO_DATA) {
				printf("No data record found\n");
				return 1;
			}
			else {
				printf("Function failed with error: %ld\n", dwError);
				return 1;
			}
		}
	}
	else {
		printf("Function returned:\n");
		printf("\tOfficial name: %s\n", remoteHost->h_name);
		for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++) {
			printf("\tAlternate name #%d: %s\n", ++i, *pAlias);
		}
		printf("\tAddress type: ");
		switch (remoteHost->h_addrtype) {
		case AF_INET:
			printf("AF_INET\n");
			break;
		case AF_NETBIOS:
			printf("AF_NETBIOS\n");
			break;
		default:
			printf(" %d\n", remoteHost->h_addrtype);
			break;
		}
		printf("\tAddress length: %d\n", remoteHost->h_length);

		i = 0;
		if (remoteHost->h_addrtype == AF_INET)
		{
			while (remoteHost->h_addr_list[i] != 0) {
				addr.s_addr = *(u_long *)remoteHost->h_addr_list[i++];
				printf("\tIP Address #%d: %s\n", i, inet_ntoa(addr));
			}
		}
		else if (remoteHost->h_addrtype == AF_NETBIOS)
		{
			printf("NETBIOS address was returned\n");
		}
	}

	return 0;
}