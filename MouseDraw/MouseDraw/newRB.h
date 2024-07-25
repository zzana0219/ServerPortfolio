#pragma once


class CRingBuffer {
private:
    char* buffer;
    int bufferSize;
    int readPos;
    int writePos;

public:
    // 기본 생성자
    CRingBuffer();
    // 버퍼 크기를 지정하는 생성자
    CRingBuffer(int iBufferSize);
    // 소멸자
    ~CRingBuffer();
    // 버퍼 크기 재조정
    void Resize(int size);
    // 버퍼 크기 얻기
    int GetBufferSize() const;
    // 현재 사용중인 용량 얻기
    int GetUseSize() const;
    // 현재 버퍼에 남은 용량 얻기
    int GetFreeSize() const;
    // WritePos에 데이터 넣음
    int Enqueue(const char* chpData, int iSize);
    // ReadPos에서 데이터 가져옴. ReadPos 이동
    int Dequeue(char* chpDest, int iSize);
    // ReadPos에서 데이터 읽어옴. ReadPos 고정
    int Peek(char* chpDest, int iSize) const;
    // 버퍼의 모든 데이터 삭제
    void ClearBuffer();
    // 끊기지 않은 길이 얻기 - Enqueue
    int DirectEnqueueSize() const;
    // 끊기지 않은 길이 얻기 - Dequeue
    int DirectDequeueSize() const;
    // 원하는 길이만큼 쓰기 위치 이동
    int MoveRear(int iSize);
    // 원하는 길이만큼 읽기 위치 이동
    int MoveFront(int iSize);
    // 버퍼의 Front 포인터 얻음
    char* GetFrontBufferPtr() const;
    // 버퍼의 Rear 포인터 얻음
    char* GetRearBufferPtr() const;
};