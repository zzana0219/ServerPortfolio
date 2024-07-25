﻿#pragma comment(lib, "ws2_32")
#pragma comment(lib, "Winmm")

#include "RingBuffer.h"
#include"PacketDefine.h"
#include <iostream>
#include <WS2tcpip.h>
#include <vector>
//#include <crtdbg.h>
//#include<conio.h>

using namespace std;

bool g_bShutDown = false;

#pragma pack(push, 1)
typedef struct st_Header
{
	BYTE	byCode;			// 패킷코드 0x89 고정.
	BYTE	bySize;			// 패킷 사이즈.
	BYTE	byType;			// 패킷타입.
}ST_HEADER;

typedef struct st_AllocClientID
{
	int ID;
	BYTE Direction;
	u_short X;
	u_short Y;
	BYTE HP;
}ALLOC_CLIENT_ID;

typedef struct st_CreateClient
{
	int ID;
	BYTE Direction;
	u_short X;
	u_short Y;
	BYTE HP;
}CREATE_CLIENT;

typedef struct st_DeleteClient
{
	int ID;
}DELETE_CLIENT;

typedef struct st_cs_Move_Start
{
	BYTE Direction;
	u_short X;
	u_short Y;
}CS_MOVE_START;

typedef struct st_cs_Move_Stop
{
	BYTE Direction;
	u_short X;
	u_short Y;
}CS_MOVE_STOP;

typedef struct st_sc_Move
{
	int ID;
	BYTE Direction;
	u_short X;
	u_short Y;
}ST_SC_MOVE;

typedef struct st_cs_Attack
{
	BYTE Direction;
	u_short X;
	u_short Y;
}CS_ATTACK;

typedef struct st_sc_Attack
{
	int ID;
	BYTE Direction;
	u_short X;
	u_short Y;
}SC_ATTACK;

typedef struct st_sc_Damage
{
	int AttackID;
	int DamageID;
	BYTE DamageHP;
}SC_DAMAGE;

// 세션 + 플레이어 객체
typedef struct PlayerInfo
{
	// 세션용
	SOCKET Socket;		// 현 접속의 TCP 소켓
	SOCKADDR_IN Addr;	// 소켓 주소

	int ID;				// 접속자의 고유 세션 ID
	CRingBuffer sendQ;	// 송신 큐
	CRingBuffer recvQ;	// 수신 큐
	

	// 캐릭터용
	BYTE Direction;		// 캐릭터 방향
	u_short X;			// 캐릭터 X 좌표
	u_short Y;			// 캐릭터 Y 좌표
	BYTE HP;			// 캐릭터 HP
	DWORD dwAction;		// 캐릭터 동작 현 게임에선 의미 X
	bool isMove;
}PLAYERINFO;

#pragma pack(pop)

int tickCount;

bool G_Disconnect = false;
int G_ID = 0;
SOCKET ListenSocket;
vector<PLAYERINFO*> playerList;

// 서버 초기화 및 세팅
BOOL SettingNetWork();
void NetworkIOProcess();
void Update();

// 연결, 읽기, 쓰기 프로세스
BOOL AcceptProc();
void ReadProc(PLAYERINFO* player);
void WriteProc(PLAYERINFO* player);

// 연결 끊긴 클라이언트 처리
void DisconnectClient(PLAYERINFO* player);

// 개인, 단체에게 메세지 전달
void SendUniCast(PLAYERINFO* player, char* header, int h_size, char* buffer, int b_size);
void SendBroadCast(PLAYERINFO* player, char* header, int h_size, char* buffer, int b_size);

// 패킷 프로세스
bool PacketProc(PlayerInfo* player, BYTE pakcetType, char* pPacket);

// 각 패킷 처리 프로세스
bool netPacketProc_MoveStart(PlayerInfo* player, char* pPacket);
bool netPacketProc_MoveStop(PlayerInfo* player, char* pPacket);
bool netPacketProc_Attack_1(PlayerInfo* player, char* pPacket);
bool netPacketProc_Attack_2(PlayerInfo* player, char* pPacket);
bool netPacketProc_Attack_3(PlayerInfo* player, char* pPacket);

