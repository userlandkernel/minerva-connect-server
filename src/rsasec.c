#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <assert.h>

void MVCSLogRSAError(const char* function, unsigned long error) {
	char* buffer = malloc(1024+1);
	if(!buffer) {
		return;
	}
	// Initialize buffer and copy error
	bzero(buffer, 1024+1);
	ERR_error_string_n(error, buffer, 1024);

	// Print the error
	fprintf(stderr, "%s: %s\n", function ? function : "Error", buffer);

	// Cleanup
	free(buffer);
	buffer = NULL;
}


RSA* MVCSLoadPubkey(const char* data) {
	RSA *rsa = NULL;
	BIO *keybio = NULL;
	keybio = BIO_new_mem_buf((void*)data, -1);
	if (!keybio) {
		fprintf(stderr, "%s: Could not create BIO from data.\n", __func__);
		return NULL;
	}
	rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);

	unsigned long err = ERR_get_error();
	if(err) {
		MVCSLogRSAError(__func__, err);
	}

	return rsa;
}


bool MVCSVerifySignatureInternal(RSA* rsa, unsigned char* msgHash, size_t msgHashLen, const char* msg, size_t msgLen, bool* isAuthentic) {
	if(!isAuthentic) {
		// Invalid argument
		fprintf(stderr, "%s: Invalid argument\n", __func__);
		return false;
	}

	EVP_PKEY* publicKey = EVP_PKEY_new();
	EVP_PKEY_assign_RSA(publicKey, rsa);
	EVP_MD_CTX* mRSAVerifyCtx = EVP_MD_CTX_create();

	// Initialize digest verification
	if (EVP_DigestVerifyInit(mRSAVerifyCtx, NULL, EVP_sha256(), NULL, publicKey) <= 0) {
		fprintf(stderr, "%s: Failed to initialize digest verification\n", __func__);
		return false; // Failed to initialize digest verification
	}

	// Update digest verification
	if( EVP_DigestVerifyUpdate(mRSAVerifyCtx, msg, msgLen) <= 0) {
		fprintf(stderr, "%s: Failed to update digest verification\n", __func__);
		return false;
	}


	// Perform actual digest verification
	int authStatus = EVP_DigestVerifyFinal(mRSAVerifyCtx, msgHash, msgHashLen);

	// Debug
	printf("DIGEST: ");
	for(int i = 0; i < msgHashLen; i++) {
		printf("%02x", msgHash[i]);
	}
	printf("\n");

	if (authStatus == 1) {
		*isAuthentic = true;
		EVP_MD_CTX_free(mRSAVerifyCtx); // Clean up the context again
		return true; // Verification succeed, it is authentic
	}
	else if (!authStatus) {
		*isAuthentic = false;
		EVP_MD_CTX_free(mRSAVerifyCtx); // Clean up the context again
		return true; // Verification succeeded, but its not authentic
	}
	else {
		*isAuthentic = false;
		EVP_MD_CTX_free(mRSAVerifyCtx); // Cleanup the context again
		return false; // Verification failed, neather it is authentic
	}
}

void MVCSDecodeMessageB64(const char* msgBase64, unsigned char** buffer, size_t* length) {
	size_t inputLen = strlen(msgBase64);
	*length = (3*inputLen)/4;
	*buffer = (unsigned char*)calloc(*length+1, 1);
	bzero(*buffer, *length);
	size_t ol = EVP_DecodeBlock(*buffer, (unsigned char*)msgBase64, inputLen);
	if(*length != ol) {
		fprintf(stderr, "%s: Expected decode len %zu but got %zu", __func__, *length, ol);
	}
}

/*void _MVCSDecodeMessageB64(const char* msgBase64, unsigned char** buffer, size_t* length) {
	BIO* bio = NULL;
	BIO* b64 = NULL;

	if(!length || !buffer || ! msgBase64) {
		return; // Invalid arguments
	}

	// Calculate the length for the decoded buffer
	int decodeLen = MVCSCalcDecodeLength(msgBase64);
	if(!decodeLen) {
	 	*length = 0;
		fprintf(stderr, "%s: Failed to get the decode length of the message\n", __func__);
		return;
	}

	// Allocate decoded buffer
	*buffer = (unsigned char*)malloc(decodeLen + 1);
	bzero(*buffer, decodeLen);

	bio = BIO_new_mem_buf(msgBase64, -1);
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_push(b64, bio);

	if(!bio) {
		fprintf(stderr, "%s: Failed to construct a BIO from the message\n", __func__);
		*length = 0;
		return;
	}

	*length = BIO_read(bio, *buffer, strlen(msgBase64));
	if(*length == 0) {
		fprintf(stderr, "%s: BIO_read returned a zero-length.\n", __func__);
	}
	BIO_free_all(bio);
}*/



bool MVCSVerifySignature(char* publicKey, char* plainText, char *signatureBase64) {
	RSA* publicKeyRSA = MVCSLoadPubkey((const char*)publicKey);

	if(!publicKeyRSA) {
		fprintf(stderr, "%s: Could not load public key\n", __func__);
		return false;
	}

	unsigned char* encMessage = NULL;
	size_t encMessageLen = 0;
	bool isAuthentic = false;

	// decode the message
	MVCSDecodeMessageB64(signatureBase64, &encMessage, &encMessageLen);
	if(!encMessageLen) {
		fprintf(stderr, "%s: failed to decode the message\n", __func__);
		// Failed to decode
		return false;
	}

	bool result = MVCSVerifySignatureInternal(publicKeyRSA,encMessage, encMessageLen, plainText, strlen(plainText), &isAuthentic);
	return result & isAuthentic;
}

bool MVCSAuthenticate(char *authMessage) {
	// For each public key in the trust store, check signature
	return false;
}
