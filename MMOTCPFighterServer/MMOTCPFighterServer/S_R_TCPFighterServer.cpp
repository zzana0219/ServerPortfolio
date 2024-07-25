#pragma comment(lib, "ws2_32")
#pragma comment(lib, "Winmm")

#pragma once

#include "RingBuffer.h"
#include"PacketDefine.h"
#include"SerializingBuffer.h"
#include <iostream>
#include <WS2tcpip.h>
#include <vector>
#include <list>
#include<unordered_map>

#include<conio.h>

using namespace std;

bool g_bShutDown = false;

#pragma pack(push, 1)
typedef struct st_PACKET_HEADER
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

typedef struct st_Echo
{
	int time;
}ST_ECHO;

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
	bool isConnect;

	u_short session_x;
	u_short session_y;
	//bool isChangeSession;
	int echoTime;
}PLAYERINFO;

typedef struct SessionInfo
{
	u_short X;
	u_short Y;

	list<PLAYERINFO*> playerList;
}SESSIONINFO;

typedef struct st_SESSION
{
	SOCKET Socket;			// 현 접속의 TCP 소켓
	DWORD dwSessionID;		// 접속자의 고유 세션 ID
	CRingBuffer sendQ;		// 송신 큐
	CRingBuffer recvQ;		// 수신 큐
	DWORD dwLastRecvTime;	// 메세지 수신 체크를 위한 시간 (타임아웃용)

}ST_SESSION;

#pragma pack(pop)

LARGE_INTEGER frequency;
LARGE_INTEGER startFrame, endFrame;
LARGE_INTEGER testStartFrame, testEndFrame;
double elapsedTime;

bool G_Disconnect = false;
int G_ID = 0;
SOCKET ListenSocket;
vector<PLAYERINFO*> playerList;
SessionInfo** G_Sessions;

// SessioInfo 대체용?
unordered_map<SOCKET, st_SESSION*> g_SessionMap;

int G_X_Count = dfRANGE_MOVE_RIGHT / dfSECTION_WIDTH;
int G_Y_Count = dfRANGE_MOVE_BOTTOM / dfSECTION_HEIGHT;
bool testKeyPressed = false;

// 서버 초기화 및 세팅
void LoadData();
BOOL SettingNetWork();
void NetworkIOProcess();
void Update();

// 키보드 입력을 통해서 서버를 제어할 경우 사용
void ServerControl();
void ProcessQKeyEvent();

// 연결, 읽기, 쓰기 프로세스
BOOL AcceptProc();
void ReadProc(PLAYERINFO* player);
void WriteProc(PLAYERINFO* player);

// 연결 끊긴 클라이언트 처리
void Disconnect();

// 개인, 단체에게 메세지 전달 (직렬화 버퍼)
void SendUniCast(PLAYERINFO* player, CPacket* packet);
void SendBroadCast(PLAYERINFO* player, CPacket* packet);
void SendBroadCastSession(PLAYERINFO* player, CPacket* packet, bool toPlayer = false);
void SendBroadCastSession(PLAYERINFO* player, CPacket* packet, u_short x, u_short y);

void CreateAnotherClientSession(PLAYERINFO* player, CPacket* packet);
void CreateAnotherClientSession(PLAYERINFO* player, CPacket* packet, u_short x, u_short y);

void CreateClientSession(PLAYERINFO* player, CPacket* packet, u_short x, u_short y);
void DeleteClientSession(PLAYERINFO* player, CPacket* packet, u_short x, u_short y);

// 패킷 프로세스 (직렬화 버퍼)
bool PacketProc(PlayerInfo* player, BYTE packetType, CPacket* pPacket);

// 각 패킷 처리 프로세스 (직렬화 버퍼)
bool netPacketProc_MoveStart(PlayerInfo* player, CPacket* pPacket);
bool netPacketProc_MoveStop(PlayerInfo* player, CPacket* pPacket);
bool netPacketProc_Attack_1(PlayerInfo* player, CPacket* pPacket);
bool netPacketProc_Attack_2(PlayerInfo* player, CPacket* pPacket);
bool netPacketProc_Attack_3(PlayerInfo* player, CPacket* pPacket);
bool netPacketProc_ECHO(PlayerInfo* player, CPacket* pPacket);

// 데미지 프로세스
bool DamageProc(PlayerInfo* player, BYTE range_X, BYTE range_Y, int damage);

// 클라가 사용하게 할 패킷 선언 자동화 (직렬화 버퍼)
void mpAllocClientID(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY, BYTE byHP);
void mpCreateClient(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY, BYTE byHP);
void mpDeleteClient(CPacket* clpPacket, int iID);
void mpMoveStart(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY);
void mpMoveStop(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY);
void mpMoveAttack1(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY);
void mpMoveAttack2(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY);
void mpMoveAttack3(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY);
void mpDamage(CPacket* clpPacket, int iID, int ano_iID, BYTE byHP);
void mpECHO(CPacket* clpPacket, int time);

