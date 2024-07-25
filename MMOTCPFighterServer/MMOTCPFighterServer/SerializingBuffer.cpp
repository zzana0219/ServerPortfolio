#include "SerializingBuffer.h"
#include <iostream>

CPacket::CPacket() : m_iBufferSize(eBUFFER_DEFAULT), m_iDataSize(0), m_iReadPos(0), m_iWritePos(0) 
{
    m_chpBuffer = new char[m_iBufferSize];
    Clear();
}

CPacket::CPacket(int iBufferSize) : m_iBufferSize(iBufferSize), m_iDataSize(0), m_iReadPos(0), m_iWritePos(0)
{
    m_chpBuffer = new char[m_iBufferSize];
    Clear();
}

CPacket::~CPacket() 
{
    delete[] m_chpBuffer;
}

void CPacket::Clear() 
{
    // memset 대신 직접 할당
    for (int i = 0; i < m_iBufferSize; ++i)
        m_chpBuffer[i] = 0;

    m_iDataSize = 0;
    m_iReadPos = 0;
    m_iWritePos = 0;
}

int CPacket::MoveWritePos(int iSize) 
{
    if (iSize < 0 || m_iWritePos + iSize > m_iBufferSize) 
        return -1;

    m_iWritePos += iSize;
    m_iDataSize = m_iWritePos;
    return iSize;
}

int CPacket::MoveReadPos(int iSize) 
{
    if (iSize < 0 || m_iReadPos + iSize > m_iDataSize)
        return -1;

    m_iReadPos += iSize;
    return iSize;
}

// 연산자 오버로딩
CPacket& CPacket::operator=(CPacket& clSrcPacket) 
{
    if (this == &clSrcPacket)
        return *this;

    if (m_iBufferSize < clSrcPacket.m_iDataSize)
    {
        delete[] m_chpBuffer;
        m_iBufferSize = clSrcPacket.m_iDataSize;
        m_chpBuffer = new char[m_iBufferSize];
    }

    // memcpy 대신 직접 할당
    for (int i = 0; i < clSrcPacket.m_iDataSize; ++i)
        m_chpBuffer[i] = clSrcPacket.m_chpBuffer[i];

    m_iDataSize = clSrcPacket.m_iDataSize;
    m_iReadPos = clSrcPacket.m_iReadPos;
    m_iWritePos = clSrcPacket.m_iWritePos;
    return *this;
}

// 넣기.	각 변수 타입마다 모두 만듬.
CPacket& CPacket::operator<<(unsigned char byValue) 
{
    if (m_iWritePos + sizeof(unsigned char) > m_iBufferSize) 
        return *this;

    *reinterpret_cast<unsigned char*>(m_chpBuffer + m_iWritePos) = byValue;
    MoveWritePos(sizeof(unsigned char));
    return *this;
}

CPacket& CPacket::operator<<(char chValue) 
{
    if (m_iWritePos + sizeof(char) > m_iBufferSize)
        return *this;

    *reinterpret_cast<char*>(m_chpBuffer + m_iWritePos) = chValue;
    MoveWritePos(sizeof(char));
    return *this;
}

CPacket& CPacket::operator<<(short shValue)
{
    if (m_iWritePos + sizeof(short) > m_iBufferSize) 
        return *this;

    *reinterpret_cast<short*>(m_chpBuffer + m_iWritePos) = shValue;
    MoveWritePos(sizeof(short));
    return *this;
}

CPacket& CPacket::operator<<(unsigned short wValue)
{
    if (m_iWritePos + sizeof(unsigned short) > m_iBufferSize)
        return *this;

    *reinterpret_cast<unsigned short*>(m_chpBuffer + m_iWritePos) = wValue;
    MoveWritePos(sizeof(unsigned short));
    return *this;
}

CPacket& CPacket::operator<<(int iValue) 
{
    if (m_iWritePos + sizeof(int) > m_iBufferSize) 
        return *this;

    *reinterpret_cast<int*>(m_chpBuffer + m_iWritePos) = iValue;
    MoveWritePos(sizeof(int));
    return *this;
}

CPacket& CPacket::operator<<(long lValue)
{
    if (m_iWritePos + sizeof(long) > m_iBufferSize)
        return *this;

    *reinterpret_cast<long*>(m_chpBuffer + m_iWritePos) = lValue;
    MoveWritePos(sizeof(long));
    return *this;
}

CPacket& CPacket::operator<<(unsigned long dwValue)
{
    if (m_iWritePos + sizeof(unsigned long) > m_iBufferSize)
        return *this;

    *reinterpret_cast<unsigned long*>(m_chpBuffer + m_iWritePos) = dwValue;
    MoveWritePos(sizeof(unsigned long));
    return *this;
}

