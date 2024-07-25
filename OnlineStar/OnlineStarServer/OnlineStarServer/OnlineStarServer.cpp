#pragma comment(lib, "ws2_32")

#include"Console.h"
#include <iostream>
#include<vector>
#include<WS2tcpip.h>

#pragma pack(push, 1)
typedef struct st_Packet
{
	int Type;
	int ID;
	int X;
	int Y;
}PACKET;

typedef struct st_PlayerInfo
{
	SOCKET socket;
	int ID;
	int X;
	int Y;
	SOCKADDR_IN IP_Port; // IP와 Port를 포함하는 구조체
	bool isConnect;
}PLAYERINFO;
#pragma pack(pop)

char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];

SOCKET listenSock;
std::vector<PLAYERINFO*> playerList;
int _id = 0;
bool _disconnect = false;

void NetWork();
void Render();

void AcceptProc();
void RecvProc(PLAYERINFO* playerinfo);
void Disconnect(int id);
void SendUniCast(SOCKET sock, PACKET buffer);
void SendBroadCast(SOCKET sock, PACKET buffer);

void Sprite_Draw(int iX, int iY, char ch = '*');
void Buffer_Clear(void);
void Buffer_Flip(void);

int main()
{
	cs_Initial();

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSock == INVALID_SOCKET) { wprintf(L"socket [%d]\n", WSAGetLastError()); return 1; }

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, ' ', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(3000);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int bindNum = bind(listenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (bindNum == SOCKET_ERROR) { wprintf(L"bind [%d]\n", WSAGetLastError()); return 1; }

	int listenNum = listen(listenSock, SOMAXCONN);
	if (listenNum == SOCKET_ERROR) { wprintf(L"listen [%d]\n", WSAGetLastError()); return 1; }

	u_long on = 1;
	int nonblockNum = ioctlsocket(listenSock, FIONBIO, &on);
	if (nonblockNum == SOCKET_ERROR) { wprintf(L"nonblockNum [%d]\n", WSAGetLastError()); return 1; }

	while (1)
	{
		NetWork();
		Render();
	}
	return 0;
}

void NetWork()
{
	fd_set r_set;
	FD_ZERO(&r_set);
	FD_SET(listenSock, &r_set);
	timeval t = { 0,0 };

	std::vector<PLAYERINFO*>::iterator set_iter;
	for (set_iter = playerList.begin(); set_iter != playerList.end(); ++set_iter)
	{
		FD_SET((*set_iter)->socket, &r_set);
	}

	int selectNum = select(0, &r_set, NULL, NULL, NULL);
	if (selectNum == SOCKET_ERROR) { wprintf(L"select [%d]\n", WSAGetLastError()); return; }

	if (selectNum > 0)
	{
		if (FD_ISSET(listenSock, &r_set))
			AcceptProc();

		std::vector<PLAYERINFO*>::iterator recv_iter;
		for (recv_iter = playerList.begin(); recv_iter != playerList.end(); ++recv_iter)
		{
			if (FD_ISSET((*recv_iter)->socket, &r_set))
				RecvProc((*recv_iter));
		}
	}

	if (_disconnect)
	{
		std::vector<PLAYERINFO*>::iterator delete_iter = playerList.begin();
		while (delete_iter != playerList.end())
		{
			if (!(*delete_iter)->isConnect)
			{
				PACKET packet = { 2, (*delete_iter)->ID, (*delete_iter)->X, (*delete_iter)->Y };
				SendBroadCast((*delete_iter)->socket, packet);
				closesocket((*delete_iter)->socket);

				// 메모리 해제
				delete* delete_iter;

				delete_iter = playerList.erase(delete_iter); // 안전하게 삭제
			}
			else
				++delete_iter;
		}
		_disconnect = false;
	}
}

void Render()
{
	Buffer_Clear();

	auto iter = playerList.begin();
	for (; iter != playerList.end(); ++iter)
		Sprite_Draw((*iter)->X, (*iter)->Y);

	Buffer_Flip();
}