int main()
{
	timeBeginPeriod(1); // 타이밍 정확도를 높임

	// 성능 카운터 주파수 가져오기
	QueryPerformanceFrequency(&frequency);

	LoadData();
	if (!SettingNetWork()) return 1;

	// 프레임 시작 시간 가져오기
	QueryPerformanceCounter(&startFrame);


	while (!g_bShutDown)
	{
		//QueryPerformanceCounter(&testStartFrame);
		NetworkIOProcess();
		Update();
		ServerControl();
		//QueryPerformanceCounter(&testEndFrame);

		// 경과 시간 계산 (단위: 초)
		//double testElapsedTime = (static_cast<double>(testEndFrame.QuadPart - testStartFrame.QuadPart) / frequency.QuadPart) * 1000;
		//if(testElapsedTime > 1)
		//	printf("test : %f ms\n", testElapsedTime);
	}
	timeEndPeriod(1); // 타이밍 정확도 원래대로 복구

	// 서버 종료 대기 서버는 함부로 종료해도 안된다.
	// DB에 저장할 데이터나 기타 마무리 할 일들이 모두 끝났는지 확인 한 뒤에 꺼주어야한다.

	return 0;
}

// 설정 및 게임 데이터, DB 데이터 로딩
void LoadData()
{
	G_Sessions = new SessionInfo * [G_Y_Count];
	for (int i = 0; i < G_Y_Count; i++) {
		G_Sessions[i] = new SessionInfo[G_X_Count];
	}

	// 각 섹션의 좌표 설정
	for (int i = 0; i < G_Y_Count; i++) {
		for (int j = 0; j < G_X_Count; j++) {
			G_Sessions[i][j].X = j;
			G_Sessions[i][j].Y = i;
		}
	}
	// 접근법
	//int value = G_Sections[y / dfSECTION_HEIGHT][x / dfSECTION_WIDTH];
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
		Disconnect();
		G_Disconnect = false;
	}
}

