#include "RingBuffer.h"

CRingBuffer::CRingBuffer(void)
	: ringBufferSize(10000)					// �⺻ ���� ũ�� ����
{
	begin = (char*)malloc(ringBufferSize);  // �޸� �Ҵ�
	end = begin + ringBufferSize;			// �� ������ ����
	readPointer = writePointer = begin;		// �б�/���� ������ �ʱ�ȭ
}

// ũ�⸦ �޾Ƽ� �ʱ�ȭ�ϴ� ������
CRingBuffer::CRingBuffer(int iBufferSize)
	: ringBufferSize(iBufferSize)  // ����� ���� ���� ũ�� ����
{
	begin = (char*)malloc(ringBufferSize);  // �޸� �Ҵ�
	end = begin + ringBufferSize;  // �� ������ ����
	readPointer = writePointer = begin;  // �б�/���� ������ �ʱ�ȭ
}

// ���� ũ�� ���� �Լ�
void CRingBuffer::Resize(int size)
{
	free(begin);  // ���� �޸� ����
	ringBufferSize = size;  // ���ο� ũ�� ����
	begin = (char*)malloc(ringBufferSize);  // �� �޸� �Ҵ�
	end = begin + ringBufferSize;  // �� ������ �缳��
	readPointer = writePointer = begin;  // �б�/���� ������ �缳��
}

// ���� ��ü ũ�� ��� �Լ�
int CRingBuffer::GetBufferSize(void)
{
	return ringBufferSize;
}

// ���� ��� ���� �뷮 ��� �Լ�
int CRingBuffer::GetUseSize(void)
{
	if (writePointer >= readPointer)
		return writePointer - readPointer;
	return (writePointer - begin) + (end - readPointer);
}

// ���� �뷮 ��� �Լ�
int CRingBuffer::GetFreeSize(void)
{
	//if (writePointer >= readPointer)
	//	return (end - writePointer) + (readPointer - begin - 1);
	//return readPointer - writePointer - 1;
	return ringBufferSize - GetUseSize() - 1;
}

// ���ۿ� ������ ���� �Լ�
int CRingBuffer::Enqueue(char* chpData, int iSize)
{
	if (GetFreeSize() < iSize)
	{
		printf("buffer full\n");
		return 0;
	}

	if (DirectEnqueueSize() >= iSize)
	{
		memcpy_s(writePointer, iSize, chpData, iSize); // ���� �����Ϳ� ������ ����
		MoveRear(iSize);  // ���� ������ �̵�
		return iSize;
	}

	// �� �κ����� ������ �����ϴ� ��� (���� �����Ͱ� ���� �Ѿ ��)
	//char* temp = chpData;
	//int directEnqueueSize = DirectEnqueueSize();
	//memcpy_s(writePointer, directEnqueueSize, temp, directEnqueueSize);
	//temp += directEnqueueSize;
	//MoveRear(directEnqueueSize);

	//int remainSize = iSize - directEnqueueSize;
	//memcpy_s(writePointer, remainSize, temp, remainSize);
	//MoveRear(remainSize);
	//return iSize;

	int firstPart = DirectEnqueueSize();
	memcpy_s(writePointer, firstPart, chpData, firstPart);
	MoveRear(firstPart);

	int secondPart = iSize - firstPart;
	memcpy_s(writePointer, secondPart, chpData + firstPart, secondPart);
	MoveRear(secondPart);
	return iSize;
}

