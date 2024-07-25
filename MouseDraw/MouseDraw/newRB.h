#pragma once


class CRingBuffer {
private:
    char* buffer;
    int bufferSize;
    int readPos;
    int writePos;

public:
    // �⺻ ������
    CRingBuffer();
    // ���� ũ�⸦ �����ϴ� ������
    CRingBuffer(int iBufferSize);
    // �Ҹ���
    ~CRingBuffer();
    // ���� ũ�� ������
    void Resize(int size);
    // ���� ũ�� ���
    int GetBufferSize() const;
    // ���� ������� �뷮 ���
    int GetUseSize() const;
    // ���� ���ۿ� ���� �뷮 ���
    int GetFreeSize() const;
    // WritePos�� ������ ����
    int Enqueue(const char* chpData, int iSize);
    // ReadPos���� ������ ������. ReadPos �̵�
    int Dequeue(char* chpDest, int iSize);
    // ReadPos���� ������ �о��. ReadPos ����
    int Peek(char* chpDest, int iSize) const;
    // ������ ��� ������ ����
    void ClearBuffer();
    // ������ ���� ���� ��� - Enqueue
    int DirectEnqueueSize() const;
    // ������ ���� ���� ��� - Dequeue
    int DirectDequeueSize() const;
    // ���ϴ� ���̸�ŭ ���� ��ġ �̵�
    int MoveRear(int iSize);
    // ���ϴ� ���̸�ŭ �б� ��ġ �̵�
    int MoveFront(int iSize);
    // ������ Front ������ ����
    char* GetFrontBufferPtr() const;
    // ������ Rear ������ ����
    char* GetRearBufferPtr() const;
};