#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ClassLinker.h"


//설정파일을 불러오는 함수
int importSettings(_Out_ SETTINGS *sets)
{
	int retval;
	char buffer[256];

	//설정파일을 읽기모드로 연다.
	FILE *rfp;
	retval = fopen_s(&rfp, "Settings.ini", "r");
	if (retval != 0)
	{
		perror("설정파일 열기fopen_s()");
		return 1;
	}

	//설정파일을 읽어온다.
	while (!feof(rfp))
	{
		//한 줄씩 파일을 읽어온다.
		fgets(buffer, sizeof(buffer), rfp);

		//예외사항을 적어둔다.
		if (strncmp(buffer, "//", 2) == 0)  //주석문 읽기안함
			continue;
		else if (strcmp(buffer, "\n") == 0)  //Enter 읽기안함
			continue;
		else if (strncmp(buffer, " ", 1) == 0)  //맨 앞이 공백인 문장 읽기안함
			continue;

		else
		{
			//'\n'문자 제거
			int len = (int)strlen(buffer);
			if (buffer[len - 1] == '\n')
				buffer[len - 1] = '\0';

			//'='문자 위치를 저장한다.
			//만약, 값이 없으면 읽지않는다.
			char *pos = strchr(buffer, '=');
			if (strcmp(pos + 1, "\0") == 0)
				continue;

			//설정 항목들을 sets객체에 각각 저장한다.
			//설정 항목은 if문에 적어둔다.
			//-------------------------------------------------------------------------
			if (strncmp(buffer, "execute_mode", pos - buffer) == 0)
				strcpy_s(sets->execute_mode, sizeof(sets->execute_mode), pos + 1);
			else if (strncmp(buffer, "server_ip", pos - buffer) == 0)
				strcpy_s(sets->server_ip, sizeof(sets->server_ip), pos + 1);
			else if (strncmp(buffer, "server_mainPort", pos - buffer) == 0)
				sets->server_mainPort = atoi(pos + 1);
			else if (strncmp(buffer, "server_requestPort", pos - buffer) == 0)
				sets->server_requestPort = atoi(pos + 1);
			else if (strncmp(buffer, "multichat_ip", pos - buffer) == 0)
				strcpy_s(sets->multichat_ip, sizeof(sets->multichat_ip), pos + 1);
			else if (strncmp(buffer, "multichat_port", pos - buffer) == 0)
				sets->multichat_port = atoi(pos + 1);
			else if (strncmp(buffer, "server_uid", pos - buffer) == 0)
				sets->server_uid = atoi(pos + 1);
			else if (strncmp(buffer, "server_nickName", pos - buffer) == 0)
				strcpy_s(sets->server_nickName, sizeof(sets->server_nickName), pos + 1);
			else if (strncmp(buffer, "client_uid", pos - buffer) == 0)
				sets->client_uid = atoi(pos + 1);
			else if (strncmp(buffer, "client_nickName", pos - buffer) == 0)
				strcpy_s(sets->client_nickName, sizeof(sets->client_nickName), pos + 1);
			//-------------------------------------------------------------------------
		}
	}

	fclose(rfp);
	return 0;
}