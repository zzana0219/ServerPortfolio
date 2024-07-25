#pragma once
/*---------------------------------------------------------------
	Packet.
	��Ʈ��ũ ��Ŷ�� Ŭ����.
	�����ϰ� ��Ŷ�� ������� ����Ÿ�� In, Out �Ѵ�.
	- ����.
	CPacket cPacket;  or CMessage Message;
	�ֱ�.
	clPacket << 40030;	or	clPacket << iValue;	(int �ֱ�)
	clPacket << 1.4;	or	clPacket << fValue;	(float �ֱ�)
	����.
	clPacket >> iValue;		(int ����)
	clPacket >> byValue;	(BYTE ����)
	clPacket >> fValue;		(float ����)

	CPacket Packet2;
	!.	���ԵǴ� ����Ÿ FIFO ������ �����ȴ�.
		ȯ�� ť�� �ƴϹǷ�, �ֱ�(<<).����(>>) �� ȥ���ؼ� ������� �ʵ��� �Ѵ�

	* ���� ��Ŷ ���ν��������� ó��
	BOOL	netPacketProc_CreateMyCharacter(CPacket *clpPacket)
	{
		DWORD dwSessionID;
		short shX, shY;
		char chHP;
		BYTE byDirection;

//		*clpPacket >> dwSessionID >> byDirection >> shX >> shY >> chHP;


		*clpPacket >> dwSessionID;
		*clpPacket >> byDirection;
		*clpPacket >> shX;
		*clpPacket >> shY;
		*clpPacket >> chHP;

		...
		...
	}

	* ���� �޽���(��Ŷ) �����ο����� ó��

	CPacket MoveStart;
	mpMoveStart(&MoveStart, dir, x, y);
	SendPacket(&MoveStart);

	void	mpMoveStart(CPacket *clpPacket, BYTE byDirection, short shX, short shY)
	{
		st_NETWORK_PACKET_HEADER	stPacketHeader;
		stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
		stPacketHeader.bySize = 5;
		stPacketHeader.byType = dfPACKET_CS_MOVE_START;

		clpPacket->PutData((char *)&stPacketHeader, dfNETWORK_PACKET_HEADER_SIZE);

		*clpPacket << byDirection;
		*clpPacket << shX;
		*clpPacket << shY;
	}

----------------------------------------------------------------*/
#ifndef  __PACKET__
#define  __PACKET__

//#include <cstring> // memcpy ����� ����

class CPacket
{
public:
	//Packet Enum.
	enum en_PACKET
	{
		eBUFFER_DEFAULT = 1400		// ��Ŷ�� �⺻ ���� ������.
	};

	// ������, �ı���.
	CPacket();
	CPacket(int iBufferSize);

	virtual	~CPacket();

	// ��Ŷ û��.
	// Parameters: ����.
	// Return: ����.
	void Clear(void);

	void ReSize();	// �ʿ��Ѱ�? �ϴ� �ʿ�� ������, �Ҷ� �α׸� ���ܼ� ��� ���� �Ǵ��� �ؾ��Ѵ�.

	// ���� ������ ���.
	// Parameters: ����.
	// Return: (int)��Ŷ ���� ������ ���.
	int	GetBufferSize(void) { return m_iBufferSize; }

	// ���� ������� ������ ���.
	// Parameters: ����.
	// Return: (int)������� ����Ÿ ������.
	int	GetDataSize(void) { return m_iDataSize; }

	// ���� ������ ���.
	// Parameters: ����.
	// Return: (char *)���� ������.
	char* GetBufferPtr(void) { return m_chpBuffer; }

	// ���� Pos �̵�. (�����̵��� �ȵ�)
	// GetBufferPtr �Լ��� �̿��Ͽ� �ܺο��� ������ ���� ������ ������ ��� ���. 
	// Parameters: (int) �̵� ������.
	// Return: (int) �̵��� ������.
	int MoveWritePos(int iSize);
	int MoveReadPos(int iSize);

	// ������ �����ε�
	CPacket& operator = (CPacket& clSrcPacket);

	// �ֱ�.	�� ���� Ÿ�Ը��� ��� ����.
	CPacket& operator << (unsigned char byValue);
	CPacket& operator << (char chValue);

	CPacket& operator << (short shValue);
	CPacket& operator << (unsigned short wValue);

	CPacket& operator << (int iValue);
	CPacket& operator << (long lValue);
	CPacket& operator << (unsigned long dwValue);
	CPacket& operator << (float fValue);

	CPacket& operator << (__int64 iValue);
	CPacket& operator << (double dValue);

	// ����.	�� ���� Ÿ�Ը��� ��� ����.
	CPacket& operator >> (unsigned char& byValue);
	CPacket& operator >> (char& chValue);

	CPacket& operator >> (short& shValue);
	CPacket& operator >> (unsigned short& wValue);

	CPacket& operator >> (int& iValue);
	CPacket& operator >> (long& lValue);
	CPacket& operator >> (unsigned long& dwValue);
	CPacket& operator >> (float& fValue);

	CPacket& operator >> (__int64& iValue);
	CPacket& operator >> (double& dValue);

	// ����Ÿ ���.
	// Parameters: (char *)Dest ������. (int)Size.
	// Return: (int)������ ������.
	int GetData(char* chpDest, int iSize);

	// ����Ÿ ����.
	// Parameters: (char *)Src ������. (int)SrcSize.
	// Return: (int)������ ������.
	int PutData(char* chpSrc, int iSrcSize);

protected:
	int	m_iBufferSize;
	int	m_iDataSize;	// ���� ���ۿ� ������� ������.
	char* m_chpBuffer;
	int m_iReadPos;
	int m_iWritePos;
};

#endif