void Update()
{
	// 프레임 종료 시간 가져오기
	QueryPerformanceCounter(&endFrame);
	// 경과 시간 계산 (단위: 초)
	elapsedTime = static_cast<double>(endFrame.QuadPart - startFrame.QuadPart) / frequency.QuadPart;

	// 50 FPS 유지하기 위해 대기 시간 계산
	double targetTime = 1.0 / dfUPDATE_FPS; // FPS에 해당하는 시간
	if (elapsedTime < targetTime)
		return;

	QueryPerformanceCounter(&startFrame);

	vector<PLAYERINFO*>::iterator playerIter;
	for (playerIter = playerList.begin(); playerIter != playerList.end(); ++playerIter)
	{
		// 나중에 성능을 위해 몇초에 한번만 확인하게 로직 변경 필요할 수도 있음
		if (abs((long)(GetTickCount() - (*playerIter)->echoTime)) > dfNETWORK_PACKET_RECV_TIMEOUT)
		{
			printf("# PACKET_ECHO_ERROR # SessionID : [%d] / 시간 오류 : [%d]\n", (*playerIter)->ID, GetTickCount() - (*playerIter)->echoTime);
			(*playerIter)->isConnect = false;
			G_Disconnect = true;
			continue;
		}

		if (!(*playerIter)->isMove) continue;

		switch ((*playerIter)->Direction)
		{
		case dfPACKET_MOVE_DIR_LL:
			if ((*playerIter)->X <= dfRANGE_MOVE_LEFT) break;
			(*playerIter)->X -= dfSPEED_PLAYER_X;
			//printf("# gameRun : LL #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_LU:
			if ((*playerIter)->X <= dfRANGE_MOVE_LEFT) break;
			if ((*playerIter)->Y <= dfRANGE_MOVE_TOP) break;
			(*playerIter)->X -= dfSPEED_PLAYER_X;
			(*playerIter)->Y -= dfSPEED_PLAYER_Y;
			//printf("# gameRun : LU #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_UU:
			if ((*playerIter)->Y <= dfRANGE_MOVE_TOP) break;
			(*playerIter)->Y -= dfSPEED_PLAYER_Y;
			//printf("# gameRun : UU #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_RU:
			if ((*playerIter)->X >= dfRANGE_MOVE_RIGHT) break;
			if ((*playerIter)->Y <= dfRANGE_MOVE_TOP) break;
			(*playerIter)->X += dfSPEED_PLAYER_X;
			(*playerIter)->Y -= dfSPEED_PLAYER_Y;
			//printf("# gameRun : RU #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_RR:
			if ((*playerIter)->X >= dfRANGE_MOVE_RIGHT) break;
			(*playerIter)->X += dfSPEED_PLAYER_X;
			//printf("# gameRun : RR #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_RD:
			if ((*playerIter)->X >= dfRANGE_MOVE_RIGHT) break;
			if ((*playerIter)->Y >= dfRANGE_MOVE_BOTTOM) break;
			(*playerIter)->X += dfSPEED_PLAYER_X;
			(*playerIter)->Y += dfSPEED_PLAYER_Y;
			//printf("# gameRun : RD #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_DD:
			if ((*playerIter)->Y >= dfRANGE_MOVE_BOTTOM) break;
			(*playerIter)->Y += dfSPEED_PLAYER_Y;
			//printf("# gameRun : DD #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		case dfPACKET_MOVE_DIR_LD:
			if ((*playerIter)->X <= dfRANGE_MOVE_LEFT) break;
			if ((*playerIter)->Y >= dfRANGE_MOVE_BOTTOM) break;
			(*playerIter)->X -= dfSPEED_PLAYER_X;
			(*playerIter)->Y += dfSPEED_PLAYER_Y;
			//printf("# gameRun : LD #  SessionID: %d / X : %d / Y : %d\n", (*playerIter)->ID, (*playerIter)->X, (*playerIter)->Y);
			break;
		}

		bool changeSession = false;
		if ((*playerIter)->session_x != (*playerIter)->X / dfSECTION_WIDTH)
		{
			G_Sessions[(*playerIter)->session_y][(*playerIter)->session_x].playerList.remove((*playerIter));
			(*playerIter)->session_x = (*playerIter)->X / dfSECTION_WIDTH;
			changeSession = true;
		}
		if ((*playerIter)->session_y != (*playerIter)->Y / dfSECTION_HEIGHT)
		{
			G_Sessions[(*playerIter)->session_y][(*playerIter)->session_x].playerList.remove((*playerIter));
			(*playerIter)->session_y = (*playerIter)->Y / dfSECTION_HEIGHT;
			changeSession = true;
		}

		if (changeSession)
		{
			//if ((*playerIter)->isChangeSession) continue;			
			//(*playerIter)->isChangeSession = true;
			// 2번 들어오는거만 고치면 됨
			//printf("2번 들오는가? s_y[%d] s_x[%d] \n", (*playerIter)->session_y, (*playerIter)->session_x);
			G_Sessions[(*playerIter)->session_y][(*playerIter)->session_x].playerList.push_back((*playerIter));
			
			// 보내는거고
			switch ((*playerIter)->Direction)
			{
			case dfPACKET_MOVE_DIR_LL:
			{
				CPacket packet;
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y - 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y + 1);
				packet.Clear();

				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x + 2, (*playerIter)->session_y);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x + 2, (*playerIter)->session_y - 1);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x + 2, (*playerIter)->session_y + 1);
				packet.Clear();
				break;
			}
			case dfPACKET_MOVE_DIR_LU:
			{
				CPacket packet;
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y + 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y - 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x, (*playerIter)->session_y - 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y - 1);
				packet.Clear();

				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x + 2, (*playerIter)->session_y);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x + 2, (*playerIter)->session_y + 1);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x + 2, (*playerIter)->session_y + 2);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y + 2);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x, (*playerIter)->session_y + 2);
				packet.Clear();
				break;
			}
			case dfPACKET_MOVE_DIR_UU:
			{
				CPacket packet;
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x, (*playerIter)->session_y - 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y - 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y - 1);
				packet.Clear();

				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y + 2);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y + 2);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x, (*playerIter)->session_y + 2);
				packet.Clear();
				break;
			}
			case dfPACKET_MOVE_DIR_RU:
			{
				CPacket packet;
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y + 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y - 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x, (*playerIter)->session_y + 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y + 1);
				packet.Clear();

				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x - 2, (*playerIter)->session_y);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x - 2, (*playerIter)->session_y + 1);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x - 2, (*playerIter)->session_y + 2);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y + 2);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x, (*playerIter)->session_y + 2);
				packet.Clear();
				break; 
			}
			case dfPACKET_MOVE_DIR_RR:
			{
				CPacket packet;
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y - 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y + 1);
				packet.Clear();

				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x - 2, (*playerIter)->session_y);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x - 2, (*playerIter)->session_y - 1);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x - 2, (*playerIter)->session_y + 1);
				packet.Clear();
				break; 
			}
			case dfPACKET_MOVE_DIR_RD:
			{
				CPacket packet;
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y - 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y + 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x, (*playerIter)->session_y + 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y + 1);
				packet.Clear();

				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x - 2, (*playerIter)->session_y);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x - 2, (*playerIter)->session_y - 1);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x - 2, (*playerIter)->session_y - 2);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y - 2);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x, (*playerIter)->session_y - 2);
				packet.Clear();
				break; 
			}
			case dfPACKET_MOVE_DIR_DD:
			{
				CPacket packet;
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x, (*playerIter)->session_y + 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y + 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y + 1);
				packet.Clear();

				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y - 2);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y - 2);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x, (*playerIter)->session_y - 2);
				packet.Clear();
				break; 
			}
			case dfPACKET_MOVE_DIR_LD:
			{
				CPacket packet;
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y - 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x - 1, (*playerIter)->session_y + 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x, (*playerIter)->session_y + 1);
				CreateClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y + 1);
				packet.Clear();

				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x + 2, (*playerIter)->session_y);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x + 2, (*playerIter)->session_y - 1);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x + 2, (*playerIter)->session_y - 2);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x + 1, (*playerIter)->session_y - 2);
				DeleteClientSession((*playerIter), &packet, (*playerIter)->session_x, (*playerIter)->session_y - 2);
				packet.Clear();
				break;
			}
			}
		}
	}
}

void ServerControl()
{
	// 키 입력 이벤트 처리
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		if (!testKeyPressed)
		{
			// 'Q' 키가 눌렸을 때
			ProcessQKeyEvent();
			testKeyPressed = true;
		}
	}
	else
		testKeyPressed = false;
}

