#include <stdio.h>
#include <winsock2.h>

//���ڻ� ������ �� ���� ��� ���
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


//���α׷��� ������ ������ ����ü
typedef struct settings
{
	char execute_mode[7];		//���α׷��� ���۸��
	char server_ip[16];			//������ IP �ּ�
	int server_mainPort;		//������ �� ��Ʈ
	int server_requestPort;		//������ Ǯ���� ��Ʈ
	int server_uid;				//������ ���̵�
	char server_nickName[50];	//������ �г���
	int client_uid;				//Ŭ���̾�Ʈ�� ���̵�
	char client_nickName[50];	//Ŭ���̾�Ʈ�� �г���
}SETTINGS;

/* Console.c �� �Լ� ���
--------------------------------------------*/
//�Է� ���۸� ����ش�.
void clearInputBuffer();

//�ܼ��� ���ڻ��� �����Ѵ�.
int textcolor(unsigned short color_number);

//Ŀ���� ��ġ�� �����Ѵ�.
int setCursorPos(short x, short y);

//ȭ�� ���� ũ�⸦ �����Ѵ�.
int setScreenBufferSize(short x, short y);


/* Error.c �� �Լ� ���
--------------------------------------------*/
// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg);

// ���� �Լ� ���� ���
void err_display(char *msg);


/* File.c �� �Լ� ���
--------------------------------------------*/
//���������� �ҷ����� �Լ�
int importSettings(_Out_ SETTINGS *sets);