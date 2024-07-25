#pragma comment (lib, "ws2_32")

#include "framework.h"
#include "MouseDraw.h"
#include"RingBuffer.h"
#include<WS2tcpip.h>
#include<windowsx.h>
#include <time.h>

#define MAX_LOADSTRING 100
#define UM_NETWORK (WM_USER + 1)

// ���� ����:
HINSTANCE hInst;                                // ���� �ν��Ͻ��Դϴ�.
WCHAR szTitle[MAX_LOADSTRING];                  // ���� ǥ���� �ؽ�Ʈ�Դϴ�.
WCHAR szWindowClass[MAX_LOADSTRING];            // �⺻ â Ŭ���� �̸��Դϴ�.

// �� �ڵ� ��⿡ ���Ե� �Լ��� ������ �����մϴ�:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// ���
struct st_HEADER
{
    unsigned short Len;
};

// ��Ŷ
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

    // ���� ���ڿ��� �ʱ�ȭ�մϴ�.
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
    // �⺻ �޽��� �����Դϴ�:
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

//  �Լ�: MyRegisterClass()
//  �뵵: â Ŭ������ ����մϴ�.
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

//   �Լ�: InitInstance(HINSTANCE, int)
//   �뵵: �ν��Ͻ� �ڵ��� �����ϰ� �� â�� ����ϴ�.
//   �ּ�:
//        �� �Լ��� ���� �ν��Ͻ� �ڵ��� ���� ������ �����ϰ�
//        �� ���α׷� â�� ���� ���� ǥ���մϴ�.
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

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

//  �Լ�: WndProc(HWND, UINT, WPARAM, LPARAM)
//  �뵵: �� â�� �޽����� ó���մϴ�.
//  WM_COMMAND  - ���ø����̼� �޴��� ó���մϴ�.
//  WM_PAINT    - �� â�� �׸��ϴ�.
//  WM_DESTROY  - ���� �޽����� �Խ��ϰ� ��ȯ�մϴ�.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // �޴� ������ ���� �м��մϴ�:
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
        // TODO: ���⿡ hdc�� ����ϴ� �׸��� �ڵ带 �߰��մϴ�...
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

// ���� ��ȭ ������ �޽��� ó�����Դϴ�.
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
        // recvRingBuffer ���� ��
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