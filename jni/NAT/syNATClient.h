#ifndef SYNATCLIENT_H
#define SYNATCLIENT_H

#include<stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <assert.h>
#define NAT_DEBUG

int
isBehindNAT();

int
discoveryNAT(char* stunUrl, int stunPort);

void traverseNAT();

int
sendBindReqPeriod(unsigned int timeout);
#endif
