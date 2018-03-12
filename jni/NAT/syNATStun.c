#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <net/if.h>
#include <assert.h>
#include <pthread.h>

#include "syNATStun.h"
#include "syNATUdp.h"
#include "syCwmpCommon.h"
//#include "sha1.h"

/// define a structure to 22hold a stun address 
const UInt8  IPv4Family = 0x01;
const UInt8  IPv6Family = 0x02;

// define  flags  
const UInt32 ChangeIpFlag   = 0x04;
const UInt32 ChangePortFlag = 0x02;

// define  stun attribute
const UInt16 MappedAddress    = 0x0001;
const UInt16 ResponseAddress  = 0x0002;
const UInt16 ChangeRequest    = 0x0003;
const UInt16 SourceAddress    = 0x0004;
const UInt16 ChangedAddress   = 0x0005;
const UInt16 Username         = 0x0006;
const UInt16 Password         = 0x0007;
const UInt16 MessageIntegrity = 0x0008;
const UInt16 ErrorCode        = 0x0009;
const UInt16 UnknownAttribute = 0x000A;
const UInt16 ReflectedFrom    = 0x000B;
const UInt16 ConnectionRequestBinding = 0xC001;
const UInt16 BindingChange            = 0xC002;

// define types for a stun message 
const UInt16 BindRequestMsg               = 0x0001;
const UInt16 BindResponseMsg              = 0x0101;
const UInt16 BindErrorResponseMsg         = 0x0111;
const UInt16 SharedSecretRequestMsg       = 0x0002;
const UInt16 SharedSecretResponseMsg      = 0x0102;
const UInt16 SharedSecretErrorResponseMsg = 0x0112;


StunMessage gRespMsg;
char gUdpConnectionMsg[STUN_MAX_MESSAGE_SIZE];
StunAtrAddress4 gMappedAddr;
MessageType gRespType = NoneMsg;
UInt128 gStunReqId;
//UInt16 gStunUsernameSize = 0;
char gUdpMsgSig[20] = {0};
UInt32 gUdpConnectAddr = 0;
int gUdpConnectPort = 0;
StunAtrString gStunUserName;
StunAtrString gStunPassword;
static UInt32 gPreNotifiedTime = 0;
static UInt32 gUdpConnectTimeStamp = 0;
extern UInt32 gNotificationLimit;
extern int gDiscoveryPhase;
extern int NATDetected;
static pthread_mutex_t gRespMutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_t gProcessThreadId = 0;

extern inline int getErrno() ;


static void
computeHmac(char* hmac, const char* input, int length, const char* key, int keySize);

static void   
itoa(unsigned long val, char *buf, unsigned int radix)   
{   
	char *p;  
	char *firstdig;
	char temp;    
	unsigned int digval; 

	p = buf;   
	firstdig = p;       

	do {   
		digval = (unsigned int)(val % radix);   
		val /= radix;            

		if (digval > 9)   
			*p++ = (char)(digval - 10 + 'a'); 
		else   
			*p++ = (char)(digval + '0');     
	} while (val > 0);   

	*p-- = '\0';

	do {   
		temp = *p;   
		*p = *firstdig;   
		*firstdig = temp;  
		--p;   
		++firstdig;    
	} while (firstdig < p);
}

static int 
stunParseAtrAddress( const char* body, unsigned int hdrLen,  StunAtrAddress4* result)
{
	if ( hdrLen != 8)
	{
		EPrint("hdrLen wrong for Address\n");
		return SY_FAILED;
	}

	result->pad = *body++;
	result->family = *body++;

	if (result->family == IPv4Family)
	{
		UInt16 nport;
		memcpy(&nport, body, 2); body+=2;
		result->ipv4.port = ntohs(nport);

		UInt32 naddr;
		memcpy(&naddr, body, 4); body+=4;
		result->ipv4.addr = ntohl(naddr);
		return SY_SUCCESS;
	}
	else if (result->family == IPv6Family)
	{
		VPrint("ipv6 not supported\n");
	}
	else
	{
		VPrint("bad address family: %d\n", result->family);
	}

	return SY_FAILED;
}

static int 
stunParseAtrChangeRequest( const char* body, unsigned int hdrLen,  StunAtrChangeRequest* result)
{
	if ( hdrLen != 4)
	{
		EPrint("hdr length = %d, expecting = %d\n", hdrLen, sizeof(*result));//mark
		return SY_FAILED;
	}
	else
	{
		memcpy(&result->value, body, 4);
		result->value = ntohl(result->value);
		return SY_SUCCESS;
	}
}

static int 
stunParseAtrError( const char* body, unsigned int hdrLen,  StunAtrError* result)
{
	if ( hdrLen >= sizeof(StunAtrError))
	{
		EPrint("head on Error too large\n");
		return SY_FAILED;
	}
	else
	{
		memcpy(&result->pad, body, 2); body+=2;
		result->pad = ntohs(result->pad);
		result->errorClass = *body++;
		result->number = *body++;
		VPrint("errorclass=%d,number=%d\n",result->errorClass,result->number);
		result->sizeReason = hdrLen - 4;
		memcpy(&result->reason, body, result->sizeReason);
		result->reason[result->sizeReason] = 0;
		return SY_SUCCESS;
	}
}

static int 
stunParseAtrUnknown( const char* body, unsigned int hdrLen,  StunAtrUnknown* result)
{
	if ( hdrLen >= sizeof(StunAtrUnknown)) 
	{
		return SY_FAILED;
	}
	else
	{
		if (hdrLen % 4 != 0) return SY_FAILED;
		result->numAttributes = hdrLen / 4;
		for (int i=0; i<result->numAttributes; i++)
		{
			memcpy(&result->attrType[i], body, 2); body+=2;
			result->attrType[i] = ntohs(result->attrType[i]);
		}
		return SY_SUCCESS;
	}
}


static int 
stunParseAtrString( const char* body, unsigned int hdrLen,  StunAtrString* result)
{
	if ( hdrLen >= STUN_MAX_STRING)
	{
		EPrint("String is too large\n");
		return SY_FAILED;
	}
	else
	{
		if (hdrLen % 4 != 0)
		{
			EPrint("Bad length string %d\n", hdrLen);
			return SY_FAILED;
		}

		result->sizeofValue = hdrLen;
		memcpy(&result->value, body, hdrLen);
		result->value[hdrLen] = 0;
		return SY_SUCCESS;
	}
}


static int 
stunParseAtrIntegrity( const char* body, unsigned int hdrLen,  StunAtrIntegrity* result)
{
	if ( hdrLen != 20)
	{
		EPrint("MessageIntegrity must be 20 bytes\n");
		return SY_FAILED;
	}
	else
	{
		memcpy(&result->hash, body, hdrLen);
		return SY_SUCCESS;
	}
}



