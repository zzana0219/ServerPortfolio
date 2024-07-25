#include "RingBuffer.h"

CRingBuffer::CRingBuffer(void)
	: ringBufferSize(10000)					// 기본 버퍼 크기 설정
{
	begin = (char*)malloc(ringBufferSize);  // 메모리 할당
	end = begin + ringBufferSize;			// 끝 포인터 설정
	readPointer = writePointer = begin;		// 읽기/쓰기 포인터 초기화
}

// 크기를 받아서 초기화하는 생성자
CRingBuffer::CRingBuffer(int iBufferSize)
	: ringBufferSize(iBufferSize)  // 사용자 지정 버퍼 크기 설정
{
	begin = (char*)malloc(ringBufferSize);  // 메모리 할당
	end = begin + ringBufferSize;  // 끝 포인터 설정
	readPointer = writePointer = begin;  // 읽기/쓰기 포인터 초기화
}

// 버퍼 크기 조정 함수
void CRingBuffer::Resize(int size)
{
	free(begin);  // 기존 메모리 해제
	ringBufferSize = size;  // 새로운 크기 설정
	begin = (char*)malloc(ringBufferSize);  // 새 메모리 할당
	end = begin + ringBufferSize;  // 끝 포인터 재설정
	readPointer = writePointer = begin;  // 읽기/쓰기 포인터 재설정
}

// 버퍼 전체 크기 얻기 함수
int CRingBuffer::GetBufferSize(void)
{
	return ringBufferSize;
}

// 현재 사용 중인 용량 얻기 함수
int CRingBuffer::GetUseSize(void)
{
	if (writePointer >= readPointer)
		return writePointer - readPointer;
	return (writePointer - begin) + (end - readPointer);
}

// 남은 용량 얻기 함수
int CRingBuffer::GetFreeSize(void)
{
	//if (writePointer >= readPointer)
	//	return (end - writePointer) + (readPointer - begin - 1);
	//return readPointer - writePointer - 1;
	return ringBufferSize - GetUseSize() - 1;
}

// 버퍼에 데이터 쓰기 함수
int CRingBuffer::Enqueue(char* chpData, int iSize)
{
	if (GetFreeSize() < iSize)
	{
		printf("buffer full\n");
		return 0;
	}

	if (DirectEnqueueSize() >= iSize)
	{
		memcpy_s(writePointer, iSize, chpData, iSize); // 쓰기 포인터에 데이터 복사
		MoveRear(iSize);  // 쓰기 포인터 이동
		return iSize;
	}

	// 두 부분으로 나눠서 저장하는 경우 (쓰기 포인터가 끝을 넘어갈 때)
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

// 버퍼에서 데이터 읽기 함수
int CRingBuffer::Dequeue(char* chpDest, int iSize)
{
	if (GetUseSize() < iSize)
	{
		printf("버퍼에 저장된 사이즈가 더 적음\n");
		printf("[Sleep] chpDest : %s , GetBufferSize : %d , GetUseSize : %d, GetFreeSize : %d, readPointer : %p, writePointer : %p\n",
			chpDest, GetBufferSize(), GetUseSize(), GetFreeSize(), readPointer, writePointer);
		// Sleep(INFINITE); // 이 라인은 실제 운영 중에는 비활성화해야 합니다.
		return 0;
	}

	if (DirectDequeueSize() >= iSize)
	{
		memcpy_s(chpDest, iSize, readPointer, iSize); // 읽기 포인터에서 데이터 복사
		MoveFront(iSize);  // 읽기 포인터 이동
		return iSize;
	}

	// 두 부분으로 나눠서 읽는 경우 (읽기 포인터가 끝을 넘어갈 때)
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

	// 두 부분으로 나눠서 읽는 경우
	int firstPart = DirectDequeueSize();
	memcpy_s(chpDest, firstPart, readPointer, firstPart);
	MoveFront(firstPart);

	int secondPart = iSize - firstPart;
	memcpy_s(chpDest + firstPart, secondPart, readPointer, secondPart);
	MoveFront(secondPart);
	return iSize;
}

// 버퍼에서 데이터 읽기 위치 고정된 채로 데이터 읽기
int CRingBuffer::Peek(char* chpDest, int iSize)
{
	if (GetUseSize() < iSize)
	{
		printf("버퍼에 저장된 사이즈가 더 적음 GetUseSize : %d | iSize : %d\n", GetUseSize(), iSize);
		return 0;
	}
	if (DirectDequeueSize() >= iSize)
	{
		memcpy_s(chpDest, iSize, readPointer, iSize); // 읽기 포인터에서 데이터 복사 (읽기 포인터 고정)
		return iSize;
	}

	// 두 부분으로 나눠서 데이터를 읽는 경우
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
	//readPointer = pFrontTemp; // 읽기 포인터를 원래 위치로 되돌림
	//return iSize;

	// 두 부분으로 나누어 데이터를 읽는 경우
	char* pFrontTemp = readPointer;

	int firstPart = DirectDequeueSize();
	memcpy_s(chpDest, firstPart, readPointer, firstPart);
	MoveFront(firstPart);

	int secondPart = iSize - firstPart;
	memcpy_s(chpDest + firstPart, secondPart, readPointer, secondPart);

	readPointer = pFrontTemp; // 읽기 포인터를 원래 위치로 되돌림
	return iSize;
}

// 버퍼의 모든 데이터 삭제
void CRingBuffer::ClearBuffer(void)
{
	readPointer = writePointer = begin; // 읽기/쓰기 포인터 초기화
}

// 버퍼의 끊기지 않은 쓰기 가능 용량 얻기
int CRingBuffer::DirectEnqueueSize(void)
{
	if (writePointer >= readPointer)
		return end - writePointer/* - 1*/;
	return readPointer - writePointer - 1;
}

// 버퍼의 끊기지 않은 읽기 가능 용량 얻기
int CRingBuffer::DirectDequeueSize(void)
{
	if (writePointer >= readPointer)
		return writePointer - readPointer;
	return end - readPointer/* - 1*/;
}

// 쓰기 위치 이동 함수
int CRingBuffer::MoveRear(int iSize)
{
	writePointer += iSize;
	//if (writePointer >= end - 1)
	if (writePointer >= end) // 쓰기 포인터가 끝을 넘으면
	{
		int overflow = writePointer - end;
		writePointer = begin + overflow;
	}
	return iSize;
}

// 읽기 위치 이동 함수
int CRingBuffer::MoveFront(int iSize)
{
	readPointer += iSize;
	//if (readPointer >= end - 1)
	if (readPointer >= end) // 읽기 포인터가 끝을 넘으면
	{
		int overflow = readPointer - end;
		readPointer = begin + overflow;
	}
	return iSize;
}

// 버퍼의 읽기 포인터 얻기
char* CRingBuffer::GetFrontBufferPtr(void)
{
	return readPointer;
}

// 버퍼의 쓰기 포인터 얻기
char* CRingBuffer::GetRearBufferPtr(void)
{
	return writePointer;
}
