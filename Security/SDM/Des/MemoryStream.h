#pragma once

enum TSeekAnchor
{
    SA_BEGINNING,
    SA_CURRENT,
    SA_END
};
class TMemoryStream
{
    public:
        TMemoryStream(void);
        ~TMemoryStream(void);
        
        int		Read(void* destBuffer, int count);
        int		Write(const void* srcBuffer, int count);
        char*	Memory();
        int		Position();
        int		Size();
        void	SetPosition(int position);
        void	SetSize(int size);
        int		Seek(const int offset, TSeekAnchor anchor);
        void	Clear();

    protected:
        void SetCapacity(int capacity);

    private:
        int   m_Position; //µ±«∞Œª÷√
        int   m_Size;
        int   m_Capacity;
        char* m_Memory;
};