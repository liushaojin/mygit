#ifndef CRYPTWITHCPUID_H
#define CRYPTWITHCPUID_H
//#include <stdafx.h>
#include "../stdafx.h"


class CryptWithCpuId
{
    public:
        enum CompressionMode
        {
            CompressionAuto,    /*!< Only apply compression if that results in a shorter plaintext. */
            CompressionAlways,  /*!< Always apply compression. Note that for short inputs, a compression may result in longer data */
            CompressionNever    /*!< Never apply compression. */
        };
        
        enum IntegrityProtectionMode
        {
            ProtectionNone,    /*!< The integerity of the encrypted data is not protected. It is not really possible to detect a wrong key, for instance. */
            ProtectionChecksum,/*!< A simple checksum is used to verify that the data is in order. If not, an empty string is returned. */
            ProtectionHash     /*!< A cryptographic hash is used to verify the integrity of the data. This method produces a much stronger, but longer check */
        };
        
        enum Error
        {
            ErrorNoError,         /*!< No error occurred. */
            ErrorNoKeySet,        /*!< No key was set. You can not encrypt or decrypt without a valid key. */
            ErrorUnknownVersion,  /*!< The version of this data is unknown, or the data is otherwise not valid. */
            ErrorIntegrityFailed, /*!< The integrity check of the data failed. Perhaps the wrong key was used. */
        };
        
        enum CryptoFlag
        {
            CryptoFlagNone = 0,
            CryptoFlagCompression = 0x01,
            CryptoFlagChecksum = 0x02,
            CryptoFlagHash = 0x04
        };
        
        
        CryptWithCpuId();   
        explicit CryptWithCpuId(DWORD key);

        void setKey(DWORD key);
		void splitKey();

        bool hasKey() const
        {
            return !m_keyParts.empty();
        }
        void setCompressionMode(CompressionMode mode)
        {
            m_compressionMode = mode;
        }
        CompressionMode compressionMode() const
        {
            return m_compressionMode;
        }
        void setIntegrityProtectionMode(IntegrityProtectionMode mode)
        {
            m_protectionMode = mode;
        }
        IntegrityProtectionMode integrityProtectionMode() const
        {
            return m_protectionMode;
        }
        Error lastError() const
        {
            return m_lastError;
        }
        
        CString encryptToString(const CString& plaintext) ;
        CString encryptToString(CByteArray plaintext) ;
        CString decryptToString(CByteArray cypher) ;
        CString decryptToString(const CString& cyphertext) ;
        
        CByteArray encryptToByteArray(const CString& plaintext) ;
        CByteArray encryptToByteArray(CByteArray plaintext) ;
        CByteArray decryptToByteArray(const CString& cyphertext) ;
        CByteArray decryptToByteArray(CByteArray cypher) ;
        
        
        
        
        
        
    private:
        void splitKey();
        DWORD m_key;
        vector<char> m_keyParts;
        CompressionMode m_compressionMode;
        IntegrityProtectionMode m_protectionMode;
        Error m_lastError;
};

#endif // CRYPTWITHCPUID_H