static char* 
encode16(char* buf, UInt16 data)
{
	UInt16 ndata = htons(data);
	memcpy(buf, (void*)&ndata, sizeof(UInt16));
	return buf + sizeof(UInt16);
}

static char* 
encode32(char* buf, UInt32 data)
{
	UInt32 ndata = htonl(data);
	memcpy(buf, (void*)&ndata, sizeof(UInt32));
	return buf + sizeof(UInt32);
}


static char* 
encode(char* buf, const char* data, unsigned int length)
{
	memcpy(buf, data, length);
	return buf + length;
}


static char* 
encodeAtrAddress4(char* ptr, UInt16 type, const StunAtrAddress4* atr)
{
	ptr = encode16(ptr, type);
	ptr = encode16(ptr, 8);
	*ptr++ = atr->pad;
	*ptr++ = IPv4Family;
	ptr = encode16(ptr, atr->ipv4.port);
	ptr = encode32(ptr, atr->ipv4.addr);

	return ptr;
}

static char* 
encodeAtrChangeRequest(char* ptr, const StunAtrChangeRequest* atr)
{
	ptr = encode16(ptr, ChangeRequest);
	ptr = encode16(ptr, 4);
	ptr = encode32(ptr, atr->value);
	return ptr;
}

static char* 
encodeAtrError(char* ptr, const StunAtrError* atr)
{
	ptr = encode16(ptr, ErrorCode);
	ptr = encode16(ptr, 6 + atr->sizeReason);
	ptr = encode16(ptr, atr->pad);
	*ptr++ = atr->errorClass;
	*ptr++ = atr->number;
	ptr = encode(ptr, atr->reason, atr->sizeReason);
	return ptr;
}


static char* 
encodeAtrUnknown(char* ptr, const StunAtrUnknown* atr)
{
	ptr = encode16(ptr, UnknownAttribute);
	ptr = encode16(ptr, 2+2*atr->numAttributes);
	for (int i=0; i<atr->numAttributes; i++)
	{
		ptr = encode16(ptr, atr->attrType[i]);
	}
	return ptr;
}


static char* 
encodeAtrString(char* ptr, UInt16 type, const StunAtrString* atr)
{
	assert(atr->sizeofValue % 4 == 0);

	ptr = encode16(ptr, type);
	ptr = encode16(ptr, atr->sizeofValue);
	ptr = encode(ptr, atr->value, atr->sizeofValue);
	return ptr;
}


static char* 
encodeAtrIntegrity(char* ptr, const StunAtrIntegrity* atr)
{
	ptr = encode16(ptr, MessageIntegrity);
	ptr = encode16(ptr, 20);
	ptr = encode(ptr, atr->hash, sizeof(atr->hash));
	return ptr;
}


#ifdef NOSSL
static void
computeHmac(char* hmac, const char* input, int length, const char* key, int sizeKey)
{
	strncpy(hmac,"hmac-not-implemented",20);
}
#else
#include <openssl/hmac.h>

static void
computeHmac(char* hmac, const char* input, int length, const char* key, int sizeKey)
{
	unsigned int resultSize=0;
	//VPrint("parameter key=%s, sizeKey=%d, length=%d\n",key,sizeKey,length);
	//for (int i = 0;i < length;i++)
	//{
	//	VPrint("send message one byte=%d\n",(int)input[i]);
	//}
	HMAC(EVP_sha1(), key, sizeKey, (const unsigned char*)input, length, (unsigned char*)hmac, &resultSize);
	assert(resultSize == 20);
}
#endif


static void
toHex(const char* buffer, int bufferSize, char* output) 
{
	static char hexmap[] = "0123456789abcdef";

	const char* p = buffer;
	char* r = output;
	for (int i=0; i < bufferSize; i++)
	{
		unsigned char temp = *p++;

		int hi = (temp & 0xf0)>>4;
		int low = (temp & 0xf);

		*r++ = hexmap[hi];
		*r++ = hexmap[low];
	}
	*r = 0;
}

