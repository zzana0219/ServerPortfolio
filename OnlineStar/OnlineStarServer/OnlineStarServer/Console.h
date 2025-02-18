// Windows 의 콘솔 화면에서 커서 제어.
#ifndef __CONSOLE__
#define __CONSOLE__

#define dfSCREEN_WIDTH		81		// 콘솔 가로 80칸 + NULL
#define dfSCREEN_HEIGHT		24		// 콘솔 세로 24칸

// 콘솔 제어를 위한 준비 작업.
void cs_Initial(void);

// 콘솔 화면의 커서를 X, Y 좌표로 이동시킨다.
void cs_MoveCursor(int iPosX, int iPosY);

// 콘솔 화면을 조기화 한다.
void cs_ClearScreen(void);

#endif

