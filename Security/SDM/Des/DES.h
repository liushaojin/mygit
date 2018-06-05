#ifndef __DES_H__
#define __DES_H__
#include <string>
#include "MemoryStream.h"
class TDES
{
public:
	TDES();
	~TDES();

	void MakeKey(std::string key);
	bool EncryptNumber(TMemoryStream &in,TMemoryStream &out);
	bool DecryptNumber(TMemoryStream &in,TMemoryStream &out);
private:
	char Key[200];
};
#endif // __DES_H__
