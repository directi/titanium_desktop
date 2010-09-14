/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "diffiehellman_binding.h"

#include <string>
#include <sstream>
#include <map>
#include <vector>

#include <openssl/dh.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/rand.h>
#include <openssl/err.h>


#define DH_PRIME_BIT_SIZE 1024
#define DH_GENERATE_MAX_TRY 3

//source of all prime numbers org.metastatic.jessie.provider
//URL: http://www.koders.com/java/fidF08F6FC6066B798166C5C5B78D936DCA4ED20996.aspx

#define DH_PRIME_GROUP_1 "00"\
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1"\
    "29024E088A67CC74020BBEA63B139B22514A08798E3404DD"\
    "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245"\
    "E485B576625E7EC6F44C42E9A63A3620FFFFFFFFFFFFFFFF"

#define DH_PRIME_GROUP_2 "00"\
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1" \
    "29024E088A67CC74020BBEA63B139B22514A08798E3404DD" \
    "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245" \
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED" \
    "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE65381" \
    "FFFFFFFFFFFFFFFF"

#define DH_PRIME_GROUP_5 "00" \
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1" \
    "29024E088A67CC74020BBEA63B139B22514A08798E3404DD" \
    "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245" \
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED" \
    "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D" \
    "C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F" \
    "83655D23DCA3AD961C62F356208552BB9ED529077096966D" \
    "670C354E4ABC9804F1746C08CA237327FFFFFFFFFFFFFFFF"

#define DH_PRIME_GROUP_14 "00" \
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1" \
    "29024E088A67CC74020BBEA63B139B22514A08798E3404DD" \
    "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245" \
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED" \
    "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D" \
    "C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F" \
    "83655D23DCA3AD961C62F356208552BB9ED529077096966D" \
    "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B" \
    "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9" \
    "DE2BCBF6955817183995497CEA956AE515D2261898FA0510" \
    "15728E5A8AACAA68FFFFFFFFFFFFFFFF"


#define DH_PRIME_GROUP_15 "00" \
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1" \
    "29024E088A67CC74020BBEA63B139B22514A08798E3404DD" \
    "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245" \
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED" \
    "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D" \
    "C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F" \
    "83655D23DCA3AD961C62F356208552BB9ED529077096966D" \
    "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B" \
    "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9" \
    "DE2BCBF6955817183995497CEA956AE515D2261898FA0510" \
    "15728E5A8AAAC42DAD33170D04507A33A85521ABDF1CBA64" \
    "ECFB850458DBEF0A8AEA71575D060C7DB3970F85A6E1E4C7" \
    "ABF5AE8CDB0933D71E8C94E04A25619DCEE3D2261AD2EE6B" \
    "F12FFA06D98A0864D87602733EC86A64521F2B18177B200C" \
    "BBE117577A615D6C770988C0BAD946E208E24FA074E5AB31" \
    "43DB5BFCE0FD108E4B82D120A93AD2CAFFFFFFFFFFFFFFFF"

#define DH_PRIME_GROUP_16 "00" \
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1" \
    "29024E088A67CC74020BBEA63B139B22514A08798E3404DD" \
    "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245" \
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED" \
    "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D" \
    "C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F" \
    "83655D23DCA3AD961C62F356208552BB9ED529077096966D" \
    "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B" \
    "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9" \
    "DE2BCBF6955817183995497CEA956AE515D2261898FA0510" \
    "15728E5A8AAAC42DAD33170D04507A33A85521ABDF1CBA64" \
    "ECFB850458DBEF0A8AEA71575D060C7DB3970F85A6E1E4C7" \
    "ABF5AE8CDB0933D71E8C94E04A25619DCEE3D2261AD2EE6B" \
    "F12FFA06D98A0864D87602733EC86A64521F2B18177B200C" \
    "BBE117577A615D6C770988C0BAD946E208E24FA074E5AB31" \
    "43DB5BFCE0FD108E4B82D120A92108011A723C12A787E6D7" \
    "88719A10BDBA5B2699C327186AF4E23C1A946834B6150BDA" \
    "2583E9CA2AD44CE8DBBBC2DB04DE8EF92E8EFC141FBECAA6" \
    "287C59474E6BC05D99B2964FA090C3A2233BA186515BE7ED" \
    "1F612970CEE2D7AFB81BDD762170481CD0069127D5B05AA9" \
    "93B4EA988D8FDDC186FFB7DC90A6C08F4DF435C934063199" \
    "FFFFFFFFFFFFFFFF"

#define DH_PRIME_GROUP_17 "00" \
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E08" \
    "8A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B" \
    "302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9" \
    "A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE6" \
    "49286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8" \
    "FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D" \
    "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C" \
    "180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF695581718" \
    "3995497CEA956AE515D2261898FA051015728E5A8AAAC42DAD33170D" \
    "04507A33A85521ABDF1CBA64ECFB850458DBEF0A8AEA71575D060C7D" \
    "B3970F85A6E1E4C7ABF5AE8CDB0933D71E8C94E04A25619DCEE3D226" \
    "1AD2EE6BF12FFA06D98A0864D87602733EC86A64521F2B18177B200C" \
    "BBE117577A615D6C770988C0BAD946E208E24FA074E5AB3143DB5BFC" \
    "E0FD108E4B82D120A92108011A723C12A787E6D788719A10BDBA5B26" \
    "99C327186AF4E23C1A946834B6150BDA2583E9CA2AD44CE8DBBBC2DB" \
    "04DE8EF92E8EFC141FBECAA6287C59474E6BC05D99B2964FA090C3A2" \
    "233BA186515BE7ED1F612970CEE2D7AFB81BDD762170481CD0069127" \
    "D5B05AA993B4EA988D8FDDC186FFB7DC90A6C08F4DF435C934028492" \
    "36C3FAB4D27C7026C1D4DCB2602646DEC9751E763DBA37BDF8FF9406" \
    "AD9E530EE5DB382F413001AEB06A53ED9027D831179727B0865A8918" \
    "DA3EDBEBCF9B14ED44CE6CBACED4BB1BDB7F1447E6CC254B33205151" \
    "2BD7AF426FB8F401378CD2BF5983CA01C64B92ECF032EA15D1721D03" \
    "F482D7CE6E74FEF6D55E702F46980C82B5A84031900B1C9E59E7C97F" \
    "BEC7E8F323A97A7E36CC88BE0F1D45B7FF585AC54BD407B22B4154AA" \
    "CC8F6D7EBF48E1D814CC5ED20F8037E0A79715EEF29BE32806A1D58B" \
    "B7C5DA76F550AA3D8A1FBFF0EB19CCB1A313D55CDA56C9EC2EF29632" \
    "387FE8D76E3C0468043E8F663F4860EE12BF2D5B0B7474D6E694F91E" \
    "6DCC4024FFFFFFFFFFFFFFFF"


