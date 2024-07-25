#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>
#include <iostream>
#include <vector>
#include <conio.h>
#include "Console.h"

void KeyBoard(SOCKET sock);
void Network(SOCKET sock);
void Render();
void Sprite_Draw(int iX, int iY);
void Buffer_Clear(void);
void Buffer_Flip(void);

#pragma pack(push, 1)
typedef struct st_Packet_Header
{
	int Type;
}ST_HEADER;

typedef struct st_Packet_ID
{
	int Type;
	int ID;
	int X;
	int Y;
}ST_ID;

typedef struct st_Packet_Create
{
	int Type;
	int ID;
	int X;
	int Y;
}ST_CREATE;

typedef struct st_Packet_Delete
{
	int Type;
	int ID;
	int X;
	int Y;
}ST_DELETE;

typedef struct st_Packet_Move
{
	int Type;
	int ID;
	int X;
	int Y;
}ST_MOVE;

typedef struct playerInfo
{
	SOCKET Socket;
	int ID;
	int X;
	int Y;
}PLAYERINFO;
#pragma pack(pop)

char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];
int myID;
PLAYERINFO* myInfo = new PLAYERINFO;
std::vector<PLAYERINFO*> pList;
std::vector<PLAYERINFO*>::iterator pListIter;

int main()
{
	cs_Initial();

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) return 1;

	//bind
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, ' ', sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(3000);

	InetPton(AF_INET, L"127.0.0.1", &sockAddr.sin_addr);
	// connect
	int connectNum = connect(sock, (SOCKADDR*)&sockAddr, sizeof(sockAddr));
	if (connectNum == SOCKET_ERROR) { printf("비정상연결 [%d]", GetLastError()); return 1; }

	// 논블로킹
	u_long on = 1;
	int ioct = ioctlsocket(sock, FIONBIO, &on);
	if (ioct == SOCKET_ERROR) return 1;

	while (1)
	{
		KeyBoard(sock);
		Network(sock);
		Render();
	}

	closesocket(sock);
	delete myInfo;

	WSACleanup();
	return 0;
}

void KeyBoard(SOCKET sock)
{
	// 키보드 입력
	if (_kbhit() && myInfo != nullptr)
	{
		int ch = _getch();
		bool isMove = false;
		switch (ch)
		{
		case 75:
			if (myInfo->X > 0)
			{
				myInfo->X--;
				isMove = true;
			}
			break;
		case 72:
			if (myInfo->Y > 0)
			{
				myInfo->Y--;
				isMove = true;
			}
			break;
		case 77:
			if (myInfo->X < dfSCREEN_WIDTH - 2)
			{
				myInfo->X++;
				isMove = true;
			}
			break;
		case 80:
			if (myInfo->Y < dfSCREEN_HEIGHT - 1)
			{
				myInfo->Y++;
				isMove = true;
			}
			break;
		}

		if (isMove)
		{
			ST_MOVE send_st_move;
			send_st_move.ID = myInfo->ID;
			send_st_move.Type = 3;
			send_st_move.X = myInfo->X;
			send_st_move.Y = myInfo->Y;

			int sendNum = send(sock, (char*)&send_st_move, sizeof(send_st_move), 0);
			if (sendNum == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					printf("비정상send [%d]\n", WSAGetLastError());
					return;
				}
			}
		}
	}
}

void Network(SOCKET sock)
{
	fd_set r_set;
	FD_ZERO(&r_set);
	FD_SET(sock, &r_set);
	timeval t = { 0,0 };

	int selectNum = select(0, &r_set, nullptr, nullptr, &t);
	if (selectNum == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			printf("비정상select [%d]\n", WSAGetLastError());
			return;
		}
	}

	// 네트워크
	char buf[16];
	int recvNum = recv(sock, buf, 16, 0);
	if (recvNum == 0 || recvNum == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			printf("비정상recv [%d]\n", WSAGetLastError());
			return;
		}
	}

	ST_HEADER* st_Header = (ST_HEADER*)&buf;
	switch (st_Header->Type)
	{
		// ID 할당
	case 0:
	{
		ST_ID* st_ID = (ST_ID*)&buf;
		myID = st_ID->ID;

		myInfo->ID = st_ID->ID;
		break;
	}
	// 별 생성
	case 1:
	{
		BOOL isCreated = false;
		ST_CREATE* st_Create = (ST_CREATE*)&buf;
		std::vector<PLAYERINFO*>::iterator pCreateIter;
		for (pCreateIter = pList.begin(); pCreateIter != pList.end(); pCreateIter++)
		{
			if ((*pCreateIter)->ID == st_Create->ID)
			{
				isCreated = true;
				break;
			}
		}

		if (!isCreated)
		{
			PLAYERINFO* player = new PLAYERINFO;
			player->ID = st_Create->ID;
			player->X = st_Create->X;
			player->Y = st_Create->Y;

			if (myID == st_Create->ID)
				myInfo = player;

			pList.push_back(player);
		}
		break;
	}
	// 별 삭제
	case 2:
	{
		ST_DELETE* st_Delete = (ST_DELETE*)&buf;
		std::vector<PLAYERINFO*>::iterator pDeleteIter;
		for (pDeleteIter = pList.begin(); pDeleteIter != pList.end(); pDeleteIter++)
		{
			if ((*pDeleteIter)->ID == st_Delete->ID)
			{
				pDeleteIter = pList.erase(pDeleteIter);
				break;
			}
		}
		break;
	}
	// 별 이동
	case 3:
	{
		ST_MOVE* st_Move = (ST_MOVE*)&buf;

		if (st_Move->ID == myID) break;
		std::vector<PLAYERINFO*>::iterator pMoveIter;
		for (pMoveIter = pList.begin(); pMoveIter != pList.end(); pMoveIter++)
		{
			if ((*pMoveIter)->ID == st_Move->ID)
			{
				(*pMoveIter)->X = st_Move->X;
				(*pMoveIter)->Y = st_Move->Y;
				break;
			}
		}
		break;
	}
	}
}

void Render()
{
	Buffer_Clear();
	auto iter = pList.begin();
	for (iter = pList.begin(); iter != pList.end(); ++iter)
	{
		Sprite_Draw((*iter)->X, (*iter)->Y);
	}
	Buffer_Flip();
}

void Sprite_Draw(int iX, int iY)
{
	if (iX < 0 || iY < 0 || iX >= dfSCREEN_WIDTH - 1 || iY >= dfSCREEN_HEIGHT)
		return;
	szScreenBuffer[iY][iX] = '*';
}

void Buffer_Clear(void)
{
	int iCnt;
	memset(szScreenBuffer, ' ', dfSCREEN_WIDTH * dfSCREEN_HEIGHT);

	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; iCnt++)
		szScreenBuffer[iCnt][dfSCREEN_WIDTH - 1] = '\0';
}

void Buffer_Flip(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; iCnt++)
	{
		cs_MoveCursor(0, iCnt);
		printf(szScreenBuffer[iCnt]);
	}
}
