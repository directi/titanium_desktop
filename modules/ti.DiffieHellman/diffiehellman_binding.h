/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _DIFFIEHELLMAN_BINDING_H_
#define _DIFFIEHELLMAN_BINDING_H_

#include <kroll/kroll.h>

#include <openssl/dh.h>

#include <map>
#include <utility>


namespace ti
{
	class DiffieHellmanBinding : public KAccessorObject
	{
	public:
		DiffieHellmanBinding();
	protected:
		virtual ~DiffieHellmanBinding();
	private:
                bool GenerateDH(int g, bool useKnownPrimeIfAvailable);
                void GetPublicKey(const ValueList& args, KValueRef result);
                static BIGNUM * GenerateRand(int bits);
                static char* GenerateRandString(int bytes);
                static unsigned char* BlockCounterXOR(const unsigned char *otherCounter, int n);
                void GetSelfNonce(const ValueList& args, KValueRef result);
                void SetPeerNonce(const ValueList& args, KValueRef result);
                void GetPeerNonce(const ValueList& args, KValueRef result);
                void GetBlockCounter(const ValueList& args, KValueRef result);
                void SetBlockCounter(const ValueList& args, KValueRef result);
                void GenerateKeys(const ValueList& args, KValueRef result);
                void VerifyPublicKeyRange(const ValueList& args, KValueRef result);
                void VerifyPublicKeyHash(const ValueList& args, KValueRef result);
                void GenerateRandStringList(const ValueList& args, KValueRef result);
                void GetInitiatorIdentityMAC(const ValueList& args, KValueRef result);
                void ValidateIdentity(const ValueList& args, KValueRef result);
                void HMACSHA256(const ValueList& args, KValueRef result);
                void GenerateSharedKey(const ValueList& args, KValueRef result);
                void SHA256Hash(const ValueList& args, KValueRef result);
                void ConcatBase64(const ValueList& args, KValueRef result);

                static const int CIPHER_BLOCK_SIZE = 16;
                static const int NONCE_BYTES = 16;
                
                std::map<int, std::pair<DH*, bool> > dhMap;

                BIGNUM *selfNonce;
                BIGNUM *peerNonce;

                unsigned char *selfEncryptionKey;
                unsigned char *selfIntegrityKey;
                unsigned char *selfSigmaKey;

                unsigned char *peerEncryptionKey;
                unsigned char *peerIntegrityKey;
                unsigned char *peerSigmaKey;

                unsigned char selfAESCounter[CIPHER_BLOCK_SIZE];
                unsigned char peerAESCounter[CIPHER_BLOCK_SIZE];

                unsigned char aesECountBuf[CIPHER_BLOCK_SIZE];
                unsigned int aesNum;
	};
        
}

#endif