#define DH_PRIME_GROUP_18 "00" \
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1" \
    "29024E088A67CC74020BBEA63B139B22514A08798E3404DD" \
    "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245" \
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED" \
    "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D" \
    "C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F" \
    "83655D23DCA3AD961C62F356208552BB9ED529077096966D" \
    "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B" \
    "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9" \
    "DE2BCBF6955817183995497CEA956AE515D2261898FA0510" \
    "15728E5A8AAAC42DAD33170D04507A33A85521ABDF1CBA64" \
    "ECFB850458DBEF0A8AEA71575D060C7DB3970F85A6E1E4C7" \
    "ABF5AE8CDB0933D71E8C94E04A25619DCEE3D2261AD2EE6B" \
    "F12FFA06D98A0864D87602733EC86A64521F2B18177B200C" \
    "BBE117577A615D6C770988C0BAD946E208E24FA074E5AB31" \
    "43DB5BFCE0FD108E4B82D120A92108011A723C12A787E6D7" \
    "88719A10BDBA5B2699C327186AF4E23C1A946834B6150BDA" \
    "2583E9CA2AD44CE8DBBBC2DB04DE8EF92E8EFC141FBECAA6" \
    "287C59474E6BC05D99B2964FA090C3A2233BA186515BE7ED" \
    "1F612970CEE2D7AFB81BDD762170481CD0069127D5B05AA9" \
    "93B4EA988D8FDDC186FFB7DC90A6C08F4DF435C934028492" \
    "36C3FAB4D27C7026C1D4DCB2602646DEC9751E763DBA37BD" \
    "F8FF9406AD9E530EE5DB382F413001AEB06A53ED9027D831" \
    "179727B0865A8918DA3EDBEBCF9B14ED44CE6CBACED4BB1B" \
    "DB7F1447E6CC254B332051512BD7AF426FB8F401378CD2BF" \
    "5983CA01C64B92ECF032EA15D1721D03F482D7CE6E74FEF6" \
    "D55E702F46980C82B5A84031900B1C9E59E7C97FBEC7E8F3" \
    "23A97A7E36CC88BE0F1D45B7FF585AC54BD407B22B4154AA" \
    "CC8F6D7EBF48E1D814CC5ED20F8037E0A79715EEF29BE328" \
    "06A1D58BB7C5DA76F550AA3D8A1FBFF0EB19CCB1A313D55C" \
    "DA56C9EC2EF29632387FE8D76E3C0468043E8F663F4860EE" \
    "12BF2D5B0B7474D6E694F91E6DBE115974A3926F12FEE5E4" \
    "38777CB6A932DF8CD8BEC4D073B931BA3BC832B68D9DD300" \
    "741FA7BF8AFC47ED2576F6936BA424663AAB639C5AE4F568" \
    "3423B4742BF1C978238F16CBE39D652DE3FDB8BEFC848AD9" \
    "22222E04A4037C0713EB57A81A23F0C73473FC646CEA306B" \
    "4BCBC8862F8385DDFA9D4B7FA2C087E879683303ED5BDD3A" \
    "062B3CF5B3A278A66D2A13F83F44F82DDF310EE074AB6A36" \
    "4597E899A0255DC164F31CC50846851DF9AB48195DED7EA1" \
    "B1D510BD7EE74D73FAF36BC31ECFA268359046F4EB879F92" \
    "4009438B481C6CD7889A002ED5EE382BC9190DA6FC026E47" \
    "9558E4475677E9AA9E3050E2765694DFC81F56E880B96E71" \
    "60C980DD98EDD3DFFFFFFFFFFFFFFFFF"

#define INITIATOR_CIPHER_KEY "INITIATOR_CIPHER_KEY"
#define INITIATOR_MAC_KEY "INITIATOR_MAC_KEY"
#define INITIATOR_SIGMA_KEY "INITIATOR_SIGMA_KEY"
#define RESPONDER_CIPHER_KEY "RESPONDER_CIPHER_KEY"
#define RESPONDER_MAC_KEY "RESPONDER_MAC_KEY"
#define RESPONDER_SIGMA_KEY "RESPONDER_SIGMA_KEY"

namespace ti {

