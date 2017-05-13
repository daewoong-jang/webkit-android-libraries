/*
 * Copyright (C) 2013 NAVER Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _WIN_SYS_CDEFS_H_INCLUDED_
#define _WIN_SYS_CDEFS_H_INCLUDED_

#if defined(__cplusplus)
#define	EXTERN_C            extern "C"
#define	__BEGIN_DECLS       extern "C" {
#define	__END_DECLS         }
#define	__static_cast(x,y)  static_cast<x>(y)
#else
#define	EXTERN_C            extern
#define	__BEGIN_DECLS
#define	__END_DECLS
#define	__static_cast(x,y)	(x)y
#endif

#if defined(_M_IX86)
#define __i386__ 1
#endif
#if defined(_M_X64)
#define __x86_64__ 1
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#undef  _WINSOCKAPI_

#define _XKEYCHECK_H
#define _WINSOCKAPI_
#define _WINSOCK2API_
#define _WINSOCK2_H_NOT_INCLUDED_
#include <sdkddkver.h>
#ifndef _WIN32_WINNT_WINTHRESHOLD
#define _WIN32_WINNT_WINTHRESHOLD   0x0A00
#endif
#ifndef NTDDI_WIN7SP1
#define NTDDI_WIN7SP1               0x06010100
#endif
#include <windows.h>
#ifndef far
#define far
#endif
#ifndef near
#define near
#endif
#ifndef NOUSER
#define DOUBLE COMDOUBLE
#include <unknwn.h>
#undef DOUBLE
#endif
#include <timeapi.h>
#include <linux/limits.h>

#define alignof   __alignof

#define __attribute__(x)

#include <android/api-level.h>

#endif // _WIN_SYS_CDEFS_H_INCLUDED_