int
stunParseMessage( const char* buf, unsigned int bufLen, StunMessage* msg)
{
	VPrint("Received stun message: %d bytes\n", bufLen);

	if (sizeof(StunMsgHdr) > bufLen)
	{
		EPrint("Bad message\n");
		return SY_FAILED;
	}

	memcpy(&(msg->msgHdr), buf, sizeof(StunMsgHdr));
	VPrint("msgType = %d, ntohs msgType = %d\n", msg->msgHdr.msgType, ntohs(msg->msgHdr.msgType));
	msg->msgHdr.msgType = ntohs(msg->msgHdr.msgType);
	msg->msgHdr.msgLength = ntohs(msg->msgHdr.msgLength);

	if (msg->msgHdr.msgLength + sizeof(StunMsgHdr) != bufLen)
	{
		EPrint("Message header length doesn't match message size: %d - %d\n", msg->msgHdr.msgLength, bufLen);
		return SY_FAILED;
	}

	const char* body = buf + sizeof(StunMsgHdr);
	unsigned int size = msg->msgHdr.msgLength;

	while ( size > 0)
	{
		// !jf! should check that there are enough bytes left in the buffer
		StunAtrHdr* attr = (StunAtrHdr*)body;

		unsigned int attrLen = ntohs(attr->length);
		int atrType = ntohs(attr->type);

		if ( attrLen+4 > size) 
		{
			EPrint("claims attribute is larger than size of message (attribute type=%d) attrLen=%d,size=%d\n",atrType,attrLen,size);
			return SY_FAILED;
		}

		body += 4; // skip the length and type in attribute header
		size -= 4;

		switch ( atrType)
		{
		case MAPPEDADDRESS:
			msg->hasMappedAddress = SY_TRUE;
			if (stunParseAtrAddress(body, attrLen, &msg->mappedAddress) == SY_FAILED)
			{
				EPrint("problem parsing MappedAddress\n");
				return SY_FAILED;
			}
			else
			{
				VPrint("MappedAddress = %x\n", msg->mappedAddress.ipv4.addr);
			}
			break;  

		case RESPONSEADDRESS:
			msg->hasResponseAddress = SY_TRUE;
			if (stunParseAtrAddress(body, attrLen, &msg->responseAddress) == SY_FAILED)
			{
				EPrint("problem parsing ResponseAddress\n");
			}
			else
			{
				VPrint("ResponseAddress = %x\n",msg->responseAddress.ipv4.addr);
			}
			break;  

		case CHANGEREQUEST:
			msg->hasChangeRequest = SY_TRUE;
			if (stunParseAtrChangeRequest(body, attrLen, &msg->changeRequest) == SY_FAILED)
			{
				EPrint("problem parsing ChangeRequest\n");
				return SY_FAILED;
			}
			else
			{
				VPrint("ChangeRequest = %x\n", msg->changeRequest.value);
			}
			break;

		case SOURCEADDRESS:
			msg->hasSourceAddress = SY_TRUE;
			if (stunParseAtrAddress(body, attrLen, &msg->sourceAddress) == SY_FAILED)
			{
				EPrint("problem parsing SourceAddress\n");
				return SY_FAILED;
			}
			else
			{
				VPrint("SourceAddress = %x\n", msg->sourceAddress.ipv4.addr);
			}
			break;  

		case CHANGEDADDRESS:
			msg->hasChangedAddress = SY_TRUE;
			if (stunParseAtrAddress(body, attrLen, &msg->changedAddress) == SY_FAILED)
			{
				EPrint("problem parsing ChangedAddress\n");
				return SY_FAILED;
			}
			else
			{
				VPrint("ChangedAddress = %x\n", msg->changedAddress.ipv4.addr);
			}
			break;  

		case USERNAME: 
			msg->hasUsername = SY_TRUE;
			if (stunParseAtrString( body, attrLen, &msg->username) == SY_FAILED)
			{
				EPrint("problem parsing Username\n");
				return SY_FAILED;
			}
			else
			{
				VPrint("Username = %s\n", msg->username.value);
			}

			break;

		case PASSWORD: 
			msg->hasPassword = SY_TRUE;
			if (stunParseAtrString( body, attrLen, &msg->password) == SY_FAILED)
			{
				EPrint("problem parsing Password\n");
				return SY_FAILED;
			}
			else
			{
				VPrint("Password = %s\n", msg->password.value);
			}
			break;

		case MESSAGEINTEGRITY:
			msg->hasMessageIntegrity = SY_TRUE;
			if (stunParseAtrIntegrity( body, attrLen, &msg->messageIntegrity) == SY_FAILED)
			{
				EPrint("problem parsing MessageIntegrity\n");
				return SY_FAILED;
			}
			else
			{
				VPrint("MessageIntegrity = %s", msg->messageIntegrity.hash);
			}

			// read the current HMAC
			// look up the password given the user of given the transaction id 
			// compute the HMAC on the buffer
			// decide if they match or not
			break;

		case ERRORCODE:
			msg->hasErrorCode = SY_TRUE;
			if (stunParseAtrError(body, attrLen, &msg->errorCode) == SY_FAILED)
			{
				EPrint("problem parsing ErrorCode\n");
				return SY_FAILED;
			}
			else
			{
				VPrint("ErrorCode = %d %s\n", msg->errorCode.number, msg->errorCode.reason);
			}
			break;

		case UNKNOWNATTRIBUTE:
			msg->hasUnknownAttributes = SY_TRUE;
			if (stunParseAtrUnknown(body, attrLen, &msg->unknownAttributes) == SY_FAILED)
			{
				EPrint("problem parsing UnknownAttribute\n");
				return SY_FAILED;
			}
			break;

		case REFLECTEDFROM:
			msg->hasReflectedFrom = SY_TRUE;
			if (stunParseAtrAddress(body, attrLen, &msg->reflectedFrom) == SY_FAILED)
			{
				EPrint("problem parsing ReflectedFrom\n");
				return SY_FAILED;
			}
			break;  

		default:

			VPrint("Unknown attribute: %x\n", atrType);
#if 0
			if ( atrType <= 0x7FFF) 
			{
				return SY_FAILED;
			}
#endif
			break;
		}

		body += attrLen;
		size -= attrLen;
	}

	return SY_SUCCESS;
}

unsigned int
stunEncodeMsg( const StunMessage* msg, 
	char* buf, 
	unsigned int bufLen, 
	const StunAtrString* password)
{
	assert(bufLen >= sizeof(StunMsgHdr));
	char* ptr = buf;

	ptr = encode16(ptr, msg->msgHdr.msgType);
	char* lengthp = ptr;
	ptr = encode16(ptr, 0);
	ptr = encode(ptr, (const char*)msg->msgHdr.id.octet, sizeof(msg->msgHdr.id));

	//VPrint("Encoding stun message: \n");
	if (msg->hasRequstBinding)
	{
		ptr = encodeAtrString(ptr, ConnectionRequestBinding, &msg->connectionRequestBinding);
	}
	if (msg->hasBindingChange)
	{
		ptr = encodeAtrString(ptr, BindingChange, &msg->bindingChange);
	}
#if 0
	if (msg->hasMappedAddress)
	{
		VPrint("Encoding MappedAddress: %d\n", msg->mappedAddress.ipv4);
		ptr = encodeAtrAddress4 (ptr, MappedAddress, &msg->mappedAddress);
	}
	if (msg->hasResponseAddress)
	{
		VPrint("Encoding ResponseAddress: %d\n", msg->responseAddress.ipv4);
		ptr = encodeAtrAddress4(ptr, ResponseAddress, &msg->responseAddress);
	}
	if (msg->hasChangeRequest)
	{
		VPrint("Encoding ChangeRequest: %d\n", msg->changeRequest.value);
		ptr = encodeAtrChangeRequest(ptr, &msg->changeRequest);
	}
	if (msg->hasSourceAddress)
	{
		VPrint("Encoding SourceAddress: %d\n", msg->sourceAddress.ipv4);
		ptr = encodeAtrAddress4(ptr, SourceAddress, &msg->sourceAddress);
	}
	if (msg->hasChangedAddress)
	{
		VPrint("Encoding ChangedAddress: %d\n", msg->changedAddress.ipv4);
		ptr = encodeAtrAddress4(ptr, ChangedAddress, &msg->changedAddress);
	}
#endif
	if (msg->hasUsername)
	{
		DPrint("Encoding Username: %s\n", msg->username.value);
		ptr = encodeAtrString(ptr, Username, &msg->username);
	}
	if (msg->hasPassword)
	{
		DPrint("Encoding Password: %s\n", msg->password.value);
		ptr = encodeAtrString(ptr, Password, &msg->password);
	}
	if (msg->hasErrorCode)
	{
		DPrint("Encoding ErrorCode: number=%d reason=%s\n", msg->errorCode.number, msg->errorCode.reason);
		ptr = encodeAtrError(ptr, &msg->errorCode);
	}
	if (msg->hasUnknownAttributes)
	{
		DPrint("Encoding UnknownAttribute: ???");
		ptr = encodeAtrUnknown(ptr, &msg->unknownAttributes);
	}
	//if (msg->hasReflectedFrom)
	//{
	//	VPrint("Encoding ReflectedFrom: %d\n", msg->reflectedFrom.ipv4);
	//	ptr = encodeAtrAddress4(ptr, ReflectedFrom, &msg->reflectedFrom);
	//}
	if (password->sizeofValue > 0)
	{
		VPrint("HMAC with password: %s, input len : %d\n", password->value,(ptr-buf));
		encode16(lengthp, (ptr - buf - sizeof(StunMsgHdr) + 24));

		StunAtrIntegrity integrity;
		computeHmac(integrity.hash, buf, ptr-buf , password->value, password->sizeofValue);
		ptr = encodeAtrIntegrity(ptr, &integrity);
	}
	else
	{
		encode16(lengthp, (ptr - buf - sizeof(StunMsgHdr)));
	}

	//encode16(lengthp, (ptr - buf - sizeof(StunMsgHdr)));
	return (ptr - buf);
}

