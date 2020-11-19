/* Portions of this file are subject to the following copyright(s).  See
* the Net-SNMP's COPYING file for more details and other copyrights
* that may apply:
*/
/*
* Portions of this file are copyrighted by:
* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
* Use is subject to license terms specified in the COPYING file
* distributed with the Net-SNMP package.
*/

/*
* scapi.c
*
*/

#include <net-snmp-config.h>
#include <net-snmp-features.h>

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <types.h>
#include <output_api.h>
#include <utilities.h>

netsnmp_feature_child_of(usm_support, libnetsnmp)
netsnmp_feature_child_of(usm_scapi, usm_support)

#ifndef NETSNMP_FEATURE_REMOVE_USM_SCAPI

#ifdef NETSNMP_USE_INTERNAL_MD5
#include <library/md5.h>
#endif
#include <library/snmp_api.h>
#include <library/callback.h>
#include <library/snmp_secmod.h>
#include <library/keytools.h>
#include <library/scapi.h>
#include <library/mib.h>
#include <library/transform_oids.h>

#ifdef QUITFUN
#undef QUITFUN
#define QUITFUN(e, l)					\
	if (e != SNMPERR_SUCCESS) {			\
		rval = SNMPERR_SC_GENERAL_FAILURE;	\
		goto l ;				\
			}
#endif

/*
* sc_get_properlength(oid *hashtype, u_int hashtype_len):
*
* Given a hashing type ("hashtype" and its length hashtype_len), return
* the length of the hash result.
*
* Returns either the length or SNMPERR_GENERR for an unknown hashing type.
*/
int
sc_get_properlength(const oid * hashtype, u_int hashtype_len)
{
	DEBUGTRACE;
	/*
	* Determine transform type hash length.
	*/
#ifndef NETSNMP_DISABLE_MD5
	if (ISTRANSFORM(hashtype, HMACMD5Auth)) {
		return BYTESIZE(SNMP_TRANS_AUTHLEN_HMACMD5);
	}
	else
#endif
		if (ISTRANSFORM(hashtype, HMACSHA1Auth)) {
			return BYTESIZE(SNMP_TRANS_AUTHLEN_HMACSHA1);
		}
	return SNMPERR_GENERR;
}

netsnmp_feature_child_of(scapi_get_proper_priv_length, netsnmp_unused)
#ifndef NETSNMP_FEATURE_REMOVE_SCAPI_GET_PROPER_PRIV_LENGTH
int
sc_get_proper_priv_length(const oid * privtype, u_int privtype_len)
{
	int properlength = 0;
#ifndef NETSNMP_DISABLE_DES
	if (ISTRANSFORM(privtype, DESPriv)) {
		properlength = BYTESIZE(SNMP_TRANS_PRIVLEN_1DES);
	}
#endif
	return properlength;
}
#endif /* NETSNMP_FEATURE_REMOVE_SCAPI_GET_PROPER_PRIV_LENGTH */


/*******************************************************************-o-******
* sc_init
*
* Returns:
*	SNMPERR_SUCCESS			Success.
*/
int
sc_init(void)
{
	int             rval = SNMPERR_SUCCESS;

	struct timeval  tv;

	DEBUGTRACE;

	gettimeofday(&tv, (struct timezone *) 0);

	srandom((unsigned)(tv.tv_sec ^ tv.tv_usec));
	return rval;
}                               /* end sc_init() */

