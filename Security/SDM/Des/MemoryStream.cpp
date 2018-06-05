#include "StdAfx.h"
#include <string>
#include "MemoryStream.h"

TMemoryStream::TMemoryStream(void)
{
    m_Position = 0;
    m_Memory = NULL;
    m_Size = 0;
    m_Capacity = 0;
}

TMemoryStream::~TMemoryStream(void)
{
    free(m_Memory);
    m_Memory = NULL;
}

int TMemoryStream::Read(void* destBuffer, int count)
{
    int size = m_Position + count;
    
    if(size > Size())
    {
        count = Size() - m_Position;
    }
    
    memcpy(destBuffer, Memory() + m_Position, count);
    m_Position += count;
    return count;
}

int TMemoryStream::Write(const void* srcBuffer, int count)
{
    int size = m_Position + count;
    
    if(size > Size())
    {
        SetSize(size);
    }
    
    memcpy(Memory() + m_Position, srcBuffer, count);
    m_Position += count;
    return count;
}

char* TMemoryStream::Memory()
{
    return m_Memory;
}

int TMemoryStream::Position()
{
    return m_Position;
}

int TMemoryStream::Size()
{
    return m_Size;
}

void TMemoryStream::SetPosition(int position)
{
    if(position > Size())
    {
        m_Position = Size();
    }
    else
    {
        m_Position = position;
    }
}

void TMemoryStream::SetSize(int size)
{
    int oldPos = m_Position;
    SetCapacity(size);
    m_Size  = size;
    
    if(oldPos > size)
    {
        m_Position = Size();
    }
}

int TMemoryStream::Seek(const int offset, TSeekAnchor anchor)
{
    if(anchor == SA_BEGINNING)
    {
        SetPosition(offset);
    }
    else if(anchor == SA_CURRENT)
    {
        SetPosition(Position() + offset);
    }
    else if(anchor == SA_END)
    {
        SetPosition(Size() + offset);
    }
    
    return Position();
}

void TMemoryStream::Clear()
{
    SetSize(0);
    m_Position = 0;
}

void TMemoryStream::SetCapacity(int capacity)
{
    static const int delta = 0x2000;
    
    if((capacity > 0) && (capacity != m_Size))
    {
        capacity = (capacity + (delta - 1)) & (0xFFFFFFFF ^ (delta - 1));
    }
    
    if(capacity != m_Capacity)
    {
        if(capacity == 0)
        {
            free(m_Memory);
            m_Memory = NULL;
        }
        else
        {
            if(m_Capacity == 0)
            {
                m_Memory = (char*)malloc(capacity);
            }
            else
            {
                m_Memory = (char*)realloc(m_Memory, capacity);
            }
            
            if(m_Memory == NULL)
            {
                throw std::exception("TMemoryStream bad allocate the memory!");
            }
        }
        
        m_Capacity = capacity;
    }
}