// 데미지 프로세스
bool DamageProc(PlayerInfo* player, BYTE range_X, BYTE range_Y, int damage);

// 클라가 사용하게 할 패킷 선언 자동화
void mpAllocClientID(ST_HEADER* pHeader, ALLOC_CLIENT_ID* pPacket, int id, BYTE direction, u_short x, u_short y, BYTE hp);
void mpCreateClient(ST_HEADER* pHeader, CREATE_CLIENT* pPacket, int id, BYTE direction, u_short x, u_short y, BYTE hp);
void mpDeleteClient(ST_HEADER* pHeader, DELETE_CLIENT* pPacket, int id);
void mpMoveStart(ST_HEADER* pHeader, ST_SC_MOVE* pPacket, int id, BYTE direction, u_short x, u_short y);
void mpMoveStop(ST_HEADER* pHeader, ST_SC_MOVE* pPacket, int id, BYTE direction, u_short x, u_short y);
void mpMoveAttack1(ST_HEADER* pHeader, SC_ATTACK* pPacket, int id, BYTE direction, u_short x, u_short y);
void mpMoveAttack2(ST_HEADER* pHeader, SC_ATTACK* pPacket, int id, BYTE direction, u_short x, u_short y);
void mpMoveAttack3(ST_HEADER* pHeader, SC_ATTACK* pPacket, int id, BYTE direction, u_short x, u_short y);

int main()
{
	timeBeginPeriod(1); // 타이밍 정확도를 높임

	LARGE_INTEGER frequency;
	LARGE_INTEGER start, end;
	double elapsedTime;

	// 성능 카운터 주파수 가져오기
	QueryPerformanceFrequency(&frequency);

	if (!SettingNetWork()) return 1;

	tickCount = GetTickCount64();

	while (!g_bShutDown)
	{
		NetworkIOProcess();

		// 프레임 시작 시간 가져오기
		QueryPerformanceCounter(&start);

		Update();

		// 프레임 종료 시간 가져오기
		QueryPerformanceCounter(&end);

		// 경과 시간 계산 (단위: 초)
		elapsedTime = static_cast<double>(end.QuadPart - start.QuadPart) / frequency.QuadPart;

		// 50 FPS 유지하기 위해 대기 시간 계산
		double targetTime = 1.0 / 50.0; // 50 FPS에 해당하는 시간
		if (elapsedTime < targetTime)
		{
			// 남은 시간 동안 대기
			Sleep(static_cast<DWORD>((targetTime - elapsedTime) * 1000.0));
		}

	}
	timeEndPeriod(1); // 타이밍 정확도 원래대로 복구

	// 서버 종료 대기 서버는 함부로 종료해도 안된다.
	// DB에 저장할 데이터나 기타 마무리 할 일들이 모두 끝났는지 확인 한 뒤에 꺼주어야한다.
	
	return 0;
}

// 초기 소켓 네트워크 설정
BOOL SettingNetWork()
{
	srand(time(NULL));

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;
	printf("WSAStartup #\n");

	// socket
	ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ListenSocket == INVALID_SOCKET)
		return false;

	LINGER l_option = { 0,0 };
	int lingerRet = setsockopt(ListenSocket, SOL_SOCKET, SO_LINGER, (char*)&l_option, sizeof(l_option));
	if (lingerRet == SOCKET_ERROR) return false;

	// 서버 주소 데이터
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, ' ', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(Server_Port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind
	int bindNum = bind(ListenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (bindNum == SOCKET_ERROR)
	{
		printf("Bind [%d]\n", GetLastError());
		return false;
	}
	printf("Bind OK # Port : %d\n", Server_Port);

	// listen
	int listenNum = listen(ListenSocket, SOMAXCONN);
	if (listenNum == SOCKET_ERROR)
	{
		printf("Listen [%d]\n", GetLastError());
		return false;
	}
	printf("Listen OK #\n");

	// 논블로킹 전환
	u_long on = 1;
	int nonblockNum = ioctlsocket(ListenSocket, FIONBIO, &on);
	if (nonblockNum == SOCKET_ERROR)
	{
		printf("NonBlock [%d]\n", GetLastError());
		return false;
	}
	return true;
}

// 업데이트 네트워크
void NetworkIOProcess()
{
	printf("GetTickCount : %d\n", GetTickCount64() - tickCount);
	fd_set r_set;
	fd_set w_set;
	FD_ZERO(&r_set);
	FD_ZERO(&w_set);
	FD_SET(ListenSocket, &r_set);
	timeval t = { 0,0 };

	// 받는거는 항상 세팅, 보낼게 잇으면 세팅
	vector<PLAYERINFO*>::iterator settingIter;
	for (settingIter = playerList.begin(); settingIter != playerList.end(); ++settingIter)
	{
		if ((*settingIter)->Socket == INVALID_SOCKET) { continue; }

		FD_SET((*settingIter)->Socket, &r_set);

		if ((*settingIter)->sendQ.GetUseSize() > 0)
			FD_SET((*settingIter)->Socket, &w_set);
	}

	// 나중에 확인
	int selectNum = select(NULL, &r_set, &w_set, NULL, &t);
	if (selectNum == SOCKET_ERROR)
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			printf("select [%d]\n", GetLastError());
			return;
		}
	}

	// 보낼게 하나라도 있으면
	if (selectNum > 0)
	{
		// 클라이언트 접속시
		if (FD_ISSET(ListenSocket, &r_set))
		{
			bool isConnect = AcceptProc();
			if (!isConnect)
			{
				printf("클라 연결 실패\n");
				return;
			}
		}

		// 받아야할, 보내야할 클라 소켓일시
		vector<PLAYERINFO*>::iterator readIter;
		for (readIter = playerList.begin(); readIter != playerList.end(); ++readIter)
		{
			if ((*readIter)->Socket == INVALID_SOCKET) continue;

				// 받는 소켓
			if (FD_ISSET((*readIter)->Socket, &r_set))
				ReadProc((*readIter));

			// 보내는 소켓
			if (FD_ISSET((*readIter)->Socket, &w_set))
					WriteProc((*readIter));
		}
	}

	if (G_Disconnect)
	{
		vector<PLAYERINFO*>::iterator disconnetIter;
		for (disconnetIter = playerList.begin(); disconnetIter != playerList.end();)
		{
			if ((*disconnetIter)->Socket == INVALID_SOCKET)
			{
				delete[] * disconnetIter; // PLAYERINFO 객체의 메모리 해제
				disconnetIter = playerList.erase(disconnetIter);
			}
			else
				disconnetIter++;
		}
		G_Disconnect = false;
	}
}