/*******************************************************************-o-******
* sc_random
*
* Parameters:
*	*buf		Pre-allocated buffer.
*	*buflen 	Size of buffer.
*
* Returns:
*	SNMPERR_SUCCESS			Success.
*/
int
sc_random(u_char * buf, size_t * buflen)
{
	int             rval = SNMPERR_SUCCESS;
	int             i;
	int             rndval;
	u_char         *ucp = buf;

	DEBUGTRACE;
	/*
	* fill the buffer with random integers.  Note that random()
	* is defined in config.h and may not be truly the random()
	* system call if something better existed
	*/
	rval = *buflen - *buflen % sizeof(rndval);
	for (i = 0; i < rval; i += sizeof(rndval)) {
		rndval = random();
		memcpy(ucp, &rndval, sizeof(rndval));
		ucp += sizeof(rndval);
	}

	rndval = random();
	memcpy(ucp, &rndval, *buflen % sizeof(rndval));

	rval = SNMPERR_SUCCESS;
	return rval;
}                               /* end sc_random() */
/*******************************************************************-o-******
* sc_generate_keyed_hash
*
* Parameters:
*	 authtype	Type of authentication transform.
*	 authtypelen
*	*key		Pointer to key (Kul) to use in keyed hash.
*	 keylen		Length of key in bytes.
*	*message	Pointer to the message to hash.
*	 msglen		Length of the message.
*	*MAC		Will be returned with allocated bytes containg hash.
*	*maclen		Length of the hash buffer in bytes; also indicates
*				whether the MAC should be truncated.
*
* Returns:
*	SNMPERR_SUCCESS			Success.
*	SNMPERR_GENERR			All errs
*
*
* A hash of the first msglen bytes of message using a keyed hash defined
* by authtype is created and stored in MAC.  MAC is ASSUMED to be a buffer
* of at least maclen bytes.  If the length of the hash is greater than
* maclen, it is truncated to fit the buffer.  If the length of the hash is
* less than maclen, maclen set to the number of hash bytes generated.
*
* ASSUMED that the number of hash bits is a multiple of 8.
*/
int
sc_generate_keyed_hash(const oid * authtype, size_t authtypelen,
const u_char * key, u_int keylen,
const u_char * message, u_int msglen,
u_char * MAC, size_t * maclen)
#if  defined(NETSNMP_USE_INTERNAL_MD5) || defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_PKCS11) || defined(NETSNMP_USE_INTERNAL_CRYPTO)
{
	int             rval = SNMPERR_SUCCESS;
	int             iproperlength;
	size_t          properlength;

	u_char          buf[SNMP_MAXBUF_SMALL];
#if  defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_PKCS11)
#endif

	DEBUGTRACE;

	/*
	* Sanity check.
	*/
	if (!authtype || !key || !message || !MAC || !maclen
		|| (keylen <= 0) || (msglen <= 0) || (*maclen <= 0)
		|| (authtypelen != USM_LENGTH_OID_TRANSFORM)) {
		QUITFUN(SNMPERR_GENERR, sc_generate_keyed_hash_quit);
	}

	iproperlength = sc_get_properlength(authtype, authtypelen);
	if (iproperlength == SNMPERR_GENERR)
		return SNMPERR_GENERR;
	properlength = (size_t)iproperlength;
	if (keylen < properlength) {
		QUITFUN(SNMPERR_GENERR, sc_generate_keyed_hash_quit);
	}

	if (*maclen > properlength)
		*maclen = properlength;
	if (MDsign(message, msglen, MAC, *maclen, key, keylen)) {
		rval = SNMPERR_GENERR;
		goto sc_generate_keyed_hash_quit;
	}

sc_generate_keyed_hash_quit:
	memset(buf, 0, SNMP_MAXBUF_SMALL);
	return rval;
}

