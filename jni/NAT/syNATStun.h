#ifndef _SYNATSTUN_H_
#define _SYNATSTUN_H_

#include <time.h>

// if you change this version, change in makefile too 
#define STUN_VERSION "0.97"

#define STUN_MAX_STRING 256
#define STUN_MAX_UNKNOWN_ATTRIBUTES 8
#define STUN_MAX_MESSAGE_SIZE 2048

#define STUN_DEFAULT_PORT 3478
#define STUN_TIMEOUT_RETRY   9
#define STUN_TIMEOUT_MAX_INTERVAL 1600

#define SY_NAT_PRIMARY_PORT    4000
#define SY_NAT_SECOND_PORT     4001
#define SY_NAT_LOCAL_MSG_PORT  4004

#define MAPPEDADDRESS     0x0001
#define RESPONSEADDRESS   0x0002
#define CHANGEREQUEST     0x0003
#define SOURCEADDRESS     0x0004
#define CHANGEDADDRESS    0x0005
#define USERNAME          0x0006
#define PASSWORD          0x0007
#define MESSAGEINTEGRITY  0x0008
#define ERRORCODE         0x0009
#define UNKNOWNATTRIBUTE  0x000A
#define REFLECTEDFROM     0x000B


// define some basic types
typedef unsigned char  UInt8;
typedef unsigned short UInt16;
typedef unsigned int   UInt32;
typedef unsigned long long UInt64;
typedef struct { unsigned char octet[16]; }  UInt128;
//typedef int Socket_t;

int gPriFd;
int gSecFd;
int gLocalMsgFd;
UInt32 gLocalIp;
int gBindingChanged;


typedef enum
{
	NoneMsg=0,
	BindResp,
	UdpReq,
} MessageType;

typedef struct 
{
	UInt16 msgType;
	UInt16 msgLength;
	UInt128 id;
} StunMsgHdr;


typedef struct
{
	UInt16 type;
	UInt16 length;
} StunAtrHdr;

typedef struct
{
      UInt16 port;
      UInt32 addr;
} StunAddress4;

typedef struct
{
      UInt8 pad;
      UInt8 family;
      StunAddress4 ipv4;
} StunAtrAddress4;

typedef struct
{
      UInt32 value;
} StunAtrChangeRequest;

typedef struct
{
      UInt16 pad; // all 0
      UInt8 errorClass;
      UInt8 number;
      char reason[STUN_MAX_STRING];
      UInt16 sizeReason;
} StunAtrError;

typedef struct
{
      UInt16 attrType[STUN_MAX_UNKNOWN_ATTRIBUTES];
      UInt16 numAttributes;
} StunAtrUnknown;

typedef struct
{
      char value[STUN_MAX_STRING];      
      UInt16 sizeofValue;
} StunAtrString;

typedef struct
{
      char hash[20];
} StunAtrIntegrity;

typedef enum 
{
   HmacUnkown=0,
   HmacOK,
   HmacBadUserName,
   HmacUnkownUserName,
   HmacFailed,
} StunHmacStatus;

typedef struct
{
      StunMsgHdr msgHdr;
	
      int hasMappedAddress;
      StunAtrAddress4  mappedAddress;
	
      int hasResponseAddress;
      StunAtrAddress4  responseAddress;
	
      int hasChangeRequest;
      StunAtrChangeRequest changeRequest;
	
      int hasSourceAddress;
      StunAtrAddress4 sourceAddress;
	
      int hasChangedAddress;
      StunAtrAddress4 changedAddress;
	
      int hasUsername;
      StunAtrString username;
	
      int hasPassword;
      StunAtrString password;
	
      int hasMessageIntegrity;
      StunAtrIntegrity messageIntegrity;
	
      int hasErrorCode;
      StunAtrError errorCode;
	
      int hasUnknownAttributes;
      StunAtrUnknown unknownAttributes;
	
      int hasReflectedFrom;
      StunAtrAddress4 reflectedFrom;

	  int hasRequstBinding;
	  StunAtrString connectionRequestBinding;

	  int hasBindingChange;
	  StunAtrString bindingChange;

//      int hasXorMappedAddress;
//      StunAtrAddress4  xorMappedAddress;
//	
//      int xorOnly;
//
//      int hasServerName;
//      StunAtrString serverName;
//      
//      int hasSecondaryAddress;
//      StunAtrAddress4 secondaryAddress;
} StunMessage; 