    DiffieHellmanBinding::DiffieHellmanBinding() :
    KAccessorObject("DiffieHellman.DH"),
    selfNonce(NULL),
    peerNonce(NULL),
    selfEncryptionKey(NULL),
    selfIntegrityKey(NULL),
    selfSigmaKey(NULL),
    peerEncryptionKey(NULL),
    peerIntegrityKey(NULL),
    peerSigmaKey(NULL),
    aesNum(0) {
        /**
         * @tiapi(method = True, name = DiffieHellman.getPublicKey, since=1.0.0) returns public key for a given generator
         * @tiarg(for = DiffieHellman.getPublicKey, name = g, type=int) the group parameter g
         * @tiarg(for = DiffieHellman.getPublicKey, name = asBase64, type=Boolean, optional=True) whether to encode public key as base64
         * @tiresult(for = DiffieHellman.getPublicKey, type = String) returns the generated public key
         */
        this->SetMethod("getPublicKey", &DiffieHellmanBinding::GetPublicKey);

        /**
         * @tiapi(method = True, name = DiffieHellman.getSelfNonce, since=1.0.0) generates a cryptographic random nonce
         * @tiresult(for = DiffieHellman.getSelfNonce, type = String) returns self nonce
         */
        this->SetMethod("getSelfNonce", &DiffieHellmanBinding::GetSelfNonce);

        /**
         * @tiapi(method = True, name = DiffieHellman.getPeerNonce, since=1.0.0) returns the value of peer's nonce.
         * @tiapi Peer's nonce must have already been explicitly set (null is returned if it's not so)
         * @tiresult(for = DiffieHellman.getPeerNonce, type = String) returns the peer nonce
         */
        this->SetMethod("getPeerNonce", &DiffieHellmanBinding::GetPeerNonce);

        /**
         * @tiapi(method = True, name = DiffieHellman.setSelfNonce, since=1.0.0) sets the value of peer's nonce
         * @tiarg(for = DiffieHellman.setSelfNonce, name = peerCounter, type=String) the nonce of the peer as base64 encode string
         */
        this->SetMethod("setPeerNonce", &DiffieHellmanBinding::SetPeerNonce);

        /**
         * @tiapi(method = True, name = DiffieHellman.getBlockCounter, since=1.0.0) generates a cryptographic random block counter for AES-CTR
         * @tiresult(for = DiffieHellman.getNonce, type = String) returns the generated or cached counter
         */
        this->SetMethod("getBlockCounter", &DiffieHellmanBinding::GetBlockCounter);

        /**
         * @tiapi(method = True, name = DiffieHellman.setBlockCounter, since=1.0.0)
         * @tiapi sets the block counter for AES-CTR as
         * @tiapi peerCounter * exp(2, n -1) where peerCounter is the
         * @tiapi provided counter of the peer
         * @tiarg(for = DiffieHellman.setBlockCounter, name = peerCounter, type=String) the counter of the peer
         */
        this->SetMethod("setBlockCounter", &DiffieHellmanBinding::SetBlockCounter);
        /**
         * @tiapi(method = True, name = DiffieHellman.generateKeys, since=1.0.0) generates the shared keys
         * @tiarg(for = DiffieHellman.generateKeys, name = g, type=int) the MODp group generator
         * @tiarg(for = DiffieHellman.generateKeys, name = peerPublicKey, type=String) the public key of the peer
         * @tiarg(for = DiffieHellman.generateKeys, name = isHex, type=Boolean, optional=True) whether the provided peer public key is in hex form
         * @tiresult(for = DiffieHellman.generateKeys, type = String) returns the generated nonce
         */
        this->SetMethod("generateKeys", &DiffieHellmanBinding::GenerateKeys);


        /**
         * @tiapi(method = True, name = DiffieHellman.verifyPublicKeyRange, since=1.0.0) verifies if the given BIGNUM is a correct public key for the
         * @tiapi group specified (i.e. 1 &lt; d &gt; p - 1)
         * @tiarg(for = DiffieHellman.verifyPublicKeyRange, name = peerPublicKey, type=String) the public key to test
         * @tiarg(for = DiffieHellman.verifyPublicKeyRange, name = g, type=int) the MODp group generator
         * @tiresult(for = DiffieHellman.verifyPublicKeyRange, type = String) true if the test passes, false otherwise
         */
        this->SetMethod("verifyPublicKeyRange", &DiffieHellmanBinding::VerifyPublicKeyRange);

        /**
         * @tiapi(method = True, name = DiffieHellman.generateRandStringList, since=1.0.0) generates a list of random number of strings each of random size
         * @tiarg(for = DiffieHellman.generateRandStringList, name = minCount, type=int) minimum length of generated
         * @tiarg(for = DiffieHellman.generateRandStringList, name = maxCount, type=int) maximum length of generated
         * @tiarg(for = DiffieHellman.generateRandStringList, name = minSize, type=int) minimum length of each generated string
         * @tiarg(for = DiffieHellman.generateRandStringList, name = maxSize, type=int) maximum length of each generated string
         * @tiresult(for = DiffieHellman.generateRandStringList, type = String) a list of generated strings
         */
        this->SetMethod("generateRandStringList", &DiffieHellmanBinding::GenerateRandStringList);

        this->SetMethod("getInitiatorIdentityMAC", &DiffieHellmanBinding::GetInitiatorIdentityMAC);

        this->SetMethod("verifyPublicKeyHash", &DiffieHellmanBinding::VerifyPublicKeyHash);

        this->SetMethod("validateIdentity", &DiffieHellmanBinding::ValidateIdentity);

        this->SetMethod("hmacSHA256", &DiffieHellmanBinding::HMACSHA256);

        this->SetMethod("sha256Hash", &DiffieHellmanBinding::SHA256Hash);

        this->SetMethod("concatBase64", &DiffieHellmanBinding::ConcatBase64);
        
        this->SetMethod("generateSharedKey", &DiffieHellmanBinding::GenerateSharedKey);

        //@TODO: cleanup in case of exception

        if (!RAND_bytes(this->selfAESCounter, this->CIPHER_BLOCK_SIZE)) {
            //@TODO: get error using Error_get_error
            throw "error in generating AES counter";
        }
        memcpy(this->peerAESCounter, BlockCounterXOR(this->selfAESCounter, this->CIPHER_BLOCK_SIZE), this->CIPHER_BLOCK_SIZE);

        this->selfNonce = GenerateRand(this->NONCE_BYTES * 8);
        if (!this->selfNonce) {
            throw "error in generating nonce";
        }

        memset(this->aesECountBuf, 0, CIPHER_BLOCK_SIZE);
        this->aesNum = 0;
    }

    DiffieHellmanBinding::~DiffieHellmanBinding() {
        if (this->selfNonce) {
            BN_clear_free(this->selfNonce);
        }
        if (this->peerNonce) {
            BN_clear_free(this->peerNonce);
        }
        if (this->selfEncryptionKey) {
            free(this->selfEncryptionKey);
        }
        if (this->selfIntegrityKey) {
            free(this->selfIntegrityKey);
        }
        if (this->selfSigmaKey) {
            free(this->selfSigmaKey);
        }
        if (this->peerEncryptionKey) {
            free(this->peerEncryptionKey);
        }
        if (this->peerIntegrityKey) {
            free(this->peerIntegrityKey);
        }
        if (this->peerSigmaKey) {
            free(this->peerSigmaKey);
        }

        std::map<int, std::pair < DH*, bool> >::iterator it = this->dhMap.begin();
        for (; it != this->dhMap.end(); ++it) {
            if ((it->second).first) {
                DH_free((it->second).first);
            }
            this->dhMap.erase(it);
        }
    }