#endif
/*
* sc_hash(): a generic wrapper around whatever hashing package we are using.
*
* IN:
* hashtype    - oid pointer to a hash type
* hashtypelen - length of oid pointer
* buf         - u_char buffer to be hashed
* buf_len     - integer length of buf data
* MAC_len     - length of the passed MAC buffer size.
*
* OUT:
* MAC         - pre-malloced space to store hash output.
* MAC_len     - length of MAC output to the MAC buffer.
*
* Returns:
* SNMPERR_SUCCESS              Success.
* SNMP_SC_GENERAL_FAILURE      Any error.
* SNMPERR_SC_NOT_CONFIGURED    Hash type not supported.
*/
int
sc_hash(const oid * hashtype, size_t hashtypelen, const u_char * buf,
size_t buf_len, u_char * MAC, size_t * MAC_len)
#if defined(NETSNMP_USE_INTERNAL_MD5) || defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_PKCS11) || defined(NETSNMP_USE_INTERNAL_CRYPTO)
{
#if defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_PKCS11) || defined(NETSNMP_USE_INTERNAL_CRYPTO)
	int            rval = SNMPERR_SUCCESS;
#endif
#if defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_PKCS11)
	unsigned int   tmp_len;
#endif
	int            ret;

	DEBUGTRACE;

	if (hashtype == NULL || buf == NULL || buf_len <= 0 ||
		MAC == NULL || MAC_len == NULL)
		return (SNMPERR_GENERR);
	ret = sc_get_properlength(hashtype, hashtypelen);
	if ((ret < 0) || (*MAC_len < (size_t)ret))
		return (SNMPERR_GENERR);

	if (MDchecksum(buf, buf_len, MAC, *MAC_len)) {
		return SNMPERR_GENERR;
	}
	if (*MAC_len > 16)
		*MAC_len = 16;
	return SNMPERR_SUCCESS;

}
#endif                          /* !defined(NETSNMP_USE_OPENSSL) && !defined(NETSNMP_USE_INTERNAL_MD5) */
/*******************************************************************-o-******
* sc_check_keyed_hash
*
* Parameters:
*	 authtype	Transform type of authentication hash.
*	*key		Key bits in a string of bytes.
*	 keylen		Length of key in bytes.
*	*message	Message for which to check the hash.
*	 msglen		Length of message.
*	*MAC		Given hash.
*	 maclen		Length of given hash; indicates truncation if it is
*				shorter than the normal size of output for
*				given hash transform.
* Returns:
*	SNMPERR_SUCCESS		Success.
*	SNMP_SC_GENERAL_FAILURE	Any error
*
*
* Check the hash given in MAC against the hash of message.  If the length
* of MAC is less than the length of the transform hash output, only maclen
* bytes are compared.  The length of MAC cannot be greater than the
* length of the hash transform output.
*/
int
sc_check_keyed_hash(const oid * authtype, size_t authtypelen,
const u_char * key, u_int keylen,
const u_char * message, u_int msglen,
const u_char * MAC, u_int maclen)
#if defined(NETSNMP_USE_INTERNAL_MD5) || defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_PKCS11) || defined(NETSNMP_USE_INTERNAL_CRYPTO)
{
	int             rval = SNMPERR_SUCCESS;
	size_t          buf_len = SNMP_MAXBUF_SMALL;

	u_char          buf[SNMP_MAXBUF_SMALL];

	DEBUGTRACE;

	/*
	* Sanity check.
	*/
	if (!authtype || !key || !message || !MAC
		|| (keylen <= 0) || (msglen <= 0) || (maclen <= 0)
		|| (authtypelen != USM_LENGTH_OID_TRANSFORM)) {
		QUITFUN(SNMPERR_GENERR, sc_check_keyed_hash_quit);
	}

	/*
	* Generate a full hash of the message, then compare
	* the result with the given MAC which may shorter than
	* the full hash length.
	*/
	rval = sc_generate_keyed_hash(authtype, authtypelen,
		key, keylen,
		message, msglen, buf, &buf_len);
	QUITFUN(rval, sc_check_keyed_hash_quit);

	if (maclen > msglen) {
		QUITFUN(SNMPERR_GENERR, sc_check_keyed_hash_quit);

	}
	else if (memcmp(buf, MAC, maclen) != 0) {
		QUITFUN(SNMPERR_GENERR, sc_check_keyed_hash_quit);
	}


sc_check_keyed_hash_quit:
	memset(buf, 0, SNMP_MAXBUF_SMALL);

	return rval;

}                               /* end sc_check_keyed_hash() */
#endif                          /* NETSNMP_USE_INTERNAL_MD5 */
/*******************************************************************-o-******
* sc_encrypt
*
* Parameters:
*	 privtype	Type of privacy cryptographic transform.
*	*key		Key bits for crypting.
*	 keylen		Length of key (buffer) in bytes.
*	*iv		IV bits for crypting.
*	 ivlen		Length of iv (buffer) in bytes.
*	*plaintext	Plaintext to crypt.
*	 ptlen		Length of plaintext.
*	*ciphertext	Ciphertext to crypt.
*	*ctlen		Length of ciphertext.
*
* Returns:
*	SNMPERR_SUCCESS			Success.
*	SNMPERR_SC_NOT_CONFIGURED	Encryption is not supported.
*	SNMPERR_SC_GENERAL_FAILURE	Any other error
*
*
* Encrypt plaintext into ciphertext using key and iv.
*
* ctlen contains actual number of crypted bytes in ciphertext upon
* successful return.
*/
int
sc_encrypt(const oid * privtype, size_t privtypelen,
u_char * key, u_int keylen,
u_char * iv, u_int ivlen,
const u_char * plaintext, u_int ptlen,
u_char * ciphertext, size_t * ctlen)
{
#	if NETSNMP_USE_INTERNAL_MD5
	{
		snmp_log(LOG_ERR, "Encryption support not enabled.\n");
		DEBUGMSGTL(("scapi", "Encrypt function not defined.\n"));
		return SNMPERR_SC_GENERAL_FAILURE;
	}
}
#endif                          /* */



/*******************************************************************-o-******
* sc_decrypt
*
* Parameters:
*	 privtype
*	*key
*	 keylen
*	*iv
*	 ivlen
*	*ciphertext
*	 ctlen
*	*plaintext
*	*ptlen
*
* Returns:
*	SNMPERR_SUCCESS			Success.
*	SNMPERR_SC_NOT_CONFIGURED	Encryption is not supported.
*      SNMPERR_SC_GENERAL_FAILURE      Any other error
*
*
* Decrypt ciphertext into plaintext using key and iv.
*
* ptlen contains actual number of plaintext bytes in plaintext upon
* successful return.
*/
int
sc_decrypt(const oid * privtype, size_t privtypelen,
u_char * key, u_int keylen,
u_char * iv, u_int ivlen,
u_char * ciphertext, u_int ctlen,
u_char * plaintext, size_t * ptlen)
{
#if NETSNMP_USE_INTERNAL_MD5
	{
		DEBUGMSGTL(("scapi", "Decryption function not defined.\n"));
		return SNMPERR_SC_GENERAL_FAILURE;
	}
#endif                      /*  */
}                        /* NETSNMP_USE_OPENSSL */

#endif /*  NETSNMP_FEATURE_REMOVE_USM_SCAPI  */