// Define enum with different types of NAT 
typedef enum 
{
   StunTypeUnknown=0,
   StunTypeFailure,
   StunTypeOpen,
   StunTypeBlocked,

   StunTypeIndependentFilter,
   StunTypeDependentFilter,
   StunTypePortDependedFilter,
   StunTypeDependentMapping,

   //StunTypeConeNat,
   //StunTypeRestrictedNat,
   //StunTypePortRestrictedNat,
   //StunTypeSymNat,
   
   StunTypeFirewall,
} NatType;


#define MAX_MEDIA_RELAYS 500
#define MAX_RTP_MSG_SIZE 1500
#define MEDIA_RELAY_TIMEOUT 3*60

typedef struct 
{
      int relayPort;       // media relay port
      int fd;              // media relay file descriptor
      StunAddress4 destination; // NAT IP:port
      time_t expireTime;      // if no activity after time, close the socket 
} StunMediaRelay;

typedef struct
{
      StunAddress4 myAddr;
      StunAddress4 altAddr;
      int myFd;
      int altPortFd;
      int altIpFd;
      int altIpPortFd;
      int relay; // true if media relaying is to be done
      StunMediaRelay relays[MAX_MEDIA_RELAYS];
} StunServerInfo;


//StunAtrAddress4 gMappedAddr;

int
stunParseMessage( const char* buf,unsigned int bufLen,
                  StunMessage* message);

void
stunBuildReqSimple( StunMessage* msg,
                    const StunAtrString* username,
                    int bindingChange, unsigned int id);

unsigned int
stunEncodeMsg( const StunMessage* message, 
                   char* buf, 
                   unsigned int bufLen, 
                   const StunAtrString* password);

int 
stunRand();

UInt64
stunGetSystemTimeSecs();

/// find the IP address of a the specified stun server - return false is fails parse 
int  
stunParseSrvName( char* serverName, StunAddress4* stunServerAddr, int port);

int 
stunParseHstName( char* peerName,
                   UInt32* ip,
                   UInt16* portVal,
                   UInt16 defaultPort );

/// returns number of address found - take array or addres 
int 
stunFindLocalInterfaces(UInt32 addresses, int maxSize);

#if 0
int
stunOpenSocket( StunAddress4* dest, 
                StunAddress4* mappedAddr, 
                int port, 
                StunAddress4* srcAddr);

int
stunOpenSocketPair( StunAddress4* dest, StunAddress4* mappedAddr, 
                    int* fd1, int* fd2, 
                    int srcPort,  StunAddress4* srcAddr);
#endif

int
stunRandomPort();

int
stunSendDiscReq(const StunAddress4* dest);

int 
stunVerifyMsg(const char *buf, MessageType type);
int
stunRecvMsgThd();

int 
stunSendRequest(const StunAddress4* dest, const StunAtrString* username, 
					const StunAtrString* password, int bindingChange);

int
stunBindDisc(const StunAddress4 *dest);

int
stunNatType(const StunAddress4* dest, int port, int bindingChange);

void 
stunSaveValue(int detected, int bindingChange);

int
stunTimeoutDisc(const StunAddress4* dest, int *isBind);

int
stunActiveNofify();
#endif /* _SYNATSTUN_H_ */

/* ====================================================================
 * The Vovida Software License, Version 1.0 
 * 
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * ====================================================================
 * 
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */

// Local Variables:
// mode:c++
// c-file-style:"ellemtel"
// c-file-offsets:((case-label . +))
// indent-tabs-mode:nil
// End:

