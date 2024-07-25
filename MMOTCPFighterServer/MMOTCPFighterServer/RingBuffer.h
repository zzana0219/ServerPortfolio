// �ۼ��� ť Ŭ���� �ʿ� �Լ�
#ifndef __RING_BUFFER__
#define __RING_BUFFER__

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

class CRingBuffer
{
private:
	char* begin;
	char* end;
	char* readPointer;
	char* writePointer;

	int ringBufferSize;
public:
	// ������.
	CRingBuffer(void);
	CRingBuffer(int iBufferSize);
	// ũ�� ������
	void Resize(int size);
	// ����ũ�� ��������
	int	GetBufferSize(void);
	//  ���� ������� �뷮 ���.
	int	GetUseSize(void);
	// ���� ���ۿ� ���� �뷮 ���. 
	int	GetFreeSize(void);
	// WritePos �� ����Ÿ ����.
	int	Enqueue(char* chpData, int iSize);
	// ReadPos ���� ����Ÿ ������. ReadPos �̵�.
	int	Dequeue(char* chpDest, int iSize);
	// ReadPos ���� ����Ÿ �о��. ReadPos ����
	int	Peek(char* chpDest, int iSize);
	// ������ ��� ����Ÿ ����
	void ClearBuffer(void);
	
	// ���� �����ͷ� �ܺο��� �ѹ濡 �а�, �� �� �ִ� ����
	int	DirectEnqueueSize(void);
	int	DirectDequeueSize(void);

	// ���ϴ� ���̸�ŭ �б���ġ ���� ���� / ���� ��ġ �̵�
	int	MoveRear(int iSize);
	int	MoveFront(int iSize);

	// ������ Front, rear ������
	char* GetFrontBufferPtr(void);
	char* GetRearBufferPtr(void);
};
#endif __RING_BUFFER__