void ProcessQKeyEvent()
{
	// 'Q' 키 관련 작업 처리
	for (int i = 0; i < G_Y_Count; i++)
	{
		for (int j = 0; j < G_X_Count; j++)
		{
			if (G_Sessions[i][j].playerList.size() > 0)
			{
				printf("[%d][%d] 섹션에 있는 사람 : ", i, j);
				list<PlayerInfo*>::iterator iter;
				for (iter = G_Sessions[i][j].playerList.begin(); iter != G_Sessions[i][j].playerList.end(); ++iter)
				{
					printf("[%d] ", (*iter)->ID);
				}
				printf("\n");
			}
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
	//newClient->X = rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) + dfRANGE_MOVE_LEFT;
	//newClient->Y = rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) + dfRANGE_MOVE_TOP;
	newClient->X = rand() % 400 + 201;
	newClient->Y = rand() % 400 + 201;

	newClient->dwAction = newClient->Direction = dfPACKET_MOVE_DIR_LL;

	newClient->sendQ = CRingBuffer(4096);
	newClient->recvQ = CRingBuffer(4096);

	newClient->isMove = false;
	newClient->isConnect = true;

	//newClient->isChangeSession = false;
	newClient->session_x = newClient->X / dfSECTION_WIDTH;
	newClient->session_y = newClient->Y / dfSECTION_HEIGHT;
	newClient->echoTime = GetTickCount();

	G_Sessions[newClient->session_y][newClient->session_x].playerList.push_back(newClient);
	printf("# PACKET_CONNECT # SessionID : %d\n", newClient->ID);

	// 클라에게 생성 메세지
	CPacket packet;
	mpAllocClientID(&packet, newClient->ID, newClient->Direction, newClient->X, newClient->Y, newClient->HP);
	SendUniCast(newClient, &packet);
	packet.Clear();

	playerList.push_back(newClient);

	// 다른 클라들에게 새로운 클라 생성 메세지
	mpCreateClient(&packet, newClient->ID, newClient->dwAction, newClient->X, newClient->Y, newClient->HP);
	//SendBroadCast(newClient, &packet);
	SendBroadCastSession(newClient, &packet);
	packet.Clear();

	printf("Create Character # SessionID : %d\tX : %d\t\tY : %d\n", newClient->ID, newClient->X, newClient->Y);
	
	// 클라에게 이전에 있던 유저들 생성 메세지
	CreateAnotherClientSession(newClient, &packet);
	return true;
}

// 각 플레이어 recv 링버퍼에 있는 데이터 읽기
void ReadProc(PLAYERINFO* player)
{
	// recv하고 끊어진 경우, 진짜 오류인 경우 걸러내기
	int recvNum = recv(player->Socket, player->recvQ.GetRearBufferPtr(), player->recvQ.GetFreeSize(), 0);
	if (recvNum == SOCKET_ERROR || recvNum == 0)
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			if (recvNum == 0)
			{
				player->isConnect = false;
				G_Disconnect = true;
				return;
			}
			// 10054 오류 : 클라쪽 피어별 연결 다시 설정?
			if (GetLastError() == WSAECONNRESET)
			{
				player->isConnect = false;
				G_Disconnect = true;
			}
			printf("recv [%d]\n", GetLastError());
			return;
		}
	}
	player->recvQ.MoveRear(recvNum);

	while (player->recvQ.GetUseSize() > 0)
	{
		//ST_HEADER* recvHeader = (ST_HEADER*)player->recvQ.GetFrontBufferPtr();

		CPacket headerPacket;
		headerPacket.PutData(player->recvQ.GetFrontBufferPtr(), 3);

		BYTE byCode;
		BYTE bySize;
		BYTE byType;

		headerPacket >> byCode >> bySize >> byType;

		//if (recvHeader->byCode != dfNETWORK_PACKET_CODE)
		if (byCode != dfNETWORK_PACKET_CODE)
		{
			//printf("dfNETWORK_PACKET_CODE(0x89) 아님 [%02X]\n", recvHeader->byCode);
			printf("dfNETWORK_PACKET_CODE(0x89) 아님 [%02X]\n", byCode);
			player->isConnect = false;
			G_Disconnect = true;
			return;
		}
		//int packetSize = recvHeader->bySize;
		int packetSize = bySize;
		player->recvQ.MoveFront(sizeof(ST_HEADER));

		CPacket testPacket;
		testPacket.PutData(player->recvQ.GetFrontBufferPtr(), packetSize);
		//PacketProc(player, recvHeader->byType, &testPacket);
		PacketProc(player, byType, &testPacket);
		player->recvQ.MoveFront(packetSize);
	}
	//player->isChangeSession = false;
}

// 각 플레이어 send 링버퍼에 있는 데이터 보내기
void WriteProc(PLAYERINFO* player)
{
	if (player->sendQ.GetUseSize() <= 0) return;

	int useSize = player->sendQ.GetUseSize();
	int peekSize = player->sendQ.Peek(player->sendQ.GetFrontBufferPtr(), useSize);
	if (peekSize != useSize)
	{
		printf("보내기 사이즈 안맞음\n");
		return;
	}

	int sendNum = send(player->Socket, player->sendQ.GetFrontBufferPtr(), peekSize, 0);
	if (sendNum == SOCKET_ERROR)
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			printf("send [%d]\n", GetLastError());
			return;
		}
	}
	player->sendQ.MoveFront(peekSize);
}