// ���ۿ��� ������ �б� �Լ�
int CRingBuffer::Dequeue(char* chpDest, int iSize)
{
	if (GetUseSize() < iSize)
	{
		printf("���ۿ� ����� ����� �� ����\n");
		printf("[Sleep] chpDest : %s , GetBufferSize : %d , GetUseSize : %d, GetFreeSize : %d, readPointer : %p, writePointer : %p\n",
			chpDest, GetBufferSize(), GetUseSize(), GetFreeSize(), readPointer, writePointer);
		// Sleep(INFINITE); // �� ������ ���� � �߿��� ��Ȱ��ȭ�ؾ� �մϴ�.
		return 0;
	}

	if (DirectDequeueSize() >= iSize)
	{
		memcpy_s(chpDest, iSize, readPointer, iSize); // �б� �����Ϳ��� ������ ����
		MoveFront(iSize);  // �б� ������ �̵�
		return iSize;
	}

	// �� �κ����� ������ �д� ��� (�б� �����Ͱ� ���� �Ѿ ��)
	//char* pDestTemp = chpDest;
	//int directDequeueSize = DirectDequeueSize();
	//memcpy_s(pDestTemp, directDequeueSize, readPointer, directDequeueSize);
	//MoveFront(directDequeueSize);
	//
	//int remainSize = iSize - directDequeueSize;
	//pDestTemp += directDequeueSize;
	//memcpy_s(pDestTemp, remainSize, readPointer, remainSize);
	//MoveFront(remainSize);
	//return iSize;

	// �� �κ����� ������ �д� ���
	int firstPart = DirectDequeueSize();
	memcpy_s(chpDest, firstPart, readPointer, firstPart);
	MoveFront(firstPart);

	int secondPart = iSize - firstPart;
	memcpy_s(chpDest + firstPart, secondPart, readPointer, secondPart);
	MoveFront(secondPart);
	return iSize;
}

// ���ۿ��� ������ �б� ��ġ ������ ä�� ������ �б�
int CRingBuffer::Peek(char* chpDest, int iSize)
{
	if (GetUseSize() < iSize)
	{
		printf("���ۿ� ����� ����� �� ���� GetUseSize : %d | iSize : %d\n", GetUseSize(), iSize);
		return 0;
	}
	if (DirectDequeueSize() >= iSize)
	{
		memcpy_s(chpDest, iSize, readPointer, iSize); // �б� �����Ϳ��� ������ ���� (�б� ������ ����)
		return iSize;
	}

	// �� �κ����� ������ �����͸� �д� ���
	//char* pFrontTemp = readPointer;
	//
	//char* pDestTemp = chpDest;
	//int directDequeueSize = DirectDequeueSize();
	//memcpy_s(pDestTemp, directDequeueSize, readPointer, directDequeueSize);
	//MoveFront(directDequeueSize);
	//
	//int remainSize = iSize - directDequeueSize;
	//pDestTemp += directDequeueSize;
	//memcpy_s(pDestTemp, remainSize, readPointer, remainSize);
	//
	//readPointer = pFrontTemp; // �б� �����͸� ���� ��ġ�� �ǵ���
	//return iSize;

	// �� �κ����� ������ �����͸� �д� ���
	char* pFrontTemp = readPointer;

	int firstPart = DirectDequeueSize();
	memcpy_s(chpDest, firstPart, readPointer, firstPart);
	MoveFront(firstPart);

	int secondPart = iSize - firstPart;
	memcpy_s(chpDest + firstPart, secondPart, readPointer, secondPart);

	readPointer = pFrontTemp; // �б� �����͸� ���� ��ġ�� �ǵ���
	return iSize;
}

// ������ ��� ������ ����
void CRingBuffer::ClearBuffer(void)
{
	readPointer = writePointer = begin; // �б�/���� ������ �ʱ�ȭ
}

// ������ ������ ���� ���� ���� �뷮 ���
int CRingBuffer::DirectEnqueueSize(void)
{
	if (writePointer >= readPointer)
		return end - writePointer/* - 1*/;
	return readPointer - writePointer - 1;
}

// ������ ������ ���� �б� ���� �뷮 ���
int CRingBuffer::DirectDequeueSize(void)
{
	if (writePointer >= readPointer)
		return writePointer - readPointer;
	return end - readPointer/* - 1*/;
}

// ���� ��ġ �̵� �Լ�
int CRingBuffer::MoveRear(int iSize)
{
	writePointer += iSize;
	//if (writePointer >= end - 1)
	if (writePointer >= end) // ���� �����Ͱ� ���� ������
	{
		int overflow = writePointer - end;
		writePointer = begin + overflow;
	}
	return iSize;
}

// �б� ��ġ �̵� �Լ�
int CRingBuffer::MoveFront(int iSize)
{
	readPointer += iSize;
	//if (readPointer >= end - 1)
	if (readPointer >= end) // �б� �����Ͱ� ���� ������
	{
		int overflow = readPointer - end;
		readPointer = begin + overflow;
	}
	return iSize;
}

// ������ �б� ������ ���
char* CRingBuffer::GetFrontBufferPtr(void)
{
	return readPointer;
}

// ������ ���� ������ ���
char* CRingBuffer::GetRearBufferPtr(void)
{
	return writePointer;
}
