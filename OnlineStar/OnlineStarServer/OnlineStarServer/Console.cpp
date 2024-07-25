#include <windows.h>
#include <stdio.h>
#include "Console.h"

HANDLE  hConsole;

// �ܼ� ��� ���� �غ� �۾�.
void cs_Initial(void)
{
	CONSOLE_CURSOR_INFO stConsoleCursor;

	// ȭ���� Ŀ���� �Ⱥ��̰Բ� �����Ѵ�.
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize   = 1;	

	// �ܼ�ȭ�� (���ٴٵ� �ƿ�ǲ) �ڵ��� ���Ѵ�.
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(hConsole, &stConsoleCursor);
}

// �ܼ� ȭ���� Ŀ���� X, Y ��ǥ�� �̵���Ų��.
void cs_MoveCursor(int iPosX, int iPosY)
{
	COORD stCoord;
	stCoord.X = iPosX;
	stCoord.Y = iPosY;
	// ���ϴ� ��ġ�� Ŀ���� �̵���Ų��.
	SetConsoleCursorPosition(hConsole, stCoord);
}

// �ܼ� ȭ���� ����ȭ �Ѵ�.
void cs_ClearScreen(void)
{
	int iCountX, iCountY;
	DWORD dw;
	FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), ' ', 100 * 100, { 0, 0 }, &dw);
}