    static Logger* GetLogger() {
        static Logger* logger = Logger::Get("DiffieHellman");
        return logger;
    }

    static DH * GenerateP(int g, bool useKnownPrimeIfAvailable, bool *usedKnownPrime) {
        const char* primeAsHex = NULL;
        DH *dh = NULL;

        *usedKnownPrime = false;

        if (useKnownPrimeIfAvailable) {
            switch (g) {
                case 1:
                    primeAsHex = DH_PRIME_GROUP_1;
                    break;
                case 2:
                    primeAsHex = DH_PRIME_GROUP_2;
                    break;
                case 5:
                    primeAsHex = DH_PRIME_GROUP_5;
                    break;
                case 14:
                    primeAsHex = DH_PRIME_GROUP_14;
                    break;
                case 15:
                    primeAsHex = DH_PRIME_GROUP_15;
                    break;
                case 16:
                    primeAsHex = DH_PRIME_GROUP_16;
                    break;
                case 17:
                    primeAsHex = DH_PRIME_GROUP_17;
                    break;
                case 18:
                    primeAsHex = DH_PRIME_GROUP_18;
                    break;
                default:
                    primeAsHex = NULL;
                    break;
            }
        }
        if (primeAsHex) {
            dh = DH_new();
            if (!dh) {
                throw kroll::ValueException::FromString(
                        "DiffieHellmanBinding::generateDH "
                        "DH_generate_parameters failed");
            }

            BIGNUM* gBN = BN_new();
            BN_set_word(gBN, g);
            dh->g = gBN;

            BIGNUM* pBN = BN_new();
            BN_hex2bn(&pBN, primeAsHex);
            dh->p = pBN;

            *usedKnownPrime = true;

        } else {

            int retry_count = 0;

            while (retry_count++ < DH_GENERATE_MAX_TRY) {

                dh = DH_generate_parameters(DH_PRIME_BIT_SIZE, g, NULL, NULL);
                if (!dh) {
                    throw kroll::ValueException::FromString(
                            "DiffieHellmanBinding::generateDH "
                            "DH_generate_parameters failed");
                }

                int codes = 0;
                if (!DH_check(dh, &codes)) {
                    throw kroll::ValueException::FromString(
                            "DiffieHellmanBinding::generateDH "
                            "DH_check failed");
                }

                if (codes & DH_CHECK_P_NOT_PRIME) {
                    GetLogger()->Critical("DH_CHECK_P_NOT_PRIME failure");
                } else if (codes & DH_CHECK_P_NOT_SAFE_PRIME) {
                    GetLogger()->Critical("DH_CHECK_P_NOT_SAFE_PRIME "
                            "failure");
                } else if (codes & DH_NOT_SUITABLE_GENERATOR) {
                    GetLogger()->Critical("DH_NOT_SUITABLE_GENERATOR "
                            "failure");
                } else if (codes & DH_UNABLE_TO_CHECK_GENERATOR) {
                    GetLogger()->Critical("DH_UNABLE_TO_CHECK_GENERATOR "
                            "failure");
                }

                if (!codes) {
                    break;
                }
            }

            if (retry_count >= DH_GENERATE_MAX_TRY) {
                throw kroll::ValueException::FromString(
                        "DiffieHellmanBinding::generateDH "
                        "Could not generat keys after max re-tries");
            }

        }


        return dh;

    }

    bool DiffieHellmanBinding::GenerateDH(int g, bool useKnownPrimeIfAvailable) {

        bool usedKnownPrime = false;

        DH *dh = GenerateP(g, useKnownPrimeIfAvailable, &usedKnownPrime);

        if (!DH_generate_key(dh)) {
            throw kroll::ValueException::FromString(
                    "DiffieHellmanBinding::generateDH "
                    "DH_generate_key failed");
        }

        this->dhMap[g] = std::pair < DH*, bool>(dh, usedKnownPrime);

        return usedKnownPrime;
    }

    static unsigned char* SHA256(unsigned char *in, int len) {

        unsigned char *hash = (unsigned char*)malloc(SHA256_DIGEST_LENGTH);

        SHA256_CTX sha256;
        SHA256_Init(&sha256);

        SHA256_Update(&sha256, in, len);
        SHA256_Final(hash, &sha256);

        return hash;
    }

    /*
     * http://www.ioncannon.net/programming/34/howto-base64-encode-with-cc-and-openssl/
     */
    static char* Base64Encode(const unsigned char *input, int length) {
        BIO *bmem, *b64;
        BUF_MEM *bptr;

        b64 = BIO_new(BIO_f_base64());
        bmem = BIO_new(BIO_s_mem());//BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        b64 = BIO_push(b64, bmem);
        BIO_write(b64, input, length);
        BIO_flush(b64);
        BIO_get_mem_ptr(b64, &bptr);

        char *buff = (char *) malloc(bptr->length);
        memcpy(buff, bptr->data, bptr->length - 1);
        buff[bptr->length - 1] = 0;

        BIO_free_all(b64);

        return buff;
    }

    /*
     * http://www.ioncannon.net/programming/122/howto-base64-decode-with-cc-and-openssl/
     */
    static int Base64Decode(const char *input, unsigned char ** out) {
        BIO *b64, *bmem;

        int length = strlen(input);

        char* inputBuf[length];
        memcpy(inputBuf, input, length);

        *out = (unsigned char *)malloc(length);
        memset(*out, 0, length);

        b64 = BIO_new(BIO_f_base64());
        if(length < 64) {
            BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        }
        bmem = BIO_new_mem_buf(inputBuf, length);
        bmem = BIO_push(b64, bmem);

        int readBytes = BIO_read(bmem, *out, length);

        BIO_free_all(bmem);

        return readBytes;
    }

    static char* BignumToString(BIGNUM *bignum, bool doHashSHA256) {
        //@TODO: possible memleak (OPENSSL_Free might be needed)
        //http://openssl.org/docs/crypto/BN_bn2bin.html
        int len = BN_bn2mpi(bignum, NULL);

        unsigned char *bignumStr = (unsigned char *) malloc(len);
        BN_bn2mpi(bignum, bignumStr);

        if(doHashSHA256) {
            //@TODO: mem leak!
            bignumStr = SHA256(bignumStr, len);
            len = SHA256_DIGEST_LENGTH;
        }
        return Base64Encode(bignumStr, len);
    }

