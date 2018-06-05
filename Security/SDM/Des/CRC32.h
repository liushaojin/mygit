#pragma once
unsigned int GenerateCRC32(const char * buffer, unsigned int length);
unsigned long GetHardwareNumber();
class TDateTime : public COleDateTime
{
    public:
        TDateTime();
        TDateTime(const TDateTime& src);
        TDateTime(DATE dtSrc);
        TDateTime(unsigned int nYear, int nMonth, unsigned int nDay, unsigned int nHour, unsigned int nMin, unsigned int nSec);
        TDateTime(const int src);
        ~TDateTime();
        
        TDateTime& operator=(const int src) throw();
        TDateTime  operator+(const TDateTime& rhs)const;
        TDateTime  operator+(const int  rhs)const;
        TDateTime  operator-(const TDateTime& rhs)const;
        TDateTime  operator-(const int rhs)const;
        operator int()const;
        static TDateTime CurrentDateTime();
        static TDateTime OriginDateTime();
};