// 해당 유저에게만 보내기
void SendUniCast(PLAYERINFO* player, CPacket* packet)
{
	player->sendQ.Enqueue(packet->GetBufferPtr(), packet->GetDataSize());
}

// 해당 유저를 제외한 모두에게 보내기
void SendBroadCast(PLAYERINFO* player, CPacket* packet)
{
	vector<PLAYERINFO*>::iterator sendIter;
	for (sendIter = playerList.begin(); sendIter != playerList.end(); ++sendIter)
	{
		if ((*sendIter) == player) continue;
		if ((*sendIter)->Socket != INVALID_SOCKET)
			SendUniCast((*sendIter), packet);
	}
}

void SendBroadCastSession(PLAYERINFO* player, CPacket* packet, bool toPlayer)
{
	if(toPlayer)
		SendUniCast(player, packet);

	SendBroadCastSession(player, packet, player->session_x, player->session_y);
	SendBroadCastSession(player, packet, player->session_x, player->session_y + 1);
	SendBroadCastSession(player, packet, player->session_x, player->session_y - 1);

	SendBroadCastSession(player, packet, player->session_x - 1, player->session_y);
	SendBroadCastSession(player, packet, player->session_x - 1, player->session_y + 1);
	SendBroadCastSession(player, packet, player->session_x - 1, player->session_y - 1);

	SendBroadCastSession(player, packet, player->session_x + 1, player->session_y);
	SendBroadCastSession(player, packet, player->session_x + 1, player->session_y + 1);
	SendBroadCastSession(player, packet, player->session_x + 1, player->session_y - 1);
}

// 해당 섹션에 있는 유저들한테 브로드캐스팅
void SendBroadCastSession(PLAYERINFO* player, CPacket* packet, u_short x, u_short y)
{
	if (x < 0) return;
	if (y < 0) return;
	if (x > dfRANGE_MOVE_RIGHT) return;
	if (y > dfRANGE_MOVE_BOTTOM) return;

	list<PLAYERINFO*>::iterator sendIter;
	for (sendIter = G_Sessions[y][x].playerList.begin(); sendIter != G_Sessions[y][x].playerList.end(); ++sendIter)
	{
		if ((*sendIter) == player) continue;
		if ((*sendIter)->Socket != INVALID_SOCKET)
			SendUniCast((*sendIter), packet);
	}
}

void CreateAnotherClientSession(PLAYERINFO* player, CPacket* packet)
{
	list<PLAYERINFO*>::iterator iter;
	for (iter = G_Sessions[player->session_y][player->session_x].playerList.begin(); iter != G_Sessions[player->session_y][player->session_x].playerList.end(); ++iter)
	{
		// 해당 섹션에 있는 유저 불러오기
		CPacket packet;
		// 플레이어면 안넣게 하기 위해
		if ((*iter)->ID == player->ID) continue;

		mpCreateClient(&packet, (*iter)->ID, (*iter)->dwAction, (*iter)->X, (*iter)->Y, (*iter)->HP);
		SendUniCast(player, &packet);
		packet.Clear();

		// 만일 해당 클라가 움직이고 있었다면
		if ((*iter)->isMove)
		{
			mpMoveStart(&packet, (*iter)->ID, (*iter)->Direction, (*iter)->X, (*iter)->Y);
			SendUniCast(player, &packet);
			packet.Clear();
		}
	}

	CreateAnotherClientSession(player, packet, player->session_x, player->session_y + 1);
	CreateAnotherClientSession(player, packet, player->session_x, player->session_y - 1);

	CreateAnotherClientSession(player, packet, player->session_x - 1, player->session_y);
	CreateAnotherClientSession(player, packet, player->session_x - 1, player->session_y + 1);
	CreateAnotherClientSession(player, packet, player->session_x - 1, player->session_y - 1);

	CreateAnotherClientSession(player, packet, player->session_x + 1, player->session_y);
	CreateAnotherClientSession(player, packet, player->session_x + 1, player->session_y + 1);
	CreateAnotherClientSession(player, packet, player->session_x + 1, player->session_y - 1);
}

void CreateAnotherClientSession(PLAYERINFO* player, CPacket* packet, u_short x, u_short y)
{
	if (x < 0) return;
	if (y < 0) return;
	if (x > dfRANGE_MOVE_RIGHT) return;
	if (y > dfRANGE_MOVE_BOTTOM) return;

	list<PLAYERINFO*>::iterator iter;
	for (iter = G_Sessions[y][x].playerList.begin(); iter != G_Sessions[y][x].playerList.end(); ++iter)
	{
		// 해당 섹션에 있는 유저 불러오기
		CPacket packet;
		mpCreateClient(&packet, (*iter)->ID, (*iter)->dwAction, (*iter)->X, (*iter)->Y, (*iter)->HP);
		SendUniCast(player, &packet);
		packet.Clear();

		// 만일 해당 클라가 움직이고 있었다면
		if ((*iter)->isMove)
		{
			mpMoveStart(&packet, (*iter)->ID, (*iter)->Direction, (*iter)->X, (*iter)->Y);
			SendUniCast(player, &packet);
			packet.Clear();
		}
	}
}

void CreateClientSession(PLAYERINFO* player, CPacket* packet, u_short x, u_short y)
{
	if (x < 0) return;
	if (y < 0) return;
	if (x > dfRANGE_MOVE_RIGHT) return;
	if (y > dfRANGE_MOVE_BOTTOM) return;

	list<PLAYERINFO*>::iterator iter;
	for (iter = G_Sessions[y][x].playerList.begin(); iter != G_Sessions[y][x].playerList.end(); ++iter)
	{
		// 해당 섹션에 있는 유저 불러오기
		CPacket packet;
		mpCreateClient(&packet, (*iter)->ID, (*iter)->dwAction, (*iter)->X, (*iter)->Y, (*iter)->HP);
		SendUniCast(player, &packet);
		packet.Clear();

		// 만일 해당 클라가 움직이고 있었다면
		if ((*iter)->isMove)
		{
			mpMoveStart(&packet, (*iter)->ID, (*iter)->Direction, (*iter)->X, (*iter)->Y);
			SendUniCast(player, &packet);
			packet.Clear();
		}

		// 해당 클라에게도 내가 해당 섹션에 들어왔다는 표식
		mpCreateClient(&packet, player->ID, player->dwAction, player->X, player->Y, player->HP);
		SendUniCast((*iter), &packet);
		packet.Clear();
		
		// 만일 내가 움직이고 있었다면
		if (player->isMove)
		{
			mpMoveStart(&packet, player->ID, player->Direction, player->X, player->Y);
			SendUniCast((*iter), &packet);
			packet.Clear();
		}
	}
}

void DeleteClientSession(PLAYERINFO* player, CPacket* packet, u_short x, u_short y)
{
	if (x < 0) return;
	if (y < 0) return;
	if (x > dfRANGE_MOVE_RIGHT) return;
	if (y > dfRANGE_MOVE_BOTTOM) return;

	list<PLAYERINFO*>::iterator iter;
	for (iter = G_Sessions[y][x].playerList.begin(); iter != G_Sessions[y][x].playerList.end(); ++iter)
	{
		// 해당 섹션에 있는 유저 삭제
		CPacket packet;
		mpDeleteClient(&packet, (*iter)->ID);
		SendUniCast(player, &packet);
		packet.Clear();

		// 해당 클라에게도 내가 해당 섹션에 나갔다 표식
		mpDeleteClient(&packet, player->ID);
		SendUniCast((*iter), &packet);
		packet.Clear();
	}
}

// 유저가 끊겼을 경우
void Disconnect()
{
	vector<PLAYERINFO*>::iterator deleteIter = playerList.begin();
	while (deleteIter != playerList.end())
	{
		if ((*deleteIter)->isConnect) { deleteIter++; continue; }

		G_Sessions[(*deleteIter)->session_y][(*deleteIter)->session_x].playerList.remove(*deleteIter);

		CPacket packet;
		mpDeleteClient(&packet, (*deleteIter)->ID);
		//SendBroadCast((*deleteIter), &packet);
		SendBroadCastSession((*deleteIter), &packet);

		printf("Disconnect # Session ID : %d [%d]\n", (*deleteIter)->ID, GetLastError());

		closesocket((*deleteIter)->Socket);
		delete[](*deleteIter);
		deleteIter = playerList.erase(deleteIter);
	}
}

// 패킷 프로세스
bool PacketProc(PlayerInfo* player, BYTE packetType, CPacket* pPacket)
{
	switch (packetType)
	{
	case dfPACKET_CS_MOVE_START:
		netPacketProc_MoveStart(player, pPacket);
		//printf("# PACKET_MOVESTART # SessionID : [%d] / Direction : [%X] / X : [%d] / Y : [%d]\n", player->ID, player->Direction, player->X, player->Y);
		break;
	case dfPACKET_CS_MOVE_STOP:
		netPacketProc_MoveStop(player, pPacket);
		//printf("# PACKET_MOVESTOP # SessionID : [%d] / Direction : [%X] / X : [%d] / Y : [%d]\n", player->ID, player->Direction, player->X, player->Y);
		break;
	case dfPACKET_CS_ATTACK1:
		netPacketProc_Attack_1(player, pPacket);
		//printf("# PACKET_ATTACK1 # SessionID : [%d] / Direction : [%X] / X : [%d] / Y : [%d]\n", player->ID, player->Direction, player->X, player->Y);
		break;
	case dfPACKET_CS_ATTACK2:
		netPacketProc_Attack_2(player, pPacket);
		//printf("# PACKET_ATTACK2 # SessionID : [%d] / Direction : [%X] / X : [%d] / Y : [%d]\n", player->ID, player->Direction, player->X, player->Y);
		break;
	case dfPACKET_CS_ATTACK3:
		netPacketProc_Attack_3(player, pPacket);
		//printf("# PACKET_ATTACK3 # SessionID : [%d] / Direction : [%X] / X : [%d] / Y : [%d]\n", player->ID, player->Direction, player->X, player->Y);
		break;
	case dfPACKET_CS_ECHO:
		netPacketProc_ECHO(player, pPacket);
		//printf("# PACKET_ECHO # SessionID : [%d] / Time : [%d] / ECHO : [%d]\n", player->ID, GetTickCount(), player->echoTime);
		break;
	}
	return TRUE;
}