    static char* BignumToString(BIGNUM *bignum) {
        return BignumToString(bignum, false);
    }

    static BIGNUM *BignumFromString(const char *str) {

        unsigned char* decodedStr;
        int len = Base64Decode(str, &decodedStr);

        BIGNUM *rv = BN_new();

        if (!BN_mpi2bn(decodedStr, len, rv)) {
            //@TODO: get error by ERR_get_error
            printf("DiffieHellmanBinding::BignumFromString "
                    "error in de-serializing mpi string to BIGNUM: %s\n", ERR_error_string(ERR_get_error(), NULL));
        }

        return rv;
    }

    void DiffieHellmanBinding::GetPublicKey(const ValueList& args, KValueRef result) {
        args.VerifyException("getPublicKey", "i ?b");

        int g = args.GetInt(0);
        int doSHA256 = args.GetBool(1, false);

        GetLogger()->Debug("Getting Public key for: %d", g);

        if (this->dhMap.find(g) == this->dhMap.end()) {
            GetLogger()->Debug("No cached Public key for: %d", g);
            this->GenerateDH(g, true);
        }

        std::pair < DH*, bool> savedDH = this->dhMap[g];

        GetLogger()->Debug("cached Public Key DH (%d): %u", g, savedDH.first);

        DH *dh = savedDH.first;
        //bool usedKnownPrime = savedDH.second;

        BIGNUM *pubKey = dh->pub_key;

        GetLogger()->Debug("cached Public Key (%d): %u", g, pubKey);
        
        std::string rv = std::string(BignumToString(pubKey, doSHA256));
        GetLogger()->Debug("Public key(%d): %s", g, rv.c_str());
        result->SetString(rv);
    }

    BIGNUM * DiffieHellmanBinding::GenerateRand(int bits) {
        GetLogger()->Debug("About to generate random bigint of %d bits", bits);

        BIGNUM* rand = BN_new();
        if (!rand) {
            throw kroll::ValueException::FromString(
                    "DiffieHellmanBinding::getNonce "
                    "creation of new BIGNUM failed");
        }

        GetLogger()->Debug("Able to create empty bigint");

        if (!BN_rand(rand, bits, -1, 0)) {
            //@TODO: get the error using ERR_get_error
            throw kroll::ValueException::FromString(
                    "DiffieHellmanBinding::getNonce "
                    "Error in generating random nonce via BN_rand");
        }

        if (!rand) {
            throw kroll::ValueException::FromString(
                    "DiffieHellmanBinding::getNonce "
                    "rand not generated (is NULL)");
        }

        return rand;
    }

    char* DiffieHellmanBinding::GenerateRandString(int bytes) {
        return BignumToString(GenerateRand(bytes * 8));
    }

    void DiffieHellmanBinding::GetSelfNonce(const ValueList& args, KValueRef result) {
        char *rvCStr = BignumToString(this->selfNonce);
        if (!rvCStr) {
            throw kroll::ValueException::FromString(
                    "DiffieHellmanBinding::geSelftNonce "
                    "conversion of nonce to string failed");
        }
        GetLogger()->Debug("conversion of nonce to string %s", rvCStr);

        std::string rv(rvCStr);
        GetLogger()->Debug("DiffieHellmanBinding::getNonce string is %s",
                rvCStr);

        result->SetString(rv);

    }

    void DiffieHellmanBinding::SetPeerNonce(const ValueList& args, KValueRef result) {
        args.VerifyException("setPeerNonce", "s");

        std::string peerNonceStr = args.GetString(0);
        // bignum will always be base64 encoded
        this->peerNonce = BignumFromString(peerNonceStr.c_str());
        if (!this->peerNonce) {
            throw kroll::ValueException::FromString(
                    "DiffieHellmanBinding::setPeerNonce "
                    "conversion of string to bignum failed");
        }

    }

    void DiffieHellmanBinding::GetPeerNonce(const ValueList& args, KValueRef result) {
        if (!this->peerNonce) {
            GetLogger()->Debug("peer nonce not set, getter will return null");
            result->SetObject(NULL);
            return;
        }

        char *rvCStr = BignumToString(this->peerNonce);
        if (!rvCStr) {
            throw kroll::ValueException::FromString(
                    "DiffieHellmanBinding::getPeerNonce "
                    "conversion of nonce to string failed");
        }
        GetLogger()->Debug("conversion of peer nonce to string %s", rvCStr);

        std::string rv(rvCStr);
        GetLogger()->Debug("DiffieHellmanBinding::getPeerNonce string is %s",
                rvCStr);

        result->SetString(rv);

    }

    void DiffieHellmanBinding::GetBlockCounter(const ValueList& args, KValueRef result) {
        char* cp = Base64Encode(this->selfAESCounter, this->CIPHER_BLOCK_SIZE);

        std::string rv(cp);
        result->SetString(rv);

    }

    unsigned char *DiffieHellmanBinding::BlockCounterXOR(const unsigned char *otherCounter, int n) {

        //@TODO: consider using BN_GF2m_add
        unsigned char *rv = (unsigned char*)malloc(n);
        
        BN_CTX *ctx = BN_CTX_new();
        BIGNUM *one = BN_CTX_get(ctx);
        BN_one(one);

        BIGNUM *two = BN_CTX_get(ctx);
        BN_set_word(two, 2);

        BIGNUM *nMinusOne = BN_CTX_get(ctx);
        BN_set_word(nMinusOne, (n * 8) - 1);

        BIGNUM *xorFactor = BN_CTX_get(ctx);
        BN_exp(xorFactor, two, nMinusOne, ctx);

        unsigned char *xorFactorBin = (unsigned char*) malloc(BN_num_bytes(xorFactor));
        BN_bn2bin(xorFactor, xorFactorBin);
        
        for (int i = 0; i<n; i++) {
            rv[i] = xorFactorBin[i] ^ otherCounter[i];
        }

        free(xorFactorBin);

        BN_CTX_end(ctx);
        BN_CTX_free(ctx);

        return rv;
    }