int 
stunRand()
{
	// return 32 bits of random stuff
	assert( sizeof(int) == 4);
	static int init = SY_FALSE;
	if ( !init)
	{ 
		init = SY_TRUE;

		UInt64 tick;

		int seed = (int)tick;
		srandom(seed);
	}

	return random(); 
}


/// return a random number to use as a port 
int
stunRandomPort()
{
	int min=0x4000;
	int max=0x7FFF;

	int ret = stunRand();
	ret = ret|min;
	ret = ret&max;

	return ret;
}

UInt64
stunGetSystemTimeSecs()
{
	UInt64 time=0;
#if defined(WIN32)  
	SYSTEMTIME t;
	// CJ TODO - this probably has bug on wrap around every 24 hours
	GetSystemTime( &t);
	time = (t.wHour*60+t.wMinute)*60+t.wSecond; 
#else
	struct timeval now;
	gettimeofday( &now , NULL);
	//assert( now);
	time = now.tv_sec;
#endif
	return time;
}


// returns SY_TRUE if it scucceeded
int 
stunParseHstName( char* peerName,
	UInt32* ip,
	UInt16* portVal,
	UInt16 defaultPort)
{
	struct in_addr sin_addr;

	char host[512];
	strncpy(host, peerName, 512);
	host[512-1]='\0';
	char* port = NULL;

	int portNum = defaultPort;
	// pull out the port part if present.
	char* sep = strchr(host,':');
	if ( sep != NULL)
	{
		*sep = '\0';
		port = sep + 1;
		// set port part

		char* endPtr=NULL;

		portNum = strtol(port, &endPtr, 10);

		if ( endPtr != NULL)
		{
			if ( *endPtr != '\0')
			{
				portNum = defaultPort;
			}
		}
	}

	if ( portNum < 1024) return SY_FAILED;
	if ( portNum >= 0xFFFF) return SY_FAILED;

	// figure out the host part 
	struct hostent* h;

	h = gethostbyname(host);
	if (h == NULL)
	{
		int err = getErrno();
		EPrint("error was %d\n", err);
		*ip = ntohl( 0x7F000001L);
		return SY_FAILED;
	}
	else
	{
		sin_addr = *(struct in_addr*)h->h_addr;
		*ip = ntohl(sin_addr.s_addr);
	}

	*portVal = portNum;

	return SY_SUCCESS;
}


int
stunParseSrvName( char* name, StunAddress4* addr, int port)
{
	assert(name);

	// TODO - put in DNS SRV stuff.
	int dstPort = (port==0) ? STUN_DEFAULT_PORT : port;	
	int ret = stunParseHstName( name, &addr->addr, &addr->port, dstPort); 
	if ( ret != SY_SUCCESS) 
	{
		addr->port=0xFFFF;
	}	
	return ret;
}

void
stunBuildReqSimple(StunMessage* msg, const StunAtrString* username,
	int bindingChange, unsigned int id)
{
	assert(msg);
	
	memset(msg , 0 , sizeof(*msg));
	msg->msgHdr.msgType = BindRequestMsg;

	for (int i=0; i<16; i=i+4)
	{
		assert(i+3<16);
		int r = stunRand();
		msg->msgHdr.id.octet[i+0]= r>>0;
		msg->msgHdr.id.octet[i+1]= r>>8;
		msg->msgHdr.id.octet[i+2]= r>>16;
		msg->msgHdr.id.octet[i+3]= r>>24;
	}

	int len = sizeof(UInt128);
	memset(&gStunReqId, 0, len);
	memcpy(&gStunReqId, &msg->msgHdr.id, len);

	if (id != 0)
	{
		msg->msgHdr.id.octet[0] = id; 
	}

	if (username->sizeofValue > 0)
	{
		msg->hasUsername = SY_TRUE;
		msg->username = *username;
	}

	StunAtrString requestBinding;
	memcpy(requestBinding.value, "dslforum.org/TR-111 ", 20);
	requestBinding.sizeofValue = 20;
	msg->hasRequstBinding = SY_TRUE;
	msg->connectionRequestBinding = requestBinding;

	if (SY_TRUE == bindingChange)
	{
		StunAtrString change;
		change.sizeofValue = 0;
		msg->hasBindingChange = SY_TRUE;
		msg->bindingChange = change;
	}
}


static int 
stunSendTest(const StunAddress4* dest, const StunAtrString* username, 
	const StunAtrString* password, int bindingChange)
{ 
	assert( dest->addr != 0);
	assert( dest->port != 0);
	//assert(buf != NULL);

	if (gPriFd == INVALID_SOCKET)
	{
		gPriFd = openPort(SY_NAT_PRIMARY_PORT, 0);
	}
	assert(gPriFd != INVALID_SOCKET);


	StunMessage req;
	memset(&req, 0, sizeof(StunMessage));

	stunBuildReqSimple(&req, username, bindingChange, 0);//need a id

	char buf[STUN_MAX_MESSAGE_SIZE];
	int len = STUN_MAX_MESSAGE_SIZE;

	len = stunEncodeMsg(&req, buf, len, password);

	DPrint("About to send msg of len %d to %d\n", len, dest->addr);
	
	//VPrint("About to send msg of len %d to %d,buf = %s\n", len, dest->addr,buf);
	int ret = sendMessage(gPriFd, buf, len, dest->addr, dest->port);
	if (SY_FAILED == ret)
	{
		SyCloseSocket(gPriFd);
		gPriFd = INVALID_SOCKET;
		return SY_FAILED;
	}

	// add some delay so the packets don't get sent too quickly 
	usleep(80 * 1000);
	return 0;
}

static int 
stunVerifyIntegrity(const StunMessage* msg, const StunAtrString* password, const char* buf)
{
	StunAtrIntegrity integrity;
	computeHmac(integrity.hash, buf, (msg->msgHdr.msgLength - 4) , password->value, password->sizeofValue);
//	for (int i = 0; i < 20; i++)
//	{
//		VPrint("client:%d, server:%d\n", (int)integrity.hash[i], (int)msg->messageIntegrity.hash[i]);
//	}
	if (memcmp(integrity.hash, msg->messageIntegrity.hash, 20) != 0)
	{
		VPrint("acs response msg hmac compare false\n");
		return SY_FAILED;
	}
	return SY_SUCCESS;
}

