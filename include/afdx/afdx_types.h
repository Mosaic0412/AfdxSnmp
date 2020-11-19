#ifndef afdx_NET_SNMP_TYPES_H
#define afdx_NET_SNMP_TYPES_H

#define MAX_OID_LEN	    128

#ifdef __cplusplus
extern "C" {
#endif

	typedef union {
		long           *integer;
		unsigned char         *string;
		unsigned long            *objid;
		unsigned char         *bitstring;
		struct counter64 *counter64;
		float          *floatVal;
		double         *doubleVal;
	} afdx_netsnmp_vardata;


	typedef struct afdx_variable_list {
		/** NULL for last variable */
		struct variable_list *next_variable;
		/** Object identifier of variable */
		unsigned long            *name;
		/** number of subid's in name */
		unsigned int          name_length;
		/** ASN type of variable */
		unsigned char          type;
		/** value of variable */
		afdx_netsnmp_vardata val;
		/** the length of the value to be copied into buf */
		unsigned int          val_len;
		/** buffer to hold the OID */
		unsigned long             name_loc[MAX_OID_LEN];
		/** 90 percentile < 40. */
		unsigned char          buf[40];
		/** (Opaque) hook for additional data */
		void           *data;
		/** callback to free above */
		void(*dataFreeHook)(void *);
		int             index;
	} afdx_netsnmp_variable_list;

	typedef struct afdx_address_list {
		unsigned char DestinatinMac[17];
		unsigned char SourceMac[17];
		unsigned char DestinatinIP[15];
		unsigned char SourceIP[15];
		unsigned char InOid[MAX_OID_LEN];
	} afdx_netsnmp_address_list;

	typedef struct afdx_snmp_pdu {

#define non_repeaters	errstat
#define max_repetitions errindex

		/*
		* Protocol-version independent fields
		*/
		/** snmp version */
		long            version;
		/** Type of this PDU */
		int             command;
		/** Request id - note: not incremented on retries */
		long            reqid;
		/** Message id for V3 messages note: incremented for each retry */
		long            msgid;
		/** Unique ID for incoming transactions */
		long            transid;
		/** Session id for AgentX messages */
		long            sessid;
		/** Error status (non_repeaters in GetBulk) */
		long            errstat;
		/** Error index (max_repetitions in GetBulk) */
		long            errindex;
		/** Uptime */
		unsigned long          time;
		unsigned long          flags;

		int             securityModel;
		/** noAuthNoPriv, authNoPriv, authPriv */
		int             securityLevel;
		int             msgParseModel;

		/**
		* Transport-specific opaque data.  This replaces the IP-centric address
		* field.
		*/

		void           *transport_data;
		int             transport_data_length;

		/**
		* The actual transport domain.  This SHOULD NOT BE FREE()D.
		*/

		const unsigned long      *tDomain;
		unsigned int          tDomainLen;

		afdx_netsnmp_variable_list *variables;
		/*
		* SNMPv1 & SNMPv2c fields
		*/
		/** community for outgoing requests. */
		unsigned char         *community;
		/** length of community name. */
		unsigned int          community_len;

		/*
		* Trap information
		*/
		/** System OID */
		unsigned long            *enterprise;
		unsigned int          enterprise_length;
		/** trap type */
		long            trap_type;
		/** specific type */
		long            specific_type;
		/** This is ONLY used for v1 TRAPs  */
		unsigned char   agent_addr[4];

		/*
		*  SNMPv3 fields
		*/
		/** context snmpEngineID */
		unsigned char         *contextEngineID;
		/** Length of contextEngineID */
		unsigned int          contextEngineIDLen;
		/** authoritative contextName */
		char           *contextName;
		/** Length of contextName */
		unsigned int          contextNameLen;
		/** authoritative snmpEngineID for security */
		unsigned char         *securityEngineID;
		/** Length of securityEngineID */
		unsigned int          securityEngineIDLen;
		/** on behalf of this principal */
		char           *securityName;
		/** Length of securityName. */
		unsigned int          securityNameLen;

		/*
		* AgentX fields
		*      (also uses SNMPv1 community field)
		*/
		int             priority;
		int             range_subid;

		void           *securityStateRef;

		//address
		unsigned char DestinatinMac[17];
		unsigned char SourceMac[17];
		unsigned char DestinatinIP[15];
		unsigned char SourceIP[15];

	} afdx_netsnmp_pdu;

	struct afdx_snmp_session;
	typedef struct afdx_snmp_session afdx_netsnmp_session;

#define USM_AUTH_KU_LEN     32
#define USM_PRIV_KU_LEN     32

	typedef int(*afdx_snmp_callback) (int, afdx_netsnmp_session *, int,
		afdx_netsnmp_pdu *, void *);
	typedef int(*afdx_netsnmp_callback) (int, afdx_netsnmp_session *, int,
		afdx_netsnmp_pdu *, void *);

	struct afdx_netsnmp_container_s;

	struct afdx_snmp_session {
		/*
		* Protocol-version independent fields
		*/
		/** snmp version */
		long            version;
		/** Number of retries before timeout. */
		int             retries;
		/** Number of uS until first timeout, then exponential backoff */
		long            timeout;
		unsigned long          flags;
		struct afdx_snmp_session *subsession;
		struct afdx_snmp_session *next;

		/** name or address of default peer (may include transport specifier and/or port number) */
		char           *peername;
		/** UDP port number of peer. (NO LONGER USED - USE peername INSTEAD) */
		unsigned short         remote_port;
		/** My Domain name or dotted IP address, 0 for default */
		char           *localname;
		/** My UDP port number, 0 for default, picked randomly */
		unsigned short         local_port;
		/**
		* Authentication function or NULL if null authentication is used
		*/
		unsigned char         *(*authenticator) (unsigned char *, unsigned int *, unsigned char *, unsigned int);
		/** Function to interpret incoming data */
		afdx_netsnmp_callback callback;
		/**
		* Pointer to data that the callback function may consider important
		*/
		void           *callback_magic;
		/** copy of system errno */
		int             s_errno;
		/** copy of library errno */
		int             s_snmp_errno;
		/** Session id - AgentX only */
		long            sessid;

		/*
		* SNMPv1 & SNMPv2c fields
		*/
		/** community for outgoing requests. */
		unsigned char         *community;
		/** Length of community name. */
		unsigned int          community_len;
		/**  Largest message to try to receive.  */
		unsigned int          rcvMsgMaxSize;
		/**  Largest message to try to send.  */
		unsigned int          sndMsgMaxSize;

		/*
		* SNMPv3 fields
		*/
		/** are we the authoritative engine? */
		unsigned char          isAuthoritative;
		/** authoritative snmpEngineID */
		unsigned char         *contextEngineID;
		/** Length of contextEngineID */
		unsigned int          contextEngineIDLen;
		/** initial engineBoots for remote engine */
		unsigned int           engineBoots;
		/** initial engineTime for remote engine */
		unsigned int           engineTime;
		/** authoritative contextName */
		char           *contextName;
		/** Length of contextName */
		unsigned int          contextNameLen;
		/** authoritative snmpEngineID */
		unsigned char         *securityEngineID;
		/** Length of contextEngineID */
		unsigned int          securityEngineIDLen;
		/** on behalf of this principal */
		char           *securityName;
		/** Length of securityName. */
		unsigned int          securityNameLen;

		/** auth protocol oid */
		unsigned long            *securityAuthProto;
		/** Length of auth protocol oid */
		unsigned int          securityAuthProtoLen;
		/** Ku for auth protocol XXX */
		unsigned char          securityAuthKey[USM_AUTH_KU_LEN];
		/** Length of Ku for auth protocol */
		unsigned int          securityAuthKeyLen;
		/** Kul for auth protocol */
		unsigned char          *securityAuthLocalKey;
		/** Length of Kul for auth protocol XXX */
		unsigned int          securityAuthLocalKeyLen;

		/** priv protocol oid */
		unsigned long            *securityPrivProto;
		/** Length of priv protocol oid */
		unsigned int          securityPrivProtoLen;
		/** Ku for privacy protocol XXX */
		unsigned char          securityPrivKey[USM_PRIV_KU_LEN];
		/** Length of Ku for priv protocol */
		unsigned int          securityPrivKeyLen;
		/** Kul for priv protocol */
		unsigned char          *securityPrivLocalKey;
		/** Length of Kul for priv protocol XXX */
		unsigned int          securityPrivLocalKeyLen;

		/** snmp security model, v1, v2c, usm */
		int             securityModel;
		/** noAuthNoPriv, authNoPriv, authPriv */
		int             securityLevel;
		/** target param name */
		char           *paramName;

		/**
		* security module specific
		*/
		void           *securityInfo;

		/**
		* transport specific configuration
		*/
		struct afdx_netsnmp_container_s *transport_configuration;

		/**
		* use as you want data
		*
		*     used by 'SNMP_FLAGS_RESP_CALLBACK' handling in the agent
		* XXX: or should we add a new field into this structure?
		*/
		void           *myvoid;
	};

	struct afdx_synch_state {
		int             waiting;
		int             status;
		/*
		* status codes
		*/
#define STAT_SUCCESS	0
#define STAT_ERROR	1
#define STAT_TIMEOUT 2
		int             reqid;
		afdx_netsnmp_pdu    *pdu;
	};

#ifdef __cplusplus
}
#endif
#include <AiFdx_def.h>
typedef struct {
	/* Resource ID */
	AiUInt32 ul_ID;
	/* Resource Handle */
	AiUInt32 ul_Handle;
	/* Resource Name */
	AiChar ac_Name[MAX_STRING_1];
	/* Is GnetBoard */
	AiBool32 b_Gnet;
} TY_FDX_RESOURCE_INFO;

#endif                          /* NET_SNMP_TYPES_H */
