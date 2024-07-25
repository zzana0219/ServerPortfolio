// 송수신 큐 클래스 필요 함수
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
	// 생성자.
	CRingBuffer(void);
	CRingBuffer(int iBufferSize);
	// 크기 재조정
	void Resize(int size);
	// 버퍼크기 가져오기
	int	GetBufferSize(void);
	//  현재 사용중인 용량 얻기.
	int	GetUseSize(void);
	// 현재 버퍼에 남은 용량 얻기. 
	int	GetFreeSize(void);
	// WritePos 에 데이타 넣음.
	int	Enqueue(char* chpData, int iSize);
	// ReadPos 에서 데이타 가져옴. ReadPos 이동.
	int	Dequeue(char* chpDest, int iSize);
	// ReadPos 에서 데이타 읽어옴. ReadPos 고정
	int	Peek(char* chpDest, int iSize);
	// 버퍼의 모든 데이타 삭제
	void ClearBuffer(void);
	
	// 버퍼 포인터로 외부에서 한방에 읽고, 쓸 수 있는 길이
	int	DirectEnqueueSize(void);
	int	DirectDequeueSize(void);

	// 원하는 길이만큼 읽기위치 에서 삭제 / 쓰기 위치 이동
	int	MoveRear(int iSize);
	int	MoveFront(int iSize);

	// 버퍼의 Front, rear 포인터
	char* GetFrontBufferPtr(void);
	char* GetRearBufferPtr(void);
};
#endif __RING_BUFFER__