//判断是否有错误的地方
static void
stunHandleError(const StunMessage *resp, StunAtrString *username, 
	StunAtrString *password, int *retry)
{
	VPrint("msgType = %d, hasErrorCode = %d\n",
		resp->msgHdr.msgType, resp->hasErrorCode);
	assert(resp);

	*retry = SY_FALSE;

	if (resp->msgHdr.msgType == BindErrorResponseMsg && resp->hasErrorCode)
	{
		DPrint("the acs return errorclass=%d,number=%d\n",
				resp->errorCode.errorClass ,resp->errorCode.number);

		if (resp->errorCode.errorClass < 4)
		{
			WPrint("error class < 4, ignore the error\n");
			return;
		}

		int code = resp->errorCode.errorClass * 100;
		if (4 == resp->errorCode.errorClass) 
		{
			code += resp->errorCode.number;
		}
		switch (code)
		{
		case 401:
		case 430:
		case 431:
			*retry = SY_TRUE;
			break;

		case 500:
			usleep(10 * 1000 * 1000);
			*retry = SY_TRUE;
			break;

		case 600:
			VPrint("error code is %d, and error %s\n", code, resp->errorCode.reason);
			break;

		default:
			if (code < 500)
			{
				VPrint("error code is %d, and error %s\n", code, resp->errorCode.reason);
			}
			break;
		}
	}
}

static int
stunChkResponse(const StunMessage *msg, 
                        	StunAtrString *password, 
                        	const char *buf)
{
	assert(msg);
	assert(password);

	//int len = sizeof(UInt128);
	//int equal = SY_TRUE;
	//for (int i = 0; i < len; ++i)
	//{
	//	if (gStunReqId.octet[i] != msg->msgHdr.id.octet[i])
	//	{
	//		equal = SY_FALSE;
	//		VPrint("request:%ld, response:%ld\n", gStunReqId.octet[i], msg->msgHdr.id.octet[i]);
	//	}
	//}

	if (memcmp(gStunReqId.octet, msg->msgHdr.id.octet, sizeof(UInt128)) != 0)
	{
		VPrint("request and response message id not equal, requst:%s, response:%s\n",
			gStunReqId.octet, msg->msgHdr.id.octet);
		return SY_FAILED;
	}

	if (msg->msgHdr.msgType == BindResponseMsg && SY_TRUE != msg->hasMappedAddress)
	{
		VPrint("get message don't have mappedaddress\n");
		return SY_FAILED;
	}

	if (password->sizeofValue > 0)
	{
		VPrint("password sizevalue > 0\n");

		if (SY_FALSE == msg->hasMessageIntegrity)
		{
			VPrint("has messageintegrity is false\n");
			return SY_FAILED;
		}
		else
		{
			//verification the response
			VPrint("verify integrity\n");
			return stunVerifyIntegrity(msg, password, buf);
		}
	}
	else 
	{
		if (SY_TRUE == msg->hasMessageIntegrity)
		{
			VPrint("client don't have integrity, so does server\n");
			return SY_FAILED;
		}
	}

	VPrint("verify binding response success\n");
	return SY_SUCCESS;
}


static int
stunVerifyAcsUdpRequest(const char *msg, const char *key)
{
	char input[128] = {0};
	char signature[20] = {0}; 
	char output[20] = {0};
	char* start = NULL;
	const char* end = msg;
	char* charin = input;
	char ts[256] = {0};
	char un[256] = {0};
	UInt32 timestamp = 0;

	char username[256] = {0};
	SyGetNodeValue("Device.ManagementServer.ConnectionRequestUsername", username);

	DO;

	while (1)
	{
		if ((start = strstr(end, "ts=")) != NULL ||
			(start = strstr(end, "id=")) != NULL ||
			(start = strstr(end, "un=")) != NULL ||
			(start = strstr(end, "cn=")) != NULL)
		{
			int hasTs = 0;
			int hasUn = 0;
			if (strncmp(start, "ts=", 3) == 0)
				hasTs = 1;
			else if (strncmp(start, "un=", 3) == 0)
				hasUn = 1;

			end = start + 3;
			const char *temp = end;
			while (*temp != '&' && *temp != '\0')
				temp++;

			int len = temp - end;
			if (hasTs)
				strncpy(ts, end, len);
			else if (hasUn)
				strncpy(un, end, len);
			memcpy(charin, end, len);	
			charin += len;
			VPrint("GET: %s\n", input);
		}
		else if ((start = strstr(end, "sig=")) != NULL)
		{
			*charin = '\0';
			end = start + 4;
			char temp[2] = {0};
			VPrint("sig= %s\n", end);
			for (int i = 0; i < 40; i += 2)
			{
				strncpy(temp, (end + i), 2);
				int value = strtoul(temp, NULL, 16);
				memcpy(&signature[i/2], &value, sizeof(char));
				VPrint("signature: %2x\n", signature[i/2]);
			}
		}
		else
		{
			break;
		}
	}

	//sha1_hmac(key, strlen(key), input, strlen(input), output);
	computeHmac(output, input, strlen(input), key, strlen(key));
	timestamp = atoi(ts);

	VPrint("udp connection msg un: %s\n", un);
	//if (timestamp > gUdpConnectTimeStamp && strcmp(un, username) == 0 && 
	//	strncmp(output, signature, 20) == 0 && strncmp(gUdpMsgSig, signature, 20) != 0)
	if (strcmp(un, username) == 0 && strncmp(output, signature, 20) == 0 &&
		strncmp(gUdpMsgSig, signature, 20) != 0)//modify by hh 取消时间对比，华为平台设置参数时会发两个相同时间的消息，需要上报两次
	{
		VPrint("verify udp connection request success and not repeated\n");
		gUdpConnectTimeStamp = timestamp;	
		strncpy(gUdpMsgSig, signature, 20);
		return SY_SUCCESS;
	}

	return SY_FAILED;
}

