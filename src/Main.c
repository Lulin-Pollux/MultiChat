#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "ClassLinker.h"

//우리 프로젝트에서 사용할 멀티캐스트 대표IP, PORT번호입니다.
//이 값은 특별한 일이 없는 한 변하지 않습니다.
#define MULTICAST_IP "225.0.0.3"
#define MULTICAST_PORT 50002

//멀티캐스트 송신 함수
DWORD WINAPI SenderThread(LPVOID arg)
{
	int retval;

	//필요한 변수를 선언한다.
	char buffer[512];
	char execute_mode[7];
	int uid;
	char nickName[50];
	strcpy_s(execute_mode, sizeof(execute_mode), ((SETTINGS *)arg)->execute_mode);

	//실행모드를 보고 아이디와 닉네임을 불러온다.
	if (strcmp(execute_mode, "server") == 0)
	{
		uid = ((SETTINGS *)arg)->server_uid;
		strcpy_s(nickName, sizeof(nickName), ((SETTINGS *)arg)->server_nickName);
	}
	else if (strcmp(execute_mode, "client") == 0)
	{
		uid = ((SETTINGS *)arg)->client_uid;
		strcpy_s(nickName, sizeof(nickName), ((SETTINGS *)arg)->client_nickName);
	}
	else
	{
		printf("잘못된 실행모드입니다. \n");
		return 1;
	}

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
		err_quit("socket()");

	// 멀티캐스트 TTL 설정
	int ttl = 64;
	retval = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl));
	if (retval == SOCKET_ERROR)
		err_quit("setsockopt()");

	// 소켓 주소 구조체 초기화
	SOCKADDR_IN remoteaddr;
	ZeroMemory(&remoteaddr, sizeof(remoteaddr));
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = inet_addr(MULTICAST_IP);
	remoteaddr.sin_port = htons(MULTICAST_PORT);

	// 멀티캐스트 데이터 보내기
	while (1)
	{
		// 데이터 입력
		gets_s(buffer, sizeof(buffer));

		if (strcmp(execute_mode, "server") == 0)
		{
			//공지사항을 보내는 메시지인지 검사한다.
			if (strncmp(buffer, "/", 1) == 0)
			{
				char temp[512];
				strcpy_s(temp, sizeof(temp), &buffer[1]);
				strcpy_s(buffer, sizeof(buffer), "[notice]");
				strcat_s(buffer, sizeof(buffer), temp);
			}
		}
		else
		{
			//공지태그를 붙인경우 전송하지 않는다.
			if (strncmp(buffer, "[notice]", 8) == 0)
			{
				printf("오류: 공지사항 태그는 사용할 수 없습니다. \n");
				continue;
			}
		}

		// 데이터 보내기
		retval = sendto(sock, (char*)&uid, sizeof(uid), 0, (SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
		if (retval == SOCKET_ERROR)
		{
			err_display("아이디 sendto()");
			break;
		}
		retval = sendto(sock, nickName, (int)strlen(nickName) + 1, 0, (SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
		if (retval == SOCKET_ERROR)
		{
			err_display("아이디 sendto()");
			break;
		}
		retval = sendto(sock, buffer, (int)strlen(buffer) + 1, 0, (SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
		if (retval == SOCKET_ERROR)
		{
			err_display("아이디 sendto()");
			break;
		}
	}

	// closesocket()
	closesocket(sock);

	return 0;
}

//멀티캐스트 수신 함수
DWORD WINAPI ReceiverThread(LPVOID arg)
{
	int retval;

	//필요한 변수 선언
	char buffer[512];
	int uid = 0;
	char nickName[50] = { "0" };

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
		err_quit("socket()");

	// SO_REUSEADDR 옵션 설정
	BOOL optval = TRUE;
	retval = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR)
		err_quit("setsockopt()");

	// bind()
	SOCKADDR_IN localaddr;
	ZeroMemory(&localaddr, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(MULTICAST_PORT);
	retval = bind(sock, (SOCKADDR *)&localaddr, sizeof(localaddr));
	if (retval == SOCKET_ERROR)
		err_quit("bind()");

	// 멀티캐스트 그룹 가입
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	retval = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
	if (retval == SOCKET_ERROR)
		err_quit("setsockopt()");

	// 데이터 통신에 사용할 변수
	SOCKADDR_IN peeraddr;
	int addrlen = sizeof(peeraddr);

	while (1)
	{
		// 데이터 받기
		retval = recvfrom(sock, (char*)&uid, sizeof(uid), 0, (SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR)
		{
			err_display("아이디 recvfrom()");
			break;
		}
		retval = recvfrom(sock, nickName, sizeof(nickName), 0, (SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR)
		{
			err_display("닉네임 recvfrom()");
			break;
		}
		retval = recvfrom(sock, buffer, sizeof(buffer), 0, (SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR)
		{
			err_display("데이터 recvfrom()");
			break;
		}

		//받은 파일을 검사한다
		if (strncmp(buffer, "[notice]", 8) == 0)
		{
			//공지 메시지 출력
			textcolor(YELLOW);
			printf("%s 공지: %s\n", nickName, &buffer[8]);
			textcolor(RESET);
		}
		else
			printf("%s(%d): %s\n", nickName, uid, buffer);
	}

	// 멀티캐스트 그룹 탈퇴
	retval = setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
	if (retval == SOCKET_ERROR)
		err_quit("setsockopt()");

	// closesocket()
	closesocket(sock);

	return 0;
}

//메인 함수
int main()
{
	int retval;

	//화면 조정
	system("mode con cols=75 lines=35");
	setScreenBufferSize(75, 9000);

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	
	//설정값 받아오기
	SETTINGS sets;
	retval = importSettings(&sets);
	if (retval != 0)
		return 1;

	//스레드 실행
	//sendNotice 함수에 매개변수로 구조체 전달해야 함
	HANDLE hThreads[2];
	hThreads[0] = CreateThread(NULL, 0, SenderThread, &sets, 0, NULL);
	hThreads[1] = CreateThread(NULL, 0, ReceiverThread, &sets, 0, NULL);

	//멀티채팅 서비스를 시작한다고 알린다.
	textcolor(SKY_BLUE);
	printf("멀티채팅 서비스를 시작합니다. \n");
	textcolor(RESET);

	//스레드가 종료될 때까지 대기한다.
	WaitForMultipleObjects(2, hThreads, FALSE, INFINITE);

	//프로그램 종료
	WSACleanup();
	system("pause");
	return 0;
}