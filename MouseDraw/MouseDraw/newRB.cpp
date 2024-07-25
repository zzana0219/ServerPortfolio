// Ringbuffer.cpp
#include "newRB.h"
#include<iostream>

  // �⺻ ������
CRingBuffer::CRingBuffer() : buffer(nullptr), bufferSize(0), readPos(0), writePos(0) 
{}

// ���� ũ�⸦ �����ϴ� ������
CRingBuffer::CRingBuffer(int iBufferSize) : bufferSize(iBufferSize), readPos(0), writePos(0)
{
    buffer = new char[bufferSize];
}

// �Ҹ���
CRingBuffer::~CRingBuffer()
{
    delete[] buffer;
}

// ���� ũ�� ������
void CRingBuffer::Resize(int size)
{
    delete[] buffer;
    bufferSize = size;
    buffer = new char[bufferSize];
    readPos = 0;
    writePos = 0;
}

// ���� ũ�� ���
int CRingBuffer::GetBufferSize() const
{
    return bufferSize;
}

// ���� ������� �뷮 ���
int CRingBuffer::GetUseSize() const
{
    if (writePos >= readPos)
        return writePos - readPos;
    else
        return bufferSize - readPos + writePos;
}

// ���� ���ۿ� ���� �뷮 ���
int CRingBuffer::GetFreeSize() const 
{
    return bufferSize - GetUseSize() - 1;
}

// WritePos�� ������ ����
int CRingBuffer::Enqueue(const char* chpData, int iSize)
{
    int freeSize = GetFreeSize();
    if (freeSize < iSize)
        return 0; // ���� ������ �����ϸ� ����

    int firstPartSize = DirectEnqueueSize();
    firstPartSize = (firstPartSize > iSize) ? iSize : firstPartSize;

    // ù ��° �κ� ����
    for (int i = 0; i < firstPartSize; ++i)
    {
        buffer[writePos + i] = chpData[i];
    }

    // �� ��° �κ� ���� (�ʿ��� ���)
    int secondPartSize = iSize - firstPartSize;
    for (int i = 0; i < secondPartSize; ++i)
    {
        buffer[i] = chpData[firstPartSize + i];
    }

    MoveRear(iSize);
    return iSize;
}

// ReadPos���� ������ ������. ReadPos �̵�
int CRingBuffer::Dequeue(char* chpDest, int iSize)
{
    if (GetUseSize() < iSize)
        return 0; // ������� �뷮�� �����ϸ� ����

    int firstPartSize = DirectDequeueSize();
    if (iSize < firstPartSize) 
    {
        firstPartSize = iSize;
    }

    for (int i = 0; i < firstPartSize; ++i)
    {
        chpDest[i] = buffer[readPos];
        readPos = (readPos + 1) % bufferSize;
    }

    int secondPartSize = iSize - firstPartSize;
    if (secondPartSize > 0) 
    {
        for (int i = 0; i < secondPartSize; ++i) 
        {
            chpDest[firstPartSize + i] = buffer[readPos];
            readPos = (readPos + 1) % bufferSize;
        }
    }

    // MoveFront �Լ��� ���� ȣ������ �ʰ�, readPos�� �������� ������Ʈ�ϴ� ����� ����߽��ϴ�.
    // ���⼭�� MoveFront(iSize); �� ȣ���ص� �˴ϴ�.

    return iSize;
}

// ReadPos���� ������ �о��. ReadPos ����
int CRingBuffer::Peek(char* chpDest, int iSize) const
{
    if (GetUseSize() < iSize)
        return 0; // ������� �뷮�� �����ϸ� ����

    int tempReadPos = readPos;
    int firstPartSize = DirectDequeueSize(); // ���������� ���� �� �ִ� ����
    if (iSize <= firstPartSize)
    {
        // ��ü �����Ͱ� ���ӵ� ������ ���� ���
        for (int i = 0; i < iSize; ++i)
        {
            chpDest[i] = buffer[tempReadPos];
            tempReadPos = (tempReadPos + 1) % bufferSize;
        }
    }
    else {
        // �����Ͱ� ������ ���� ���ۿ� ���ҵǾ� ���� ���
        // ù ��° �κ� ����
        for (int i = 0; i < firstPartSize; ++i)
        {
            chpDest[i] = buffer[tempReadPos];
            tempReadPos = (tempReadPos + 1) % bufferSize;
        }
        // ������ �κ� ����
        for (int i = firstPartSize; i < iSize; ++i) 
        {
            chpDest[i] = buffer[tempReadPos];
            tempReadPos = (tempReadPos + 1) % bufferSize;
        }
    }
    return iSize;
}

// ������ ��� ������ ����
void CRingBuffer::ClearBuffer() 
{
    readPos = 0;
    writePos = 0;
}

// ������ ���� ���� ��� - Enqueue
int CRingBuffer::DirectEnqueueSize() const 
{
    if (writePos >= readPos)
        return bufferSize - writePos;
    else
        return readPos - writePos - 1;
}

// ������ ���� ���� ��� - Dequeue
int CRingBuffer::DirectDequeueSize() const
{
    if (readPos <= writePos)
        return writePos - readPos;
    else
        return bufferSize - readPos;
}

// ���ϴ� ���̸�ŭ ���� ��ġ �̵�
int CRingBuffer::MoveRear(int iSize) 
{
    if (GetFreeSize() < iSize)
        return 0; // ���� ������ �����ϸ� ����

    writePos = (writePos + iSize) % bufferSize;
    return iSize;
}

// ���ϴ� ���̸�ŭ �б� ��ġ �̵�
int CRingBuffer::MoveFront(int iSize) 
{
    if (GetUseSize() < iSize)
        return 0; // ������� �뷮�� �����ϸ� ����

    readPos = (readPos + iSize) % bufferSize;
    return iSize;
}

// ������ Front ������ ����
char* CRingBuffer::GetFrontBufferPtr() const 
{
    return buffer + readPos;
}

// ������ Rear ������ ����
char* CRingBuffer::GetRearBufferPtr() const 
{
    return buffer + writePos;
}