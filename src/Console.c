#include <stdio.h>
#include <Windows.h>


//입력 버퍼를 비워준다.
void clearInputBuffer()
{
	while (getchar() != '\n');
}

//콘솔의 글자색을 변경한다.
int textcolor(unsigned short color_number)
{
	int retval = SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_number);
	return retval;
}

//커서의 위치를 변경한다.
int setCursorPos(short x, short y)
{
	COORD position = { x - 1, y - 1 };
	int retval = SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);
	return retval;
}

//화면 버퍼 크기를 설정한다.
int setScreenBufferSize(short x, short y)
{
	COORD size = { x, y };
	int retval = SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), size);
	return retval;
}