    void DiffieHellmanBinding::SetBlockCounter(const ValueList& args, KValueRef result) {
        args.VerifyException("setBlockCounter", "s");

        std::string peerCounter = args.GetString(0);
        unsigned char * peerCounterBytes;
        Base64Decode(peerCounter.c_str(), &peerCounterBytes);

        unsigned char *buffer = BlockCounterXOR(peerCounterBytes, this->CIPHER_BLOCK_SIZE);
        memcpy(this->selfAESCounter, buffer, this->CIPHER_BLOCK_SIZE);
        memcpy(this->peerAESCounter, peerCounterBytes, this->CIPHER_BLOCK_SIZE);

        free(buffer);
        free(peerCounterBytes);
    }

    void DiffieHellmanBinding::GenerateSharedKey(const ValueList& args, KValueRef result) {

        args.VerifyException("generateSharedKey", "i s");

        int g = args.GetInt(0);

        std::string otherPublicKeyStr = args.GetString(1);

        BIGNUM *otherPublicKey = BignumFromString(otherPublicKeyStr.c_str());

        DH *dh = this->dhMap[g].first;

        unsigned int sharedScretLen = DH_size(dh);
        unsigned char* sharedSecret = (unsigned char*) malloc(sharedScretLen);

        if (DH_compute_key(sharedSecret, otherPublicKey, dh) < 0) {
            //@TODO: get the error using ERR_get_error
            throw kroll::ValueException::FromString(
                    "DiffieHellmanBinding::GenerateSharedSecret "
                    "DH_compute_key failed");
        }

        std::string rv(Base64Encode(sharedSecret, sharedScretLen));
        result->SetString(rv);

    }

    void DiffieHellmanBinding::GenerateKeys(const ValueList& args, KValueRef result) {

        args.VerifyException("generateKeys", "s");

        std::string sharedSecretUC = args.GetString(0);

        unsigned char* sharedSecret;

        unsigned int sharedScretLen = Base64Decode(sharedSecretUC.c_str(), &sharedSecret);
        

        if (this->selfEncryptionKey) {
            free(this->selfEncryptionKey);
        }
        this->selfEncryptionKey = (unsigned char*) malloc(16);

        if (this->selfIntegrityKey) {
            free(this->selfIntegrityKey);
        }
        this->selfIntegrityKey = (unsigned char*) malloc(16);

        if (this->selfSigmaKey) {
            free(this->selfSigmaKey);
        }
        this->selfSigmaKey = (unsigned char*) malloc(16);

        if (this->peerEncryptionKey) {
            free(this->peerEncryptionKey);
        }
        this->peerEncryptionKey = (unsigned char*) malloc(16);

        if (this->peerIntegrityKey) {
            free(this->peerIntegrityKey);
        }
        this->peerIntegrityKey = (unsigned char*) malloc(16);

        if (this->peerSigmaKey) {
            free(this->peerSigmaKey);
        }
        this->peerSigmaKey = (unsigned char*) malloc(16);

        //we need only the least significant 128 bits of the SHA256 hash
        unsigned char *mac = HMAC(EVP_sha256(), INITIATOR_CIPHER_KEY,
                strlen(INITIATOR_CIPHER_KEY), sharedSecret, sharedScretLen,
                NULL, NULL);
        memcpy(this->selfEncryptionKey, mac, 16);

        mac = HMAC(EVP_sha256(), INITIATOR_MAC_KEY,
                strlen(INITIATOR_MAC_KEY), sharedSecret, sharedScretLen,
                NULL, NULL);
        memcpy(this->selfIntegrityKey, mac, 16);

        mac = HMAC(EVP_sha256(), INITIATOR_SIGMA_KEY,
                strlen(INITIATOR_SIGMA_KEY), sharedSecret, sharedScretLen,
                NULL, NULL);
        memcpy(this->selfSigmaKey, mac, 16);

        mac = HMAC(EVP_sha256(), RESPONDER_CIPHER_KEY,
                strlen(RESPONDER_CIPHER_KEY), sharedSecret, sharedScretLen,
                NULL, NULL);
        memcpy(this->peerEncryptionKey, mac, 16);

        mac = HMAC(EVP_sha256(), RESPONDER_MAC_KEY,
                strlen(RESPONDER_MAC_KEY), sharedSecret, sharedScretLen,
                NULL, NULL);
        memcpy(this->peerIntegrityKey, mac, 16);

        mac = HMAC(EVP_sha256(), RESPONDER_SIGMA_KEY,
                strlen(RESPONDER_SIGMA_KEY), sharedSecret, sharedScretLen,
                NULL, NULL);
        memcpy(this->peerSigmaKey, mac, 16);

        //this a copy of our stored DH structs and has shared key stored
        //with it, we can safely destroy it with harming the saved instances
        //DH_free(dh);

    }

    void DiffieHellmanBinding::VerifyPublicKeyRange(const ValueList& args, KValueRef result) {
        args.VerifyException("verifyPublicKeyRange", "s i");

        std::string peerKey = args.GetString(0);
        int g = args.GetInt(1);

        if (this->dhMap.find(g) == this->dhMap.end()) {
            GetLogger()->Debug("No cached Public key for: %d", g);
            this->GenerateDH(g, true);
        }

        bool rv = true;

        BIGNUM *peerKeyBignum = BignumFromString(peerKey.c_str());

        const BIGNUM *one = BN_value_one();

        if (BN_cmp(peerKeyBignum, one) != 1) {
            GetLogger()->Debug("inocrrect peer public key (< 1) for group %d", g);
            rv = false;
        }
        else {
            DH *dh = this->dhMap[g].first;
            BIGNUM *upperDiff = BN_new();
            BN_sub(upperDiff, dh->p, peerKeyBignum);
            if (BN_cmp(upperDiff, one) != 1) {
                GetLogger()->Debug("inocrrect peer public key (> p-1) for group %d", g);
                BN_clear_free(upperDiff);
                rv = false;
            }
        }

        BN_clear_free(peerKeyBignum);

        result->SetBool(rv);
    }

    static int RandRange(int min, int max) {
        BIGNUM *rvBN = BN_new();
        BIGNUM *range = BN_new();
        if (!BN_set_word(range, max - min)) {
            throw "error in setting BN word value";
        }
        if (!BN_rand_range(rvBN, range)) {
            throw "error in generating random bignumber";
        }

        unsigned int rvInt = BN_get_word(rvBN);
        return rvInt + min;

    }