void Update()
{
	printf("Update GetTickCount : %d\n", GetTickCount64() - tickCount);

	vector<PLAYERINFO*>::iterator playerIter;
	for (playerIter = playerList.begin(); playerIter != playerList.end(); ++playerIter)
	{
		if (!(*playerIter)->isMove) continue;

		switch ((*playerIter)->Direction)
		{
		case dfPACKET_MOVE_DIR_LL:
			if ((*playerIter)->X <= dfRANGE_MOVE_LEFT) return;
			(*playerIter)->X -= 3;
			printf("# gameRun : LL #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_LU:
			if ((*playerIter)->X <= dfRANGE_MOVE_LEFT) return;
			if ((*playerIter)->Y <= dfRANGE_MOVE_TOP) return;
			(*playerIter)->X -= 3;
			(*playerIter)->Y -= 2;
			printf("# gameRun : LU #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_UU:
			if ((*playerIter)->Y <= dfRANGE_MOVE_TOP) return;
			(*playerIter)->Y -= 2;
			printf("# gameRun : UU #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_RU:
			if ((*playerIter)->X >= dfRANGE_MOVE_RIGHT) return;
			if ((*playerIter)->Y <= dfRANGE_MOVE_TOP) return;
			(*playerIter)->X += 3;
			(*playerIter)->Y -= 2;
			printf("# gameRun : RU #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_RR:
			if ((*playerIter)->X >= dfRANGE_MOVE_RIGHT) return;
			(*playerIter)->X += 3;
			printf("# gameRun : RR #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_RD:
			if ((*playerIter)->X >= dfRANGE_MOVE_RIGHT) return;
			if ((*playerIter)->Y >= dfRANGE_MOVE_BOTTOM) return;
			(*playerIter)->X += 3;
			(*playerIter)->Y += 2;
			printf("# gameRun : RD #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_DD:
			if ((*playerIter)->Y >= dfRANGE_MOVE_BOTTOM) return;
			(*playerIter)->Y += 2;
			printf("# gameRun : DD #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_LD:
			if ((*playerIter)->X <= dfRANGE_MOVE_LEFT) return;
			if ((*playerIter)->Y >= dfRANGE_MOVE_BOTTOM) return;
			(*playerIter)->X -= 3;
			(*playerIter)->Y += 2;
			printf("# gameRun : LD #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		}
	}
}

// 플레이어 연결시 인게임 유저 데이터 적용
BOOL AcceptProc() 
{
	if (playerList.size() > FD_SETSIZE - 1) { printf("인원수 꽉 참\n"); return false; }

	SOCKADDR_IN clientAddr;
	int addlen = sizeof(clientAddr);
	SOCKET clientSock = accept(ListenSocket, (SOCKADDR*)&clientAddr, &addlen);
	if (clientSock == INVALID_SOCKET)
	{
		printf("Accept [%d]\n", GetLastError());
		return false;
	}

	WCHAR szClientIP[16] = { 0 };
	InetNtop(AF_INET, &clientAddr.sin_addr, szClientIP, 16);
	wprintf(L"Connect # IP : %ls / SessionID : %d\n", szClientIP, G_ID);

	// 새로운 클라 정보 생성
	PLAYERINFO* newClient = new PLAYERINFO;
	newClient->Socket = clientSock;
	newClient->Addr = clientAddr;

	newClient->ID = G_ID++;
	newClient->HP = 100;
	newClient->X = rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) + dfRANGE_MOVE_LEFT;
	newClient->Y = rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) + dfRANGE_MOVE_TOP;
	newClient->dwAction = newClient->Direction = dfPACKET_MOVE_DIR_LL;

	newClient->sendQ = CRingBuffer(4096);
	newClient->recvQ = CRingBuffer(4096);

	newClient->isMove = false;

	printf("# PACKET_CONNECT # SessionID : %d\n", newClient->ID);

	// 클라에게 생성 메세지
	ST_HEADER header;
	ALLOC_CLIENT_ID allocID;
	mpAllocClientID(&header, &allocID, newClient->ID, newClient->Direction, newClient->X, newClient->Y, newClient->HP);
	SendUniCast(newClient, (char*)&header, sizeof(ST_HEADER), (char*)&allocID, sizeof(ALLOC_CLIENT_ID));

	playerList.push_back(newClient);
	
	// 다른 클라들에게 새로운 클라 생성 메세지
	CREATE_CLIENT createClient;
	mpCreateClient(&header, &createClient, newClient->ID, newClient->Direction, newClient->X, newClient->Y, newClient->HP);
	SendBroadCast(newClient, (char*)&header, sizeof(ST_HEADER), (char*)&createClient, sizeof(CREATE_CLIENT));

	printf("Create Character # SessionID : %d\tX : %d\t\tY : %d\n", newClient->ID, newClient->X, newClient->Y);

	// 클라에게 이전에 있던 유저들 생성 메세지
	vector<PLAYERINFO*>::iterator anotherClientIter;
	for (anotherClientIter = playerList.begin(); anotherClientIter != playerList.end(); ++anotherClientIter)
	{
		if ((*anotherClientIter) == newClient)  continue;
		CREATE_CLIENT anoterClient;
		mpCreateClient(&header, &anoterClient, (*anotherClientIter)->ID, (*anotherClientIter)->Direction, (*anotherClientIter)->X, (*anotherClientIter)->Y, (*anotherClientIter)->HP);
		SendUniCast(newClient, (char*)&header, sizeof(ST_HEADER), (char*)&anoterClient, sizeof(CREATE_CLIENT));

		// 만일 해당 클라가 접속 중에 움직이고 있었다면
		if ((*anotherClientIter)->isMove)
		{
			ST_SC_MOVE sendMove;
			mpMoveStart(&header, &sendMove, (*anotherClientIter)->ID, (*anotherClientIter)->Direction, (*anotherClientIter)->X, (*anotherClientIter)->Y);
			SendUniCast(newClient, (char*)&header, sizeof(ST_HEADER), (char*)&sendMove, sizeof(ST_SC_MOVE));
		}
	}
	return true;
}

// 각 플레이어 recv 링버퍼에 있는 데이터 읽기
void ReadProc(PLAYERINFO* player)
{
	// recv하고 끊어진 경우, 진짜 오류인 경우 걸러내기
	char r_buffer[1000];
	int recvNum = recv(player->Socket, r_buffer, 1000, 0);
	if (recvNum == SOCKET_ERROR || recvNum == 0)
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			if (recvNum == 0)
			{
				DisconnectClient(player);
				return;
			}
			// 10054 오류 : 클라쪽 피어별 연결 다시 설정?
			if(GetLastError() == WSAECONNRESET)
				DisconnectClient(player);
			printf("recv [%d]\n", GetLastError());
			return;
		}
	}

	if (player->recvQ.GetFreeSize() <= 0) return;
	player->recvQ.Enqueue(r_buffer, recvNum);
	
	while (player->recvQ.GetUseSize() > 0)
	{
		char packetBuffer[1000];
		player->recvQ.Peek(packetBuffer, sizeof(ST_HEADER));
		ST_HEADER* recvHeader = (ST_HEADER*)packetBuffer;
		if (recvHeader->byCode != dfNETWORK_PACKET_CODE) { printf("dfNETWORK_PACKET_CODE(0x89) 아님 [%02X]\n", recvHeader->byCode); DisconnectClient(player); return; }
		int packetSize = recvHeader->bySize;
		player->recvQ.MoveFront(sizeof(ST_HEADER));
	
		char packet[1000];
		int packetNum = player->recvQ.Peek(packet, packetSize);
		if (packetNum <= 0) return;

		PacketProc(player, recvHeader->byType, packet);
	}
}

// 각 플레이어 send 링버퍼에 있는 데이터 보내기
void WriteProc(PLAYERINFO* player)
{
	if (player->sendQ.GetUseSize() <= 0) return;

	int useSize = player->sendQ.GetUseSize();
	char* buffer = new char[useSize];
	
	int peekSize = player->sendQ.Peek(buffer, useSize);
	if (peekSize != useSize)
	{
		printf("보내기 사이즈 안맞음\n");
		delete[] buffer;
		return;
	}
	
	int sendNum = send(player->Socket, buffer, peekSize, 0);
	if (sendNum == SOCKET_ERROR)
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			printf("send [%d]\n", GetLastError());
			delete[] buffer;
			return;
		}
	}
	
	player->sendQ.MoveFront(peekSize);
	delete[] buffer;
}

// 해당 유저에게만 보내기
void SendUniCast(PLAYERINFO* player, char* header, int h_size, char* buffer, int b_size)
{
	player->sendQ.Enqueue(header, h_size);
	player->sendQ.Enqueue(buffer, b_size);
}

// 해당 유저를 제외한 모두에게 보내기
void SendBroadCast(PLAYERINFO* player, char* header, int h_size, char* buffer, int b_size)
{
	vector<PLAYERINFO*>::iterator sendIter;
	for (sendIter = playerList.begin(); sendIter != playerList.end(); ++sendIter)
	{
		if ((*sendIter) == player) continue;
		if((*sendIter)->Socket != INVALID_SOCKET)
			SendUniCast((*sendIter), header, h_size, buffer, b_size);
	}
}

// 유저가 끊겼을 경우
void DisconnectClient(PLAYERINFO* player)
{
	vector<PLAYERINFO*>::iterator deleteIter = playerList.begin();
	while (deleteIter != playerList.end())
	{
		if ((*deleteIter) != player)
			deleteIter++;

		ST_HEADER header;
		DELETE_CLIENT packet;
		mpDeleteClient(&header, &packet, player->ID);
		SendBroadCast(player, (char*)&header, sizeof(ST_HEADER), (char*)&packet, sizeof(DELETE_CLIENT));

		printf("Disconnect # Session ID : %d\n", player->ID);

		closesocket(player->Socket);
		player->Socket = INVALID_SOCKET;

		deleteIter = playerList.end();
		G_Disconnect = true;
		break;
	}
}

// 패킷 프로세스
bool PacketProc(PlayerInfo* player, BYTE packetType, char* pPacket)
{
	switch (packetType)
	{
	case dfPACKET_CS_MOVE_START:
		netPacketProc_MoveStart(player, pPacket);
		printf("# PACKET_MOVESTART # SessionID : [%d] / Direction : [%X] / X : [%d] / Y : [%d]\n", player->ID, player->Direction, player->X, player->Y);
	break;
	case dfPACKET_CS_MOVE_STOP:
		netPacketProc_MoveStop(player, pPacket);
		printf("# PACKET_MOVESTOP # SessionID : [%d] / Direction : [%X] / X : [%d] / Y : [%d]\n", player->ID, player->Direction, player->X, player->Y);
	break;
	case dfPACKET_CS_ATTACK1:
		netPacketProc_Attack_1(player, pPacket);
		printf("# PACKET_ATTACK1 # SessionID : [%d] / Direction : [%X] / X : [%d] / Y : [%d]\n", player->ID, player->Direction, player->X, player->Y);
	break;
	case dfPACKET_CS_ATTACK2:
		netPacketProc_Attack_2(player, pPacket);
		printf("# PACKET_ATTACK2 # SessionID : [%d] / Direction : [%X] / X : [%d] / Y : [%d]\n", player->ID, player->Direction, player->X, player->Y);
	break;
	case dfPACKET_CS_ATTACK3:
		netPacketProc_Attack_3(player, pPacket);
		printf("# PACKET_ATTACK3 # SessionID : [%d] / Direction : [%X] / X : [%d] / Y : [%d]\n", player->ID, player->Direction, player->X, player->Y);
	break;
	}
	return TRUE;
}

bool netPacketProc_MoveStart(PlayerInfo* player, char* pPacket)
{
	CS_MOVE_START* m_packet = (CS_MOVE_START*)pPacket;
	player->recvQ.MoveFront(sizeof(CS_MOVE_START));

	// 데드 레커닝 문제가 일어나면 아래 코드에서 터짐

	//메세지 수신 로그 확인
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 끊어버림
	if (abs(player->X - m_packet->X) > dfERROR_RANGE || abs(player->Y - m_packet->Y) > dfERROR_RANGE)
	{
		DisconnectClient(player);
		return FALSE;
	}

	// 방향을 변경
	switch (m_packet->Direction)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		player->dwAction = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		player->dwAction = dfPACKET_MOVE_DIR_LL;
		break;
	}

	// 동작을 변경. 지금 구현에선 동작번호가 방향값
	player->Direction = m_packet->Direction;

	// X,Y 좌표 갱신
	player->X = m_packet->X;
	player->Y = m_packet->Y;

	player->isMove = true;

	// 현재 접속중인 사용자에게 모든 패킷을 뿌린다. 당사자 제외
	ST_HEADER sendHeader;
	ST_SC_MOVE sendMove;
	mpMoveStart(&sendHeader, &sendMove, player->ID, player->Direction, player->X, player->Y);
	SendBroadCast(player, (char*)&sendHeader, sizeof(ST_HEADER), (char*)&sendMove, sizeof(ST_SC_MOVE));
	return TRUE;
}

bool netPacketProc_MoveStop(PlayerInfo* player, char* pPacket)
{
	CS_MOVE_STOP* m_packet = (CS_MOVE_STOP*)pPacket;
	player->recvQ.MoveFront(sizeof(CS_MOVE_STOP));

	//메세지 수신 로그 확인
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 끊어버림
	if (abs(player->X - m_packet->X) > dfERROR_RANGE || abs(player->Y - m_packet->Y) > dfERROR_RANGE)
	{
		DisconnectClient(player);
		return FALSE;
	}

	player->Direction = m_packet->Direction;
	player->X = m_packet->X;
	player->Y = m_packet->Y;

	player->isMove = false;

	ST_HEADER sendHeader;
	ST_SC_MOVE sendMove;
	mpMoveStop(&sendHeader, &sendMove, player->ID, player->Direction, player->X, player->Y);
	SendBroadCast(player, (char*)&sendHeader, sizeof(ST_HEADER), (char*)&sendMove, sizeof(ST_SC_MOVE));
	return TRUE;
}

bool netPacketProc_Attack_1(PlayerInfo* player, char* pPacket)
{
	CS_ATTACK* a_packet = (CS_ATTACK*)pPacket;
	player->recvQ.MoveFront(sizeof(CS_ATTACK));

	ST_HEADER sendHeader;
	SC_ATTACK sendAttack;
	mpMoveAttack1(&sendHeader, &sendAttack, player->ID, player->Direction, player->X, player->Y);
	SendBroadCast(player, (char*)&sendHeader, sizeof(ST_HEADER), (char*)&sendAttack, sizeof(SC_ATTACK));

	DamageProc(player, dfATTACK1_RANGE_X, dfATTACK1_RANGE_Y, 10);
	return TRUE;
}

bool netPacketProc_Attack_2(PlayerInfo* player, char* pPacket)
{
	CS_ATTACK* a_packet = (CS_ATTACK*)pPacket;
	player->recvQ.MoveFront(sizeof(CS_ATTACK));

	ST_HEADER sendHeader;
	SC_ATTACK sendAttack;
	mpMoveAttack2(&sendHeader, &sendAttack, player->ID, player->Direction, player->X, player->Y);
	SendBroadCast(player, (char*)&sendHeader, sizeof(ST_HEADER), (char*)&sendAttack, sizeof(SC_ATTACK));

	DamageProc(player, dfATTACK2_RANGE_X, dfATTACK2_RANGE_Y, 20);
	return TRUE;
}

bool netPacketProc_Attack_3(PlayerInfo* player, char* pPacket)
{
	CS_ATTACK* a_packet = (CS_ATTACK*)pPacket;
	player->recvQ.MoveFront(sizeof(CS_ATTACK));

	ST_HEADER sendHeader;
	SC_ATTACK sendAttack;
	mpMoveAttack3(&sendHeader, &sendAttack, player->ID, player->Direction, player->X, player->Y);
	SendBroadCast(player, (char*)&sendHeader, sizeof(ST_HEADER), (char*)&sendAttack, sizeof(SC_ATTACK));

	DamageProc(player, dfATTACK3_RANGE_X, dfATTACK3_RANGE_Y, 30);
	return TRUE;
}

bool DamageProc(PlayerInfo* player, BYTE range_X, BYTE range_Y, int damage)
{
	vector<PLAYERINFO*>::iterator anotherClientIter;
	for (anotherClientIter = playerList.begin(); anotherClientIter != playerList.end(); ++anotherClientIter)
	{
		// 내가 나를 때릴순 없으니 넘김
		if ((*anotherClientIter) == player) continue;
		// 공격1 범위 안에 있으면
		if (abs(player->X - (*anotherClientIter)->X) < range_X && abs(player->Y - (*anotherClientIter)->Y) < range_Y)
		{
			// 포지션이 오른쪽을 보면서 적이 오른쪽에 있을때거나 포지션이 왼쪽을 보면서 적이 왼쪽에 있을때
			if ((player->X <= (*anotherClientIter)->X && player->Direction == dfPACKET_MOVE_DIR_RR) ||
				(player->X >= (*anotherClientIter)->X && player->Direction == dfPACKET_MOVE_DIR_LL))
			{
				ST_HEADER damageHeader = { 0x89, sizeof(SC_DAMAGE), dfPACKET_SC_DAMAGE };
				(*anotherClientIter)->HP -= damage;
				SC_DAMAGE sendDamage = { player->ID, (*anotherClientIter)->ID, (*anotherClientIter)->HP };
				if ((*anotherClientIter)->HP <= 0 || (*anotherClientIter)->HP > 100)
					DisconnectClient((*anotherClientIter));
				else
					SendBroadCast(NULL, (char*)&damageHeader, sizeof(ST_HEADER), (char*)&sendDamage, sizeof(SC_DAMAGE));
				break;
			}
		}
	}
	anotherClientIter = playerList.end();
	return TRUE;
}


void mpAllocClientID(ST_HEADER* pHeader, ALLOC_CLIENT_ID* pPacket, int id, BYTE direction, u_short x, u_short y, BYTE hp)
{
	pHeader->byCode = dfNETWORK_PACKET_CODE;
	pHeader->bySize = sizeof(ALLOC_CLIENT_ID);
	pHeader->byType = dfPACKET_SC_CREATE_MY_CHARACTER;

	pPacket->ID = id;
	pPacket->Direction = direction;
	pPacket->X = x;
	pPacket->Y = y;
	pPacket->HP = hp;
}

void mpCreateClient(ST_HEADER* pHeader, CREATE_CLIENT* pPacket, int id, BYTE direction, u_short x, u_short y, BYTE hp)
{
	pHeader->byCode = dfNETWORK_PACKET_CODE;
	pHeader->bySize = sizeof(CREATE_CLIENT);
	pHeader->byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;

	pPacket->ID = id;
	pPacket->Direction = direction;
	pPacket->X = x;
	pPacket->Y = y;
	pPacket->HP = hp;
}

void mpDeleteClient(ST_HEADER* pHeader, DELETE_CLIENT* pPacket, int id)
{
	pHeader->byCode = dfNETWORK_PACKET_CODE;
	pHeader->bySize = sizeof(DELETE_CLIENT);
	pHeader->byType = dfPACKET_SC_DELETE_CHARACTER;

	pPacket->ID = id;
}

void mpMoveStart(ST_HEADER* pHeader, ST_SC_MOVE* pPacket, int id, BYTE direction, u_short x, u_short y)
{
	pHeader->byCode = dfNETWORK_PACKET_CODE;
	pHeader->bySize = sizeof(ST_SC_MOVE);
	pHeader->byType = dfPACKET_SC_MOVE_START;

	pPacket->ID = id;
	pPacket->Direction = direction;
	pPacket->X = x;
	pPacket->Y = y;
}

void mpMoveStop(ST_HEADER* pHeader, ST_SC_MOVE* pPacket, int id, BYTE direction, u_short x, u_short y)
{
	pHeader->byCode = dfNETWORK_PACKET_CODE;
	pHeader->bySize = sizeof(ST_SC_MOVE);
	pHeader->byType = dfPACKET_SC_MOVE_STOP;

	pPacket->ID = id;
	pPacket->Direction = direction;
	pPacket->X = x;
	pPacket->Y = y;
}

void mpMoveAttack1(ST_HEADER* pHeader, SC_ATTACK* pPacket, int id, BYTE direction, u_short x, u_short y)
{
	pHeader->byCode = dfNETWORK_PACKET_CODE;
	pHeader->bySize = sizeof(ST_SC_MOVE);
	pHeader->byType = dfPACKET_SC_ATTACK1;

	pPacket->ID = id;
	pPacket->Direction = direction;
	pPacket->X = x;
	pPacket->Y = y;
}

void mpMoveAttack2(ST_HEADER* pHeader, SC_ATTACK* pPacket, int id, BYTE direction, u_short x, u_short y)
{
	pHeader->byCode = dfNETWORK_PACKET_CODE;
	pHeader->bySize = sizeof(SC_ATTACK);
	pHeader->byType = dfPACKET_SC_ATTACK2;

	pPacket->ID = id;
	pPacket->Direction = direction;
	pPacket->X = x;
	pPacket->Y = y;
}

void mpMoveAttack3(ST_HEADER* pHeader, SC_ATTACK* pPacket, int id, BYTE direction, u_short x, u_short y)
{
	pHeader->byCode = dfNETWORK_PACKET_CODE;
	pHeader->bySize = sizeof(SC_ATTACK);
	pHeader->byType = dfPACKET_SC_ATTACK3;

	pPacket->ID = id;
	pPacket->Direction = direction;
	pPacket->X = x;
	pPacket->Y = y;
}