static int
stunSendMsgToTR069()
{
	DO;
	int count = 3;
	int ret = -1;
	if (INVALID_SOCKET == gLocalMsgFd)
	{
		gLocalMsgFd = socket(PF_INET, SOCK_STREAM, 0); 
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		addr.sin_port = htons(CMD_PROC_PORT);

		struct timeval tv;
		tv.tv_usec = 0;
		tv.tv_sec = 3;  

		setsockopt(gLocalMsgFd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));   
		ret = connect(gLocalMsgFd, (struct sockaddr *)&addr, sizeof(addr));
		VPrint("gLocalMsgFd:%d, ret:%d\n", gLocalMsgFd, ret);
		if (ret < 0){
			EPrint("Connect Error: %d\n", errno);
			SyCloseSocket(gLocalMsgFd);
			gLocalMsgFd = -1;
			return SY_FAILED;
		}
		
		struct in_addr localAddr;
		struct sockaddr_in RecvAddr;
		localAddr.s_addr = 0;
		int nLen = sizeof(RecvAddr);
		memset(&RecvAddr, 0, sizeof(RecvAddr));
		RecvAddr.sin_family = AF_INET;
		if (getsockname(gLocalMsgFd, (struct sockaddr *)&RecvAddr, &nLen) == 0)
		{
			localAddr.s_addr = RecvAddr.sin_addr.s_addr;
			DPrint("Local ip %s:%d, Net-Order(BE) port =%d\n",
					inet_ntoa(localAddr), ntohs(RecvAddr.sin_port), RecvAddr.sin_port);
		}
	}
	assert(gLocalMsgFd != INVALID_SOCKET);

	Msg_TR069_s msg;
	memset(&msg, 0, sizeof(msg));

	strcpy(msg.user, "tr069");
    char cmdstr[] = "SY_OP_CONNECTION_REQUEST";
	msg.len = snprintf(msg.msg, SY_MSG_LEN, cmdstr, sizeof(cmdstr));


	while (count--)	{
		DPrint("user:%s, cmd:%d, type:%d, msg:%s, len:%d.\n", 
			msg.user,
			msg.cmd,
			msg.type,
			msg.msg,
			msg.len);
    	ret = send(gLocalMsgFd, (const char*)&msg, sizeof(struct sockmsg), MSG_NOSIGNAL);
		if (ret > 0)
			break;
		usleep(100 * 1000);
	}
    if(ret <= 0){
        EPrint("Send Msg Fail\n");      
        SyCloseSocket(gLocalMsgFd);
        gLocalMsgFd = -1;
        return SY_FAILED;
    }

	VPrint("send message to tr069 success ret=%d\n",ret);
	return SY_SUCCESS;
}

static void*
stunHandleUdpConnectRequst(void *args)
{ 
	stunVerifyMsg(gUdpConnectionMsg, UdpReq);
	return NULL;
}

static void*
stunRecvMessage(void *args)
{ 
	DO;

	while(1)
	{
		if (INVALID_SOCKET == gPriFd)
		{
			gPriFd = openPort(SY_NAT_PRIMARY_PORT, 0);
		}
		assert(gPriFd != INVALID_SOCKET);
		
		char msg[STUN_MAX_MESSAGE_SIZE];
		int msgLen = sizeof(msg);
		StunAddress4 from;

		VPrint("begin recv message \n");
		memset(msg, 0x0, sizeof(msg));
		int ret = getMessage(gPriFd, msg, &msgLen, &from.addr, &from.port);
		if (SY_FAILED == ret)
		{
			EPrint("get message failed\n");
			SyCloseSocket(gPriFd);
			gPriFd = INVALID_SOCKET;
			//return NULL;
			continue;
		}
		VPrint("recvdmessage msg[0] = %d\n",msg[0]);
		if (msg[0] == 1)
		{
			//StunMessage tempMsg;
			//int len = sizeof(StunMessage);
			//memcpy(&tempMsg, &gRespMsg, len);
			memset(&gRespMsg, 0, sizeof(StunMessage));
			stunParseMessage(msg, msgLen, &gRespMsg);

			VPrint("Received message of type %x, id= %d\n", 
					gRespMsg.msgHdr.msgType, (int)(gRespMsg.msgHdr.id.octet[0]));
			//if (SY_TRUE == gRespMsg.hasReflectedFrom)
			//{
			//	memcpy(&gRespMsg, &tempMsg, len);
			//	VPrint("get binding changed response\n");
			//}

			if (SY_SUCCESS == stunVerifyMsg(msg, BindResp))
			{
				VPrint("acs message verify success\n");
				pthread_mutex_lock(&gRespMutex);
				gRespType = BindResp;
				pthread_mutex_unlock(&gRespMutex);
			}
		}
		else
		{
			pthread_mutex_lock(&gRespMutex);
			gRespType = UdpReq;
			pthread_mutex_unlock(&gRespMutex);
			memset(gUdpConnectionMsg, 0, sizeof(gUdpConnectionMsg));
			memcpy(gUdpConnectionMsg, msg, STUN_MAX_MESSAGE_SIZE);
			//char *value = "?ts=1120673700&id=1234&un=CPE57689&cn=XTGRWIPC6D3IPXS3&sig=3545F7B5820D76A3DF45A3A509DA8D8C38F13512";
			//strncpy(gUdpConnectionMsg, value, strlen(value));  //test
			VPrint("get udp connection request is %s", gUdpConnectionMsg);

			pthread_t thread;
			int ret = pthread_create(&thread, NULL, stunHandleUdpConnectRequst, NULL);
			if (SY_SUCCESS != ret)
			{
				EPrint("thread create failed\n");
			} else {
				pthread_detach(thread);
			}
		}
	}
	DONE;
	return NULL;
}

static void 
stunBuildReqSec(StunMessage *msg, UInt32 id)
{
	assert(msg);
	
	memset(msg , 0 , sizeof(*msg));
	msg->msgHdr.msgType = BindRequestMsg;

	for (int i=0; i<16; i=i+4)
	{
		assert(i+3<16);
		int r = stunRand();
		msg->msgHdr.id.octet[i+0]= r>>0;
		msg->msgHdr.id.octet[i+1]= r>>8;
		msg->msgHdr.id.octet[i+2]= r>>16;
		msg->msgHdr.id.octet[i+3]= r>>24;
	}

	int len = sizeof(UInt128);
	memset(&gStunReqId, 0, len);
	memcpy(&gStunReqId, &msg->msgHdr.id, len);

	if (id != 0)
	{
		msg->msgHdr.id.octet[0] = id; 
	}

	msg->hasResponseAddress = SY_TRUE;
	memcpy(&msg->responseAddress, &gMappedAddr, sizeof(StunAtrAddress4));
	VPrint("response address ip = %d, port = %d\n", 
		msg->responseAddress.ipv4.addr, msg->responseAddress.ipv4.port);
}

