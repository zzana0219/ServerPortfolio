#pragma comment (lib, "ws2_32")

#include "framework.h"
#include "MouseDraw.h"
#include"RingBuffer.h"
#include<WS2tcpip.h>
#include<windowsx.h>
#include <time.h>

#define MAX_LOADSTRING 100
#define UM_NETWORK (WM_USER + 1)

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// 헤더
struct st_HEADER
{
    unsigned short Len;
};

// 패킷
struct st_DRAW_PACKET
{
    int iStartX;
    int iStartY;
    int iEndX;
    int iEndY;
};

int oldX = 0;
int oldY = 0;
int x = 0;
int y = 0;

bool g_bConnect = false;
bool b_Send_Flag = false;
bool bMouseButtonClicked = false;

HWND g_hWnd;
SOCKET sock;
CRingBuffer sendQ;
CRingBuffer recvQ;
void Send(st_HEADER header, st_DRAW_PACKET packet);
void ReadEvent();
void WriteEvent();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    srand(time(NULL));
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MOUSEDRAW, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    g_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    if (!g_hWnd)
        return FALSE;
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MOUSEDRAW));

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    SOCKADDR_IN serverAddr;
    memset(&serverAddr, ' ', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(25000);
    InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        return 1;

    int asyncSelectNum = WSAAsyncSelect(sock, g_hWnd, UM_NETWORK, FD_CONNECT | FD_CLOSE | FD_WRITE | FD_READ);
    if (asyncSelectNum == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            int t = WSAGetLastError();
            MessageBox(g_hWnd, L"??", L"WSAAsyncSelect", 0);
            return 1;
        }
    }

    int connectNum = connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (connectNum == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            int t = WSAGetLastError();
            MessageBox(g_hWnd, L"connect", L"??", 0);
            printf("connect [%d]\n", WSAGetLastError());
            return 1;
        }
    }
    MSG msg;
    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;

    while(1)
    {
        int _s_test_x = 0;
        int _s_test_y = 0;
        int _e_test_x = rand() % 200;
        int _e_test_y = rand() % 200;
        st_HEADER header;
        st_DRAW_PACKET packet;
        header.Len = 16;
        packet.iStartX = _s_test_x;
        packet.iStartY = _s_test_y;
        packet.iEndX = _e_test_x;
        packet.iEndY = _e_test_y;
        Send(header, packet);
    }
}

//  함수: MyRegisterClass()
//  용도: 창 클래스를 등록합니다.
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MOUSEDRAW));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MOUSEDRAW);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//   함수: InitInstance(HINSTANCE, int)
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//   주석:
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//  용도: 주 창의 메시지를 처리합니다.
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_LBUTTONDOWN:
        bMouseButtonClicked = true;
        break;
    case WM_MOUSEMOVE:
        break;
    case WM_LBUTTONUP:
        bMouseButtonClicked = false;
        break;
    case UM_NETWORK:
        if (WSAGETSELECTERROR(lParam))
        {
            printf("??");
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_CONNECT:
            printf("??");
            break;
        case FD_CLOSE:
            break;
        case FD_READ:
            ReadEvent();
            break;
        case FD_WRITE:
            b_Send_Flag = true;
            WriteEvent();
            break;
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void Send(st_HEADER header, st_DRAW_PACKET packet)
{
    sendQ.Enqueue((char*)&header, sizeof(header));
    sendQ.Enqueue((char*)&packet, sizeof(packet));
    WriteEvent();
}

void WriteEvent()
{
    if (!b_Send_Flag) return;
    while (1)
    {
        char buffer[1000]{ 0 };
        int sendPRet = sendQ.Peek(buffer, min(1000, sendQ.GetUseSize()));
        if (sendPRet == 0) break;
        int send_ret = send(sock, buffer, sendPRet, 0);
        if (send_ret == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                PostMessage(g_hWnd, WM_DESTROY, 0, 0);
                break;
            }
            b_Send_Flag = false;
            break;
        }
        sendQ.MoveFront(send_ret);
    }
}

void ReadEvent()
{
    char buffer[1000];
    int recv_ret = recv(sock, buffer, 1000, 0);
    if (recv_ret == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            PostMessage(g_hWnd, WM_DESTROY, 0, 0);
            return;
        }
    }
    int eq_ret = recvQ.Enqueue(buffer, recv_ret);
    if (eq_ret == 0)
    {
        // recvRingBuffer 가득 참
        return;
    }
    HDC hdc = GetDC(g_hWnd);
    while (1)
    {
        st_HEADER header;
        if (recvQ.GetFreeSize() < sizeof(st_DRAW_PACKET)) break;
        int recvPeekRet = recvQ.Peek((char*)&header, sizeof(st_HEADER));
        if (recvPeekRet == 0) break;
        recvQ.MoveFront(sizeof(st_HEADER));

        st_DRAW_PACKET packet;
        int recvDQRet = recvQ.Dequeue((char*)&packet, sizeof(st_DRAW_PACKET));
        if (recvDQRet == 0) break;
        MoveToEx(hdc, packet.iStartX, packet.iStartY, nullptr);
        LineTo(hdc, packet.iEndX, packet.iEndY);
    }
    ReleaseDC(g_hWnd, hdc);
    return;
}