    void DiffieHellmanBinding::GenerateRandStringList(const ValueList& args, KValueRef result) {
        args.VerifyException("generateRandStringList", "i i i i");

        int minCount = args.GetInt(0);
        int maxCount = args.GetInt(1);
        int minBytes = args.GetInt(2);
        int maxBytes = args.GetInt(3);

        std::vector<std::string> rv;

        //@TODO: use BN_CTX here: too many BN's might be created in the loop
        int count = RandRange(minCount, maxCount);
        for (int i = 0; i < count; i++) {
            int size = RandRange(minBytes, maxBytes);
            char* randCStr = GenerateRandString(size);
            rv.push_back(std::string(randCStr));
        }

        result->SetList(StaticBoundList::FromStringVector(rv));
    }

    void DiffieHellmanBinding::GetInitiatorIdentityMAC(const ValueList& args, KValueRef result) {
        args.VerifyException("getInitiatorIdentityMAC", "i s s");

        if(!this->peerNonce) {
            throw kroll::ValueException::FromString(
                    "DiffieHellmanBinding::getInitiatorIdentityMAC "
                    "Illegal State: peer nonce should be set");
        }

        int g = args.GetInt(0);
        if (this->dhMap.find(g) == this->dhMap.end()) {
            GetLogger()->Debug("No cached Public key for: %d", g);
            this->GenerateDH(g, true);
        }

        //for the nomenclature refer to XEP 0217
        std::string formA = args.GetString(1);
        std::string formA2 = args.GetString(2);
        
        std::string concatForm = formA + formA2;
        int concatFormLen = concatForm.size();

        BIGNUM *e = ((this->dhMap[g]).first)->pub_key;
        
        int eLen = BN_bn2mpi(e, NULL);

        unsigned char *eStr = (unsigned char*) malloc(eLen);
        BN_bn2mpi(e, eStr);

        unsigned char *peerNonceUC = (unsigned char*)malloc(BN_bn2mpi(this->peerNonce, NULL));
        int peerNonceLen = BN_bn2mpi(this->peerNonce, peerNonceUC);

        unsigned char *selfNonceUC = (unsigned char*)malloc(BN_bn2mpi(this->selfNonce, NULL));
        int selfNonceLen = BN_bn2mpi(this->selfNonce, selfNonceUC);

        unsigned int dataSize = peerNonceLen + selfNonceLen + eLen + concatFormLen;
 
        unsigned char macData[dataSize];
        memcpy(macData, peerNonceUC, peerNonceLen);
        memcpy(macData + peerNonceLen, selfNonceUC, selfNonceLen);
        memcpy(macData + peerNonceLen + selfNonceLen, eStr, eLen);
        memcpy(macData + peerNonceLen + selfNonceLen + eLen, concatForm.c_str(), concatFormLen);

        
        unsigned char mac[EVP_MAX_MD_SIZE];
        unsigned int macLen;
        
        HMAC(EVP_sha256(), this->selfSigmaKey,
                16, macData,
                dataSize, mac, &macLen);

        AES_KEY aesKey;
        if (AES_set_encrypt_key(this->selfEncryptionKey, 128, &aesKey)) {
            //@TODO: get the error using ERR_get_error
            throw kroll::ValueException::FromFormat(
                    "DiffieHellmanBinding::GetInitiatorIdentityMAC "
                    "AES_set_encrypt_key failed: %s", ERR_error_string(ERR_get_error(), NULL));
        }

        unsigned char aesCounterCopy[CIPHER_BLOCK_SIZE];
        memcpy(aesCounterCopy, this->selfAESCounter, CIPHER_BLOCK_SIZE);

        unsigned char encrypted[macLen];
        AES_ctr128_encrypt(mac, encrypted, macLen, &aesKey, this->selfAESCounter, this->aesECountBuf, &(this->aesNum));

        GetLogger()->Debug("Generating ID: block counter: %s", Base64Encode(aesCounterCopy, CIPHER_BLOCK_SIZE));

        //concat cA + idA
        unsigned char macMacData[macLen + CIPHER_BLOCK_SIZE];
        memcpy(macMacData, aesCounterCopy, CIPHER_BLOCK_SIZE);
        memcpy(macMacData + CIPHER_BLOCK_SIZE, encrypted, macLen);

        unsigned char macMac[EVP_MAX_MD_SIZE];
        unsigned int macMacLen;
        
        GetLogger()->Debug("Generating ID: final mac input length: %d", macLen + CIPHER_BLOCK_SIZE);
        GetLogger()->Debug("Generating ID: final mac input: %s", Base64Encode(macMacData, macLen + CIPHER_BLOCK_SIZE));
        GetLogger()->Debug("Generating ID: final mac key: %s", Base64Encode(this->selfIntegrityKey, 16));

        HMAC(EVP_sha256(), this->selfIntegrityKey,
                16, macMacData,
                macLen + CIPHER_BLOCK_SIZE, macMac, &macMacLen);

        GetLogger()->Debug("Generating ID: final mac output length: %d", macMacLen);
        GetLogger()->Debug("Generating ID: final mac output: %s", Base64Encode(macMac, macMacLen));
        
        std::string encryptedRV(Base64Encode(encrypted, macLen));
        std::string macRV(Base64Encode(macMac, macMacLen));

        std::vector<string> rvVec;
        rvVec.push_back(encryptedRV);
        rvVec.push_back(macRV);
        
        result->SetList(StaticBoundList::FromStringVector(rvVec));
    }

    void DiffieHellmanBinding::VerifyPublicKeyHash(const ValueList& args, KValueRef result) {
        //publicKey(=base64(k')), publicKeyHash(=base64(SHA256(k)))
        args.VerifyException("verifyPublicKeyHash", "s s");

        std::string publicKeyb64 = args.GetString(0);
        std::string publicKeyHash = args.GetString(1);

        unsigned char* publicKey;
        int len = Base64Decode(publicKeyb64.c_str(), &publicKey);

        char *newHash = Base64Encode(SHA256(publicKey, len), SHA256_DIGEST_LENGTH);

        bool rv = publicKeyHash.compare(newHash) == 0;
        
        result->SetBool(rv);
    }