void AcceptProc()
{
	SOCKADDR_IN clientAddr;
	int addlen = sizeof(clientAddr);
	SOCKET acceptSock = accept(listenSock, (SOCKADDR*)&clientAddr, &addlen);
	if (acceptSock == INVALID_SOCKET) { wprintf(L"accept [%d]\n", WSAGetLastError()); return; }

	PLAYERINFO* clientInfo = new PLAYERINFO;
	clientInfo->ID = _id;
	_id++;
	clientInfo->socket = acceptSock;
	clientInfo->X = dfSCREEN_WIDTH / 2;
	clientInfo->Y = dfSCREEN_HEIGHT / 2;

	// 클라이언트 정보 출력 (디버그용)
	WCHAR ipStr[16];
	InetNtop(AF_INET, &(clientInfo->IP_Port.sin_addr), ipStr, 16);
	//wprintf(L"clientInfo [ID : %d] [X : %d] [Y : %d] [IP : %s] [Port : %d]\n", clientInfo->ID, clientInfo->X, clientInfo->Y, ipStr, ntohs(clientInfo->IP_Port.sin_port));

	PACKET id_packet = { 0, clientInfo->ID, clientInfo->X, clientInfo->Y };
	PACKET create_packet = { 1, clientInfo->ID, clientInfo->X, clientInfo->Y };

	// ID할당을 개인 클라에만 요청
	SendUniCast(clientInfo->socket, id_packet);

	// 리스트에 클라 정보를 추가 및 모두에게 클라 생성 요청
	playerList.push_back(clientInfo);
	SendBroadCast(NULL, create_packet);

	// 지금 연결한 개인에게 이전에 접속해 있는 클라이언트 생성 요청
	std::vector<PLAYERINFO*>::iterator send_iter;
	for (send_iter = playerList.begin(); send_iter != playerList.end(); ++send_iter)
	{
		//wprintf(L"send_iter\n");
		if (clientInfo->socket == (*send_iter)->socket) continue;
		PACKET anotherPacket = { 1,  (*send_iter)->ID,  (*send_iter)->X,  (*send_iter)->Y };
		SendUniCast(clientInfo->socket, anotherPacket);
	}
}

void RecvProc(PLAYERINFO* playerInfo)
{
	PACKET packet;
	int recvNum = recv(playerInfo->socket, (char*)&packet, sizeof(packet), 0);
	if (recvNum == SOCKET_ERROR || recvNum == 0)
	{
		if (recvNum == 0)
		{
			Disconnect(playerInfo->ID);
			wprintf(L"recv [%d]\n", WSAGetLastError());
			return;
		}
		else if (recvNum != WSAEWOULDBLOCK)
		{
			Disconnect(playerInfo->ID);
			wprintf(L"recv [%d]\n", WSAGetLastError());
			return;
		}
	}

	if ((playerInfo->ID == packet.ID) && packet.Type == 3)
	{
		playerInfo->X = packet.X;
		playerInfo->Y = packet.Y;
		//wprintf(L"RECV [TCP type : %d] [ID : %d] ( %d, %d )\n", packet.Type, packet.ID, packet.X, packet.Y);

		SendBroadCast(playerInfo->socket, packet);
	}
}

void Disconnect(int id)
{
	std::vector<PLAYERINFO*>::iterator delete_iter;
	for (delete_iter = playerList.begin(); delete_iter != playerList.end(); ++delete_iter)
	{
		if ((*delete_iter)->ID == id)
		{
			(*delete_iter)->isConnect = false;
			_disconnect = true;
			break;
		}
	}
}

void SendUniCast(SOCKET sock, PACKET buffer)
{
	int sendNum = send(sock, (char*)&buffer, sizeof(buffer), 0);
	if (sendNum == SOCKET_ERROR)
	{
		if (sendNum != WSAEWOULDBLOCK)
		{
			Disconnect(buffer.ID);
			return;
		}
	}
}

void SendBroadCast(SOCKET sock, PACKET buffer)
{
	std::vector<PLAYERINFO*>::iterator send_iter;
	for (send_iter = playerList.begin(); send_iter != playerList.end(); ++send_iter)
	{
		if ((*send_iter)->socket == sock) continue;

		if ((*send_iter)->socket != INVALID_SOCKET)
			SendUniCast((*send_iter)->socket, buffer);
	}
}

void Sprite_Draw(int iX, int iY, char ch)
{
	if (iX < 0 || iY < 0 || iX >= dfSCREEN_WIDTH - 1 || iY >= dfSCREEN_HEIGHT)
		return;
	szScreenBuffer[iY][iX] = ch;
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