bool netPacketProc_MoveStart(PlayerInfo* player, CPacket* pPacket)
{
	short shX, shY;
	BYTE byDirection;
	*pPacket >> byDirection >> shX >> shY;

	// 데드 레커닝 문제가 일어나면 아래 코드에서 터짐

	//메세지 수신 로그 확인
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 끊어버림
	if (abs(player->X - shX) > dfERROR_RANGE || abs(player->Y - shY) > dfERROR_RANGE)
	{
		printf("MoveStart 아디 [%d] 서버 [%d] [%d] 클라 [%d] [%d]\n", player->ID, player->X, player->Y, shX, shY);
		player->isConnect = false;
		G_Disconnect = true;
		return FALSE;
	}

	// 방향을 변경
	switch (byDirection)
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
	player->Direction = byDirection;

	// X,Y 좌표 갱신
	player->X = shX;
	player->Y = shY;

	player->isMove = true;

	CPacket packet;
	mpMoveStart(&packet, player->ID, player->Direction, player->X, player->Y);
	//SendBroadCast(player, &packet);
	SendBroadCastSession(player, &packet);
	return TRUE;
}

bool netPacketProc_MoveStop(PlayerInfo* player, CPacket* pPacket)
{
	short shX, shY;
	BYTE byDirection;
	*pPacket >> byDirection >> shX >> shY;

	//메세지 수신 로그 확인
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 끊어버림
	if (abs(player->X - shX) > dfERROR_RANGE || abs(player->Y - shY) > dfERROR_RANGE)
	{
		printf("MoveStop 아디 [%d] 서버 [%d] [%d] 클라 [%d] [%d]\n", player->ID, player->X, player->Y, shX, shY);
		player->isConnect = false;
		G_Disconnect = true;
		return FALSE;
	}

	player->Direction = byDirection;
	player->X = shX;
	player->Y = shY;

	player->isMove = false;

	CPacket packet;
	mpMoveStop(&packet, player->ID, player->Direction, player->X, player->Y);
	//SendBroadCast(player, &packet);
	SendBroadCastSession(player, &packet);
	return TRUE;
}

bool netPacketProc_Attack_1(PlayerInfo* player, CPacket* pPacket)
{
	short shX, shY;
	BYTE byDirection;

	*pPacket >> byDirection >> shX >> shY;

	CPacket packet;
	mpMoveAttack1(&packet, player->ID, byDirection, shX, shY);
	//SendBroadCast(player, &packet);
	SendBroadCastSession(player, &packet);

	DamageProc(player, dfATTACK3_RANGE_X, dfATTACK3_RANGE_Y, 30);
	return TRUE;
}

bool netPacketProc_Attack_2(PlayerInfo* player, CPacket* pPacket)
{
	short shX, shY;
	BYTE byDirection;

	*pPacket >> byDirection >> shX >> shY;

	CPacket packet;
	mpMoveAttack2(&packet, player->ID, byDirection, shX, shY);
	//SendBroadCast(player, &packet);
	SendBroadCastSession(player, &packet);

	DamageProc(player, dfATTACK3_RANGE_X, dfATTACK3_RANGE_Y, 30);
	return TRUE;
}

bool netPacketProc_Attack_3(PlayerInfo* player, CPacket* pPacket)
{
	short shX, shY;
	BYTE byDirection;

	*pPacket >> byDirection >> shX >> shY;

	CPacket packet;
	mpMoveAttack3(&packet, player->ID, byDirection, shX, shY);
	//SendBroadCast(player, &packet);
	SendBroadCastSession(player, &packet);

	DamageProc(player, dfATTACK3_RANGE_X, dfATTACK3_RANGE_Y, 30);
	return TRUE;
}

bool netPacketProc_ECHO(PlayerInfo* player, CPacket* pPacket)
{
	int time;
	*pPacket >> time;


	player->echoTime = time;

	CPacket packet;
	mpECHO(&packet, time);
	SendUniCast(player, &packet);
	return TRUE;
}

bool DamageProc(PlayerInfo* player, BYTE range_X, BYTE range_Y, int damage)
{
	vector<PLAYERINFO*>::iterator anotherClientIter;
	for (anotherClientIter = playerList.begin(); anotherClientIter != playerList.end(); ++anotherClientIter)
	{
		// 내가 나를 때릴순 없으니 넘김
		if ((*anotherClientIter) == player) continue;
		// 공격 범위 안에 있으면
		if (abs(player->X - (*anotherClientIter)->X) < range_X && abs(player->Y - (*anotherClientIter)->Y) < range_Y)
		{
			// 포지션이 오른쪽을 보면서 적이 오른쪽에 있을때거나 포지션이 왼쪽을 보면서 적이 왼쪽에 있을때
			if ((player->X <= (*anotherClientIter)->X && player->Direction == dfPACKET_MOVE_DIR_RR) ||
				(player->X >= (*anotherClientIter)->X && player->Direction == dfPACKET_MOVE_DIR_LL))
			{
				(*anotherClientIter)->HP -= damage;
				CPacket packet;
				mpDamage(&packet, player->ID, (*anotherClientIter)->ID, (*anotherClientIter)->HP);
				if ((*anotherClientIter)->HP <= 0 || (*anotherClientIter)->HP > 100)
				{
					(*anotherClientIter)->isConnect = false;
					G_Disconnect = true;
				}
				else
				{
					//SendBroadCast(NULL, &packet);
					SendBroadCastSession(player, &packet, true);
				}
				break;
			}
		}
	}
	anotherClientIter = playerList.end();
	return TRUE;
}

