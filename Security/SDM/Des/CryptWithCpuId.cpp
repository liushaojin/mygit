#include "CryptWithCpuId.h"

CryptWithCpuId::CryptWithCpuId():
    m_key(0),
    m_compressionMode(CompressionAuto),
    m_protectionMode(ProtectionChecksum),
    m_lastError(ErrorNoError)
{
    //qsrand(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFF));
}

CryptWithCpuId::CryptWithCpuId(DWORD key):
    m_key(key),
    m_compressionMode(CompressionAuto),
    m_protectionMode(ProtectionChecksum),
    m_lastError(ErrorNoError)
{
    //qsrand(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFF));
    splitKey();
}

void CryptWithCpuId::setKey(DWORD key)
{
    m_key = key;
    splitKey();
}

void CryptWithCpuId::splitKey()
{
    m_keyParts.clear();
    m_keyParts.resize(8);
    
    for (int i = 0; i < 8; i++)
    {
        DWORD part = m_key;
        
        for (int j = i; j > 0; j--)
        {
            part = part >> 8;
        }
        
        part = part & 0xff;
        m_keyParts[i] = static_cast<char>(part);
    }
}


CByteArray CryptWithCpuId::encryptToByteArray(const CString& plaintext)
{
	CByteArray plaintextArray;
	int nSize = plaintext.GetLength() * sizeof(CHAR);
	plaintextArray.SetSize(nSize);
	lstrcpy((LPTSTR)plaintextArray.GetData(), plaintext);
	//char buf[64];
	//strcpy((char*)buf, (LPSTR)(LPCTSTR)plaintext);
    //CByteArray plaintextArray = (BYTE*)plaintext.GetBuffer(plaintext.GetLength());
    return encryptToByteArray(&plaintextArray);
}

CByteArray CryptWithCpuId::encryptToByteArray(CByteArray plaintext)
{
    if (m_keyParts.empty())
    {
        //qWarning() << "No key set.";
        m_lastError = ErrorNoKeySet;
		CByteArray byteArray;
        return byteArray;
    }
    
    CByteArray ba = plaintext;
    CryptoFlag flags = CryptoFlagNone;
    
    if (m_compressionMode == CompressionAlways)
    {
        ba = qCompress(ba, 9); //maximum compression
        flags |= CryptoFlagCompression;
    }
    else if (m_compressionMode == CompressionAuto)
    {
        CByteArray compressed = qCompress(ba, 9);
        
        if (compressed.count() < ba.count())
        {
            ba = compressed;
            flags |= CryptoFlagCompression;
        }
    }
    
    CByteArray integrityProtection;
    
    if (m_protectionMode == ProtectionChecksum)
    {
        flags |= CryptoFlagChecksum;
        QDataStream s(&integrityProtection, QIODevice::WriteOnly);
        s << qChecksum(ba.constData(), ba.length());
    }
    else if (m_protectionMode == ProtectionHash)
    {
        flags |= CryptoFlagHash;
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(ba);
        integrityProtection += hash.result();
    }
    
    //prepend a random char to the string
    char randomChar = char(qrand() & 0xFF);
    ba = randomChar + integrityProtection + ba;
    int pos(0);
    char lastChar(0);
    int cnt = ba.GetCount();
    
    while (pos < cnt)
    {
        ba[pos] = ba.GetAt(pos) ^ m_keyParts.at(pos % 8) ^ lastChar;
        lastChar = ba.GetAt(pos);
        ++pos;
    }
    
    CByteArray resultArray;
    resultArray.Add(char(0x03));  //version for future updates to algorithm
    resultArray.Add(char(flags)); //encryption flags
    resultArray.Append(ba);
    m_lastError = ErrorNoError;
    return resultArray;
}

CString CryptWithCpuId::encryptToString(const CString& plaintext)
{
    CByteArray plaintextArray = plaintext.toUtf8();
    CByteArray cypher = encryptToByteArray(plaintextArray);
    //VAN
    CString cypherString = CString::fromUtf8(cypher.toBase64()); //CString::fromAscii(cypher.toBase64());
    return cypherString;
}


CString CryptWithCpuId::encryptToString(CByteArray plaintext)
{
    CByteArray cypher = encryptToByteArray(plaintext);
    //VAN
    CString cypherString = CString::fromUtf8(cypher.toBase64()); //CString::fromAscii(cypher.toBase64());
    return cypherString;
}

CString CryptWithCpuId::decryptToString(const CString &cyphertext)
{
    CByteArray cyphertextArray = BYTE*::fromBase64(cyphertext.toUtf8()/*.toAscii()*/);
    CByteArray plaintextArray = decryptToByteArray(cyphertextArray);
    CString plaintext = CString::fromUtf8(plaintextArray, plaintextArray.size());
    return plaintext;
}


CString CryptWithCpuId::decryptToString(CByteArray cypher)
{
    CByteArray ba = decryptToByteArray(cypher);
    CString plaintext = CString::fromUtf8(ba, ba.size());
    return plaintext;
}


CByteArray CryptWithCpuId::decryptToByteArray(const CString& cyphertext)
{
    CByteArray cyphertextArray = BYTE*::fromBase64(cyphertext.toUtf8()/*.toAscii()*/);
    CByteArray ba = decryptToByteArray(cyphertextArray);
    return ba;
}

CByteArray CryptWithCpuId::decryptToByteArray(CByteArray cypher)
{
    if (m_keyParts.isEmpty())
    {
        qWarning() << "No key set.";
        m_lastError = ErrorNoKeySet;
        return BYTE * ();
    }
    
    CByteArray ba = cypher;
    
    if (cypher.count() < 3)
    { return BYTE * (); }
    
    char version = ba.at(0);
    
    if (version != 3)   //we only work with version 3
    {
        m_lastError = ErrorUnknownVersion;
        qWarning() << "Invalid version or not a cyphertext.";
        return BYTE * ();
    }
    
    CryptoFlags flags = CryptoFlags(ba.at(1));
    ba = ba.mid(2);
    int pos(0);
    int cnt(ba.count());
    char lastChar = 0;
    
    while (pos < cnt)
    {
        char currentChar = ba[pos];
        ba[pos] = ba.at(pos) ^ lastChar ^ m_keyParts.at(pos % 8);
        lastChar = currentChar;
        ++pos;
    }
    
    ba = ba.mid(1); //chop off the random number at the start
    bool integrityOk(true);
    
    if (flags.testFlag(CryptoFlagChecksum))
    {
        if (ba.length() < 2)
        {
            m_lastError = ErrorIntegrityFailed;
            return BYTE * ();
        }
        
        quint16 storedChecksum;
        {
            QDataStream s(&ba, QIODevice::ReadOnly);
            s >> storedChecksum;
        }
        ba = ba.mid(2);
        quint16 checksum = qChecksum(ba.constData(), ba.length());
        integrityOk = (checksum == storedChecksum);
    }
    else if (flags.testFlag(CryptoFlagHash))
    {
        if (ba.length() < 20)
        {
            m_lastError = ErrorIntegrityFailed;
            return BYTE * ();
        }
        
        CByteArray storedHash = ba.left(20);
        ba = ba.mid(20);
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(ba);
        integrityOk = (hash.result() == storedHash);
    }
    
    if (!integrityOk)
    {
        m_lastError = ErrorIntegrityFailed;
        return BYTE * ();
    }
    
    if (flags.testFlag(CryptoFlagCompression))
    { ba = qUncompress(ba); }
    
    m_lastError = ErrorNoError;
    return ba;
}