CPacket& CPacket::operator<<(float fValue) 
{
    if (m_iWritePos + sizeof(float) > m_iBufferSize)
        return *this;

    // memcpy 대신 직접 할당
    *reinterpret_cast<float*>(m_chpBuffer + m_iWritePos) = fValue;
    MoveWritePos(sizeof(float));
    return *this;
}

CPacket& CPacket::operator<<(__int64 iValue)
{
    if (m_iWritePos + sizeof(__int64) > m_iBufferSize) 
        return *this;

    *reinterpret_cast<__int64*>(m_chpBuffer + m_iWritePos) = iValue;
    MoveWritePos(sizeof(__int64));
    return *this;
}

CPacket& CPacket::operator<<(double dValue)
{
    if (m_iWritePos + sizeof(double) > m_iBufferSize)
        return *this;

    *reinterpret_cast<double*>(m_chpBuffer + m_iWritePos) = dValue;
    MoveWritePos(sizeof(double));
    return *this;
}


// 빼기.	각 변수 타입마다 모두 만듬.
CPacket& CPacket::operator>>(unsigned char& byValue) 
{
    if (m_iReadPos + sizeof(unsigned char) > m_iDataSize) 
        return *this;

    byValue = *reinterpret_cast<unsigned char*>(m_chpBuffer + m_iReadPos);
    MoveReadPos(sizeof(unsigned char));
    return *this;
}

CPacket& CPacket::operator>>(char& chValue)
{
    if (m_iReadPos + sizeof(char) > m_iDataSize)
        return *this;

    chValue = *reinterpret_cast<char*>(m_chpBuffer + m_iReadPos);
    MoveReadPos(sizeof(char));
    return *this;
}

CPacket& CPacket::operator>>(short& shValue)
{
    if (m_iReadPos + sizeof(short) > m_iDataSize)
        return *this;

    shValue = *reinterpret_cast<short*>(m_chpBuffer + m_iReadPos);
    MoveReadPos(sizeof(short));
    return *this;
}

CPacket& CPacket::operator>>(unsigned short& wValue)
{
    if (m_iReadPos + sizeof(unsigned short) > m_iDataSize)
        return *this;

    wValue = *reinterpret_cast<unsigned short*>(m_chpBuffer + m_iReadPos);
    MoveReadPos(sizeof(unsigned short));
    return *this;
}

CPacket& CPacket::operator>>(int& iValue)
{
    if (m_iReadPos + sizeof(int) > m_iDataSize) 
        return *this;

    iValue = *reinterpret_cast<int*>(m_chpBuffer + m_iReadPos);
    MoveReadPos(sizeof(int));
    return *this;
}

CPacket& CPacket::operator>>(long& lValue)
{
    if (m_iReadPos + sizeof(long) > m_iDataSize) 
        return *this;

    lValue = *reinterpret_cast<long*>(m_chpBuffer + m_iReadPos);
    MoveReadPos(sizeof(long));
    return *this;
}

CPacket& CPacket::operator>>(unsigned long& dwValue)
{
    if (m_iReadPos + sizeof(unsigned long) > m_iDataSize)
        return *this;

    dwValue = *reinterpret_cast<unsigned long*>(m_chpBuffer + m_iReadPos);
    MoveReadPos(sizeof(unsigned long));
    return *this;
}

CPacket& CPacket::operator>>(float& fValue)
{
    if (m_iReadPos + sizeof(float) > m_iDataSize)
        return *this;

    fValue = *reinterpret_cast<float*>(m_chpBuffer + m_iReadPos);
    MoveReadPos(sizeof(float));
    return *this;
}

CPacket& CPacket::operator>>(__int64& iValue)
{
    if (m_iReadPos + sizeof(__int64) > m_iDataSize) 
        return *this;

    iValue = *reinterpret_cast<__int64*>(m_chpBuffer + m_iReadPos);
    MoveReadPos(sizeof(__int64));
    return *this;
}

CPacket& CPacket::operator>>(double& dValue) 
{
    if (m_iReadPos + sizeof(double) > m_iDataSize) 
        return *this;

    dValue = *reinterpret_cast<double*>(m_chpBuffer + m_iReadPos);
    MoveReadPos(sizeof(double));
    return *this;
}

int CPacket::GetData(char* chpDest, int iSize) 
{
    if (iSize > m_iDataSize)
        iSize = m_iDataSize;

    for (int i = 0; i < iSize; ++i)
        chpDest[i] = m_chpBuffer[i];
    
    return iSize;
}

int CPacket::PutData(char* chpSrc, int iSrcSize) 
{
    if (iSrcSize + m_iWritePos > m_iBufferSize)
        return -1;

    for (int i = 0; i < iSrcSize; ++i)
        m_chpBuffer[m_iWritePos + i] = chpSrc[i];

    MoveWritePos(iSrcSize);
    return iSrcSize;
}