static int
stunCompAddr(const StunAtrAddress4 *addr, int localPort)
{
	DO;
	int ret = getLocalIp(&gLocalIp);
	if (SY_SUCCESS == ret)
	{
		struct in_addr tmp_In_addr = {0};
		tmp_In_addr.s_addr = htonl(gLocalIp);
		VPrint("LocalIp:%s", inet_ntoa((tmp_In_addr)));
		memset(&tmp_In_addr, 0, sizeof(tmp_In_addr));
		tmp_In_addr.s_addr = htonl((long)addr->ipv4.addr);
		VPrint("Hole IP:%s", inet_ntoa((tmp_In_addr)));
		VPrint("Hole port:%d, local Port:%d\n", addr->ipv4.port, localPort);
		if (gLocalIp == addr->ipv4.addr 
			&& localPort == addr->ipv4.port) 
		{
			DPrint("address compared false\n");
			return SY_FALSE;
		} 
		else
		{
			DPrint("address compared true\n");
			return SY_TRUE;
		}
	}

	EPrint("get local ip failed\n");
	return SY_FAILED;
}
/* send discovery request */
int
stunSendDiscReq(const StunAddress4* dest)
{
	assert( dest->addr != 0);
	assert( dest->port != 0);

	if (INVALID_SOCKET == gSecFd)
	{
		gSecFd = openPort(SY_NAT_SECOND_PORT, 0);
	}
	assert(gSecFd != INVALID_SOCKET);

	StunMessage req;
	memset(&req, 0, sizeof(StunMessage));

	stunBuildReqSec(&req, 0);

	char buf[STUN_MAX_MESSAGE_SIZE];
	int len = STUN_MAX_MESSAGE_SIZE;
	StunAtrString password;
	password.sizeofValue = 0;

	len = stunEncodeMsg(&req, buf, len, &password);

	VPrint("About to send msg of len %d to %d\n", len, dest->addr);

	int ret = sendMessage(gSecFd, buf, len, dest->addr, dest->port);
	if (SY_FAILED == ret)
	{
		SyCloseSocket(gSecFd);
		gSecFd = INVALID_SOCKET;
		return SY_FAILED;
	}

	// add some delay so the packets don't get sent too quickly 
	sleep(1);
	return SY_SUCCESS;
}
/*  receive message thread */
int
stunRecvMsgThd()
{
	gMappedAddr.ipv4.addr = 0;
	gMappedAddr.ipv4.port = 0;

	if (gPriFd == INVALID_SOCKET)
	{
		EPrint("Some problem opening primary socket port\n");
		return SY_FAILED; 
	}

	pthread_t thread;
	int ret = pthread_create(&thread, NULL, stunRecvMessage, NULL);
	if (SY_SUCCESS != ret)
	{
		EPrint("create thread failed\n");
		return SY_FAILED; 
	} else {
		pthread_detach(thread);
	}

	return SY_SUCCESS;
}

int 
stunVerifyMsg(const char *buf, MessageType type)
{
	if (type == BindResp)
	{
		VPrint("received message is binding response\n");
		return stunChkResponse(&gRespMsg, &gStunPassword, buf);
	}
	else if (type == UdpReq)
	{
		VPrint("received message is udp connection request\n");
		//virificateion udp connection request
		char password[256] = {0};
		SyGetNodeValue("Device.ManagementServer.ConnectionRequestPassword", password);
		int ret = stunVerifyAcsUdpRequest(buf, (const char *)password);
		if (SY_SUCCESS == ret)
		{
			VPrint("send mssage to tr069\n");
			stunSendMsgToTR069();
		}
		return ret;
	}

	return SY_SUCCESS;
}

int 
stunSendRequest(const StunAddress4* dest, const StunAtrString* username, 
					const StunAtrString* password, int bindingChange)
{
	if (INVALID_SOCKET == gPriFd)
	{
		gPriFd = openPort(SY_NAT_PRIMARY_PORT, 0);
	}

	assert(gPriFd != INVALID_SOCKET);
	
	int interval = 0;
	int count = 0;
	int timeout = 0;
	int isBindingRequest = SY_FALSE;

	gRespType = NoneMsg;

	while (1)
	{
		pthread_mutex_lock(&gRespMutex);
		if (gRespType == BindResp)
			isBindingRequest = SY_TRUE;
		pthread_mutex_unlock(&gRespMutex);

		if (SY_TRUE == isBindingRequest || count >= STUN_TIMEOUT_RETRY) break;

		DPrint("stun send message timeout is %d\n", timeout);

		//struct timeval tv;
		fd_set fdSet; 
		int fdSetSize;
		fdSetSize = 0;

		FD_ZERO(&fdSet); 
		FD_SET(gPriFd, &fdSet); 
		fdSetSize = (gPriFd+1 > fdSetSize) ? gPriFd+1 : fdSetSize;

		int  err = select(fdSetSize, NULL, &fdSet, NULL, NULL);
		int e = getErrno();
		if (SOCKET_ERROR == err)
		{
			// error occured
			EPrint("Error %d - (%s) in select\n", e, strerror(e));
			return SY_FAILED;
		}
		else if (FD_ISSET(gPriFd, &fdSet))
		{
			//VPrint("the socket can send request\n");
			count++;
			int ret = stunSendTest(dest, username, password, bindingChange);
			if (SY_FAILED == ret)
			{
				SyCloseSocket(gPriFd);
				gPriFd = INVALID_SOCKET;
				return SY_FAILED;
			}

			if (interval != STUN_TIMEOUT_MAX_INTERVAL)
				interval = (interval == 0) ? 100 : (2 * interval);		
			timeout += interval;
		}

		usleep(interval * 1000);
	}

	if (gRespType == BindResp && count < STUN_TIMEOUT_RETRY)
	{
		gRespType = NoneMsg;
		VPrint("stun send request success\n");
		return SY_SUCCESS;
	}

	return SY_FAILED;
}

static void stunUpdateParam()
{
	SyGetNodeValue("Device.ManagementServer.STUNUsername", gStunUserName.value);
	SyGetNodeValue("Device.ManagementServer.STUNPassword", gStunPassword.value);
#if 1
	if (strlen(gStunUserName.value) == 0)
		SyGetNodeValue("Device.X_00E0FC.STBID", gStunUserName.value);
#endif
	gStunUserName.sizeofValue = strlen(gStunUserName.value);
	gStunPassword.sizeofValue = strlen(gStunPassword.value);
}

//检测是否有映射地址
int
stunNatType(const StunAddress4* dest, int port, int bindingChange)
{
	assert(dest->addr != 0);
	assert(dest->port != 0);
	assert(port != 0);

	stunUpdateParam();
	//char buf[STUN_MAX_MESSAGE_SIZE];
	int retry = SY_TRUE;
	int ret = 0;

	while (retry)
	{
		ret = stunSendRequest(dest, &gStunUserName, &gStunPassword, bindingChange);	

		if (SY_SUCCESS == ret)
		{
			stunHandleError(&gRespMsg, &gStunUserName, &gStunPassword, &retry);
			if (SY_TRUE == retry)
				continue;

//			if (SY_FAILED == stunVerifyMsg(BindResp))
//			{
//				VPrint("acs message verify failed, must discard\n");
//				return SY_FAILED;
//			}

			if (SY_TRUE == gRespMsg.hasMappedAddress)
			{
				//memcpy(&gMappedAddr, &gRespMsg.mappedAddress, sizeof(StunAtrAddress4));
				//compare with the local ip and port
				return stunCompAddr(&gRespMsg.mappedAddress, port);
			}
		} else {
			EPrint("stun send request failed\n");
			return SY_FAILED;
		}
	}

	return SY_FAILED;
}