    void DiffieHellmanBinding::ValidateIdentity(const ValueList& args, KValueRef result) {
        //idA, mA
        args.VerifyException("validateIdentity", "s s s s s");

        std::string idAB64 = args.GetString(0);
        std::string mAB64 = args.GetString(1);

        unsigned char *idA;
        int idALen = Base64Decode(idAB64.c_str(), &idA);

        GetLogger()->Debug("Verifying ID: block counter: %s", Base64Encode(this->peerAESCounter, CIPHER_BLOCK_SIZE));

        //concat cA + idA
        unsigned char macData[CIPHER_BLOCK_SIZE + idALen];
        memcpy(macData, this->peerAESCounter, CIPHER_BLOCK_SIZE);
        memcpy(macData + CIPHER_BLOCK_SIZE, idA, idALen);

        unsigned char mac[EVP_MAX_MD_SIZE];
        unsigned int macLen;

        GetLogger()->Debug("Verifying ID: final mac input length: %d", idALen + CIPHER_BLOCK_SIZE);
        GetLogger()->Debug("Verifying ID: final mac input: %s", Base64Encode(macData, idALen + CIPHER_BLOCK_SIZE));
        GetLogger()->Debug("Verifying ID: final mac key: %s", Base64Encode(this->selfIntegrityKey, 16));

        HMAC(EVP_sha256(), this->selfIntegrityKey,
                16, macData,
                CIPHER_BLOCK_SIZE + idALen, mac, &macLen);

        GetLogger()->Debug("Verifying ID: final mac output length: %d", macLen);
        GetLogger()->Debug("Verifying ID: final mac output: %s", Base64Encode(mac, macLen));

        char *macCStr = Base64Encode(mac, macLen);

        bool rv = mAB64.compare(macCStr) == 0;

        if(!rv) {
            GetLogger()->Debug("identity check failed at first check\n");
            result->SetBool(rv);
            return;
        }

        AES_KEY aesKey;
        if (AES_set_encrypt_key(this->selfEncryptionKey, 128, &aesKey)) {
            throw kroll::ValueException::FromFormat(
                    "DiffieHellmanBinding::DecryptIdentity "
                    "AES_set_encrypt_key failed: %s", ERR_error_string(ERR_get_error(), NULL));
        }
        
        unsigned char *decrypted = (unsigned char*)malloc(idALen);
        
        AES_ctr128_encrypt(idA, decrypted, idALen, &aesKey, this->peerAESCounter, this->aesECountBuf, &(this->aesNum));
        
        std::string formA = args.GetString(2);
        std::string formA2 = args.GetString(3);
        std::string e = args.GetString(4);

        std::string concatForm = formA + formA2;
        int concatFormLen = concatForm.size();

        unsigned char *eStr;
        int eLen = Base64Decode(e.c_str(), &eStr);

        unsigned char *peerNonceUC = (unsigned char*)malloc(BN_bn2mpi(this->peerNonce, NULL));
        int peerNonceLen = BN_bn2mpi(this->peerNonce, peerNonceUC);

        unsigned char *selfNonceUC = (unsigned char*)malloc(BN_bn2mpi(this->selfNonce, NULL));
        int selfNonceLen = BN_bn2mpi(this->selfNonce, selfNonceUC);

        unsigned int dataSize = selfNonceLen + peerNonceLen + eLen + concatFormLen;

        unsigned char macDecData[dataSize];
        memcpy(macDecData, selfNonceUC, selfNonceLen);
        memcpy(macDecData + selfNonceLen, peerNonceUC, peerNonceLen);
        memcpy(macDecData + selfNonceLen + peerNonceLen, eStr, eLen);
        memcpy(macDecData + selfNonceLen + peerNonceLen + eLen, concatForm.c_str(), concatFormLen);
        
        unsigned char macDec[EVP_MAX_MD_SIZE];
        unsigned int macDecLen;

        HMAC(EVP_sha256(), this->selfSigmaKey,
                16, macDecData,
                dataSize, macDec, &macDecLen);

        rv = true;
        
        for(unsigned int i=0; i<macDecLen; i++) {
            if(macDec[i] != decrypted[i]) {
                rv = false;
                break;
            }
        }
        
        if(!rv) {
            GetLogger()->Debug("match failed for decrypted mac");
        }
        else {
            GetLogger()->Debug("match succeded for identity");
        }

        result->SetBool(rv);
    }

    void DiffieHellmanBinding::HMACSHA256(const ValueList& args, KValueRef result) {
        args.VerifyException("hmacSHA256", "s s");

        unsigned char *key;
        int keyLen = Base64Decode(args.GetString(0).c_str(), &key);

        unsigned char *data;
        int dataLen = Base64Decode(args.GetString(1).c_str(), &data);
        
        unsigned char mac[EVP_MAX_MD_SIZE];
        unsigned int macLen;

        HMAC(EVP_sha256(), key,
                keyLen, data,
                dataLen, mac, &macLen);

        std::string rv(Base64Encode(mac, macLen));
        result->SetString(rv);
    }

    void DiffieHellmanBinding::SHA256Hash(const ValueList& args, KValueRef result) {
        args.VerifyException("sha256Hash", "s");

        unsigned char *in;
        int len = Base64Decode(args.GetString(0).c_str(), &in);

        unsigned char *digest = SHA256(in, len);

        std::string rv(Base64Encode(digest, SHA256_DIGEST_LENGTH));

        result->SetString(rv);
    }

    void DiffieHellmanBinding::ConcatBase64(const ValueList& args, KValueRef result) {

        std::vector<std::pair<unsigned char*, int> > bufferVector;
        int total = 0;

        for(unsigned int i = 0; i< args.size(); i++) {
            if(!args.at(i)->IsString()) continue;
            unsigned char *in;
            int len = Base64Decode(args.GetString(i).c_str(), &in);
            total += len;
            bufferVector.push_back(std::pair<unsigned char*, int>(in, len));
        }

        unsigned char rvBuffer[total];
        int index = 0;
        for(unsigned int i=0; i<bufferVector.size(); i++) {
            std::pair<unsigned char*, int> p = bufferVector.at(i);
            memcpy(rvBuffer + index, p.first, p.second);
            index += p.second;
            free(p.first);
        }

        std::string rv(Base64Encode(rvBuffer, total));
        result->SetString(rv);
    }

}
