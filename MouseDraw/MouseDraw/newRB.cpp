// Ringbuffer.cpp
#include "newRB.h"
#include<iostream>

  // 기본 생성자
CRingBuffer::CRingBuffer() : buffer(nullptr), bufferSize(0), readPos(0), writePos(0) 
{}

// 버퍼 크기를 지정하는 생성자
CRingBuffer::CRingBuffer(int iBufferSize) : bufferSize(iBufferSize), readPos(0), writePos(0)
{
    buffer = new char[bufferSize];
}

// 소멸자
CRingBuffer::~CRingBuffer()
{
    delete[] buffer;
}

// 버퍼 크기 재조정
void CRingBuffer::Resize(int size)
{
    delete[] buffer;
    bufferSize = size;
    buffer = new char[bufferSize];
    readPos = 0;
    writePos = 0;
}

// 버퍼 크기 얻기
int CRingBuffer::GetBufferSize() const
{
    return bufferSize;
}

// 현재 사용중인 용량 얻기
int CRingBuffer::GetUseSize() const
{
    if (writePos >= readPos)
        return writePos - readPos;
    else
        return bufferSize - readPos + writePos;
}

// 현재 버퍼에 남은 용량 얻기
int CRingBuffer::GetFreeSize() const 
{
    return bufferSize - GetUseSize() - 1;
}

// WritePos에 데이터 넣음
int CRingBuffer::Enqueue(const char* chpData, int iSize)
{
    int freeSize = GetFreeSize();
    if (freeSize < iSize)
        return 0; // 여유 공간이 부족하면 실패

    int firstPartSize = DirectEnqueueSize();
    firstPartSize = (firstPartSize > iSize) ? iSize : firstPartSize;

    // 첫 번째 부분 복사
    for (int i = 0; i < firstPartSize; ++i)
    {
        buffer[writePos + i] = chpData[i];
    }

    // 두 번째 부분 복사 (필요한 경우)
    int secondPartSize = iSize - firstPartSize;
    for (int i = 0; i < secondPartSize; ++i)
    {
        buffer[i] = chpData[firstPartSize + i];
    }

    MoveRear(iSize);
    return iSize;
}

// ReadPos에서 데이터 가져옴. ReadPos 이동
int CRingBuffer::Dequeue(char* chpDest, int iSize)
{
    if (GetUseSize() < iSize)
        return 0; // 사용중인 용량이 부족하면 실패

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

    // MoveFront 함수를 직접 호출하지 않고, readPos를 수동으로 업데이트하는 방식을 사용했습니다.
    // 여기서는 MoveFront(iSize); 를 호출해도 됩니다.

    return iSize;
}

// ReadPos에서 데이터 읽어옴. ReadPos 고정
int CRingBuffer::Peek(char* chpDest, int iSize) const
{
    if (GetUseSize() < iSize)
        return 0; // 사용중인 용량이 부족하면 실패

    int tempReadPos = readPos;
    int firstPartSize = DirectDequeueSize(); // 연속적으로 읽을 수 있는 길이
    if (iSize <= firstPartSize)
    {
        // 전체 데이터가 연속된 공간에 있을 경우
        for (int i = 0; i < iSize; ++i)
        {
            chpDest[i] = buffer[tempReadPos];
            tempReadPos = (tempReadPos + 1) % bufferSize;
        }
    }
    else {
        // 데이터가 버퍼의 끝과 시작에 분할되어 있을 경우
        // 첫 번째 부분 복사
        for (int i = 0; i < firstPartSize; ++i)
        {
            chpDest[i] = buffer[tempReadPos];
            tempReadPos = (tempReadPos + 1) % bufferSize;
        }
        // 나머지 부분 복사
        for (int i = firstPartSize; i < iSize; ++i) 
        {
            chpDest[i] = buffer[tempReadPos];
            tempReadPos = (tempReadPos + 1) % bufferSize;
        }
    }
    return iSize;
}

// 버퍼의 모든 데이터 삭제
void CRingBuffer::ClearBuffer() 
{
    readPos = 0;
    writePos = 0;
}

// 끊기지 않은 길이 얻기 - Enqueue
int CRingBuffer::DirectEnqueueSize() const 
{
    if (writePos >= readPos)
        return bufferSize - writePos;
    else
        return readPos - writePos - 1;
}

// 끊기지 않은 길이 얻기 - Dequeue
int CRingBuffer::DirectDequeueSize() const
{
    if (readPos <= writePos)
        return writePos - readPos;
    else
        return bufferSize - readPos;
}

// 원하는 길이만큼 쓰기 위치 이동
int CRingBuffer::MoveRear(int iSize) 
{
    if (GetFreeSize() < iSize)
        return 0; // 여유 공간이 부족하면 실패

    writePos = (writePos + iSize) % bufferSize;
    return iSize;
}

// 원하는 길이만큼 읽기 위치 이동
int CRingBuffer::MoveFront(int iSize) 
{
    if (GetUseSize() < iSize)
        return 0; // 사용중인 용량이 부족하면 실패

    readPos = (readPos + iSize) % bufferSize;
    return iSize;
}

// 버퍼의 Front 포인터 얻음
char* CRingBuffer::GetFrontBufferPtr() const 
{
    return buffer + readPos;
}

// 버퍼의 Rear 포인터 얻음
char* CRingBuffer::GetRearBufferPtr() const 
{
    return buffer + writePos;
}