//通知网管模块,有值改变
int
stunActiveNofify()
{
	VPrint("Enter stunActiveNofify !\n");
	char value[48]    = {0};
	char cNotify[8]   = {0};
	char cWritable[8] = {0};
	char cValueChangeBuf[248] = {0};
	char cValueChangeBufs[1024] = {0};
	char* names[3] = {"Device.ManagementServer.NATDetected",
					  "Device.ManagementServer.UDPConnectionRequestAddress",
					  NULL
					 };

	int i = 0;
	char* startValueBufPtr = cValueChangeBufs;
	while(names[i] != NULL)
	{
		memset(cNotify, 0, 8);
		memset(cWritable, 0, 8);
		memset(cValueChangeBuf, 0, 248);
		memset(value, 0, 48);
		SyGetNodeAttr(names[i], cNotify);
		SyGetNodeRW(names[i], cWritable);
		VPrint("Notify:%s Writable:%s\n", cNotify, cWritable);
		if (0 == strcmp(cNotify, "2") ) {
			// TODO: 这里判断 cWritable == R 是什么用意?
			// TODO: 难道还有 node 是 write only ?
			SyGetNodeValue(names[i], value);
			sprintf(cValueChangeBuf, "%s=%s\r\n", names[i], value);
			memcpy(startValueBufPtr, cValueChangeBuf, strlen(cValueChangeBuf));
		}           
		startValueBufPtr += strlen(cValueChangeBufs);
		++i;
	}

	DPrint("cValueChangeBufs:%s, len:%d.\n", cValueChangeBufs, strlen(cValueChangeBufs));
	if (0 != strlen(cValueChangeBufs)) {
		FILE* pFile = fopen(SY_VALUE_CHANGE_INFORM_FLAG_0, "wb");
		if (NULL != pFile) {
			fwrite(cValueChangeBufs, 1, strlen(cValueChangeBufs), pFile);
			fclose(pFile);
		} else {
			EPrint("open %s Error...\n", SY_VALUE_CHANGE_INFORM_FLAG_0);
			return SY_FAILED;
		}
	}

	return SY_SUCCESS;
}
/* bind discovery */
int
stunBindDisc(const StunAddress4 *dest)
{
	int ret = stunNatType(dest, SY_NAT_PRIMARY_PORT, SY_FALSE);
	int limit = SY_FALSE;
	gBindingChanged = SY_FALSE;

	VPrint("first Nat detected is %d\n", ret);
	if (SY_TRUE == ret) //the server must use STUN-based Approach	
	{
		//add by andy zhao 2014-4-8 13:13:38为了减少nat穿越所需时间
		NATDetected = SY_TRUE;
		if (0 == gPreNotifiedTime)
		{
			gPreNotifiedTime = time(NULL);
			limit = SY_FALSE;
			VPrint("gPreNotifiedTime == 0\n");
		}
		else if (SY_TRUE == gDiscoveryPhase)
		{
			UInt32 endTime = time(NULL); 
			if ((endTime - gPreNotifiedTime) >= gNotificationLimit)
			{
				limit = SY_FALSE;
				gPreNotifiedTime = endTime;
			}
			else
			{
				limit = SY_TRUE;
			}
		}

		// 映射的ip或者端口有改变
		if (!limit && (gRespMsg.mappedAddress.ipv4.addr != gMappedAddr.ipv4.addr ||
			 gRespMsg.mappedAddress.ipv4.port != gMappedAddr.ipv4.port))
		{
			memcpy(&gMappedAddr, &gRespMsg.mappedAddress, sizeof(StunAtrAddress4));

			WPrint("binding changed");
			stunSaveValue(SY_TRUE, SY_TRUE);
			gBindingChanged = SY_TRUE;

			if (gStunUserName.sizeofValue > 0 && gStunPassword.sizeofValue > 0)
			{
				VPrint("stun notify value");
				stunNatType(dest, SY_NAT_PRIMARY_PORT, gBindingChanged);
			}
			else
			{
				VPrint("tr069 notify value");
				stunActiveNofify();
			}
		}
	} 
	else {
		stunSaveValue(SY_FALSE, SY_FALSE);
	}

	return ret;
}

void 
stunSaveValue(int detected, int bindingChange)
{
	//assert(detected == SY_FALSE || detected == SY_TRUE);

	DO;

	DPrint("detected:%d, bindingChange:%d.\n", detected, bindingChange);
	char value[8] = {0};
	itoa(detected, value, 10);
	SySetNodeValue("Device.ManagementServer.NATDetected", value);

	UInt32 ip = gLocalIp;
	int port = SY_NAT_PRIMARY_PORT;

	if (SY_TRUE == detected)
	{
		if (SY_FALSE == gRespMsg.hasReflectedFrom && SY_TRUE == gRespMsg.hasMappedAddress)
		{
			ip = gMappedAddr.ipv4.addr;
			port = gMappedAddr.ipv4.port;
		}
	}
	else
		VPrint("NAT detected false, so use local address.\n");

	struct in_addr addr;
	ip = htonl(ip);
	char *net;
	memset(&addr, 0, sizeof(struct in_addr));
	memcpy(&addr, &ip, sizeof(UInt32));
	net = inet_ntoa(addr);

	char address[32] = {0};
	strncpy(address, net, strlen(net));
	int len = strlen(address);

	if (port != 0 && port != 80)
	{
		address[len] = ':';
		char *add = address + len + 1;
		itoa(port, add, 10);
	}

	VPrint("udp connection request address is %s\n", address);
	SySetNodeValue("Device.ManagementServer.UDPConnectionRequestAddress", address);
}
/* timeout discovery */
int
stunTimeoutDisc(const StunAddress4* dest, int *isBind)
{
	assert( dest->addr != 0);
	assert( dest->port != 0);

	DO;

	*isBind = SY_FALSE;

	if (INVALID_SOCKET == gSecFd)
	{
		gSecFd = openPort(SY_NAT_SECOND_PORT, 0);
	}
	assert(INVALID_SOCKET != gSecFd);

	int ret = stunSendDiscReq(dest);
	if (SY_SUCCESS == ret)
	{
		if (gRespType == BindResp)
		{
			*isBind = SY_TRUE;
			gRespType = NoneMsg;
		}
		return SY_SUCCESS;
	}

	return SY_FAILED;
}


