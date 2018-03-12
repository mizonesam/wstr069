/***********************************************************************
*
* base64.h
*
* Copyright (C) 2013 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2013 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#ifndef BASE54_H
#define BASE54_H

long base64_encode(char *to, char *from, unsigned int len);
long base64_decode(char *to, char *from, unsigned int len);

#endif