// 직렬화 버퍼를 사용한 매크로
void mpAllocClientID(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY, BYTE byHP)
{
	ST_HEADER header;
	header.byCode = dfNETWORK_PACKET_CODE;
	header.bySize = sizeof(ALLOC_CLIENT_ID);
	header.byType = dfPACKET_SC_CREATE_MY_CHARACTER;

	clpPacket->PutData((char*)&header, dfNETWORK_PACKET_HEADER_SIZE);

	*clpPacket << iID;
	*clpPacket << byDirection;
	*clpPacket << shX;
	*clpPacket << shY;
	*clpPacket << byHP;
}

void mpCreateClient(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY, BYTE byHP)
{
	ST_HEADER header;
	header.byCode = dfNETWORK_PACKET_CODE;
	header.bySize = sizeof(CREATE_CLIENT);
	header.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;

	clpPacket->PutData((char*)&header, dfNETWORK_PACKET_HEADER_SIZE);

	*clpPacket << iID;
	*clpPacket << byDirection;
	*clpPacket << shX;
	*clpPacket << shY;
	*clpPacket << byHP;
}

void mpDeleteClient(CPacket* clpPacket, int iID)
{
	ST_HEADER header;
	header.byCode = dfNETWORK_PACKET_CODE;
	header.bySize = sizeof(DELETE_CLIENT);
	header.byType = dfPACKET_SC_DELETE_CHARACTER;

	clpPacket->PutData((char*)&header, dfNETWORK_PACKET_HEADER_SIZE);

	*clpPacket << iID;
}

void mpMoveStart(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY)
{
	ST_HEADER header;
	header.byCode = dfNETWORK_PACKET_CODE;
	header.bySize = sizeof(ST_SC_MOVE);
	header.byType = dfPACKET_SC_MOVE_START;

	clpPacket->PutData((char*)&header, dfNETWORK_PACKET_HEADER_SIZE);

	*clpPacket << iID;
	*clpPacket << byDirection;
	*clpPacket << shX;
	*clpPacket << shY;
}

void mpMoveStop(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY)
{
	ST_HEADER header;
	header.byCode = dfNETWORK_PACKET_CODE;
	header.bySize = sizeof(ST_SC_MOVE);
	header.byType = dfPACKET_SC_MOVE_STOP;

	clpPacket->PutData((char*)&header, dfNETWORK_PACKET_HEADER_SIZE);

	*clpPacket << iID;
	*clpPacket << byDirection;
	*clpPacket << shX;
	*clpPacket << shY;
}

void mpMoveAttack1(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY)
{
	ST_HEADER header;
	header.byCode = dfNETWORK_PACKET_CODE;
	header.bySize = sizeof(SC_ATTACK);
	header.byType = dfPACKET_SC_ATTACK1;

	clpPacket->PutData((char*)&header, dfNETWORK_PACKET_HEADER_SIZE);

	*clpPacket << iID;
	*clpPacket << byDirection;
	*clpPacket << shX;
	*clpPacket << shY;
}

void mpMoveAttack2(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY)
{
	ST_HEADER header;
	header.byCode = dfNETWORK_PACKET_CODE;
	header.bySize = sizeof(SC_ATTACK);
	header.byType = dfPACKET_SC_ATTACK2;

	clpPacket->PutData((char*)&header, dfNETWORK_PACKET_HEADER_SIZE);

	*clpPacket << iID;
	*clpPacket << byDirection;
	*clpPacket << shX;
	*clpPacket << shY;
}

void mpMoveAttack3(CPacket* clpPacket, int iID, BYTE byDirection, short shX, short shY)
{
	ST_HEADER header;
	header.byCode = dfNETWORK_PACKET_CODE;
	header.bySize = sizeof(SC_ATTACK);
	header.byType = dfPACKET_SC_ATTACK3;

	clpPacket->PutData((char*)&header, dfNETWORK_PACKET_HEADER_SIZE);

	*clpPacket << iID;
	*clpPacket << byDirection;
	*clpPacket << shX;
	*clpPacket << shY;
}

void mpDamage(CPacket* clpPacket, int iID, int ano_iID, BYTE byHP)
{
	ST_HEADER header;
	header.byCode = dfNETWORK_PACKET_CODE;
	header.bySize = sizeof(SC_DAMAGE);
	header.byType = dfPACKET_SC_DAMAGE;

	clpPacket->PutData((char*)&header, dfNETWORK_PACKET_HEADER_SIZE);

	*clpPacket << iID;
	*clpPacket << ano_iID;
	*clpPacket << byHP;
}

void mpECHO(CPacket* clpPacket, int time)
{
	ST_HEADER header;
	header.byCode = dfNETWORK_PACKET_CODE;
	header.bySize = sizeof(ST_ECHO);
	header.byType = dfPACKET_SC_ECHO;

	clpPacket->PutData((char*)&header, dfNETWORK_PACKET_HEADER_SIZE);

	*clpPacket << time;
}
