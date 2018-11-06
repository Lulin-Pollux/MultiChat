#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

//글자색 변경할 때 숫자 대신 사용
#define RESET 7
#define DARK_BLUE 1
#define DARK_GREEN 2
#define BRIGHT_BLUE 3
#define DARK_RED 4
#define DARK_PURPLE 5
#define DARK_YELLOW 6
#define DARK_WHITE 7
#define GRAY 8
#define BLUE 9
#define GREEN 10
#define SKY_BLUE 11
#define RED 12
#define PURPLE 13
#define YELLOW 14
#define WHITE 15

//우리 프로젝트에서 사용할 멀티캐스트 대표IP, PORT번호입니다.
//이 값은 특별한 일이 없는 한 변하지 않습니다.
#define MULTICAST_IP "225.0.0.2"
#define MULTICAST_PORT 50002

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

//프로그램의 설정을 저장할 구조체
typedef struct settings
{
	char execute_mode[7];		//프로그램의 시작모드
	char server_ip[16];			//서버의 IP 주소
	int server_mainPort;		//서버의 주 포트
	int server_requestPort;		//서버의 풀리퀘 포트
	int server_uid;				//서버의 아이디
	char server_nickName[50];	//서버의 닉네임
	int client_uid;				//클라이언트의 아이디
	char client_nickName[50];	//클라이언트의 닉네임
}SETTINGS;

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

//콘솔의 글자색을 변경한다.
int textcolor(unsigned short color_number)
{
	int retval = SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_number);
	return retval;
}


//멀티캐스트 송신 함수
DWORD WINAPI SenderThread(LPVOID arg)
{
	int retval;

	char buffer[512];  //데이터 통신에 사용할 변수
	int uid = ((SETTINGS *)arg)->client_uid; // 아이디가 저장될 배열
	char nickName[50]; // 닉네임이 저장될 배열
	strcpy_s(nickName, sizeof(nickName), ((SETTINGS *)arg)->client_nickName);

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

		//공지태그를 붙인경우 전송하지 않는다.
		if (strncmp(buffer, "[notice]", 8) == 0)
		{
			printf("오류: 공지사항 태그는 사용할 수 없습니다. \n");
			continue;
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
			printf("%s 공지: %s\n", nickName, buffer);
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

	system("mode con cols=80 lines=30");

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	
	//설정값 받아오기
	SETTINGS sets;
	retval = importSettings(&sets);

	//스레드 실행
	//sendNotice 함수에 매개변수로 구조체 전달해야 함
	HANDLE hThreads[3];
	hThreads[0] = CreateThread(NULL, 0, SenderThread, &sets, 0, NULL);
	hThreads[1] = CreateThread(NULL, 0, ReceiverThread, &sets, 0, NULL);
	WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);

	// 윈속 종료
	WSACleanup();
	return 0;
}