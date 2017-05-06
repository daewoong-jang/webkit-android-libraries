/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
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
#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

#define __need_size_t
#define __need_ptrdiff_t
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>

#include <linux/posix_types.h>
#include <asm/types.h>

typedef __u32    __kernel_dev_t;

typedef __kernel_clock_t     clock_t;
typedef __kernel_dev_t       dev_t;
typedef __kernel_ino_t       ino_t;
typedef __kernel_mode_t      mode_t;
#ifndef _OFF_T_DEFINED_
#define _OFF_T_DEFINED_
typedef __kernel_off_t       off_t;
#endif
typedef __kernel_loff_t      loff_t;
typedef loff_t               off64_t;  /* GLibc-specific */

typedef __kernel_pid_t		 pid_t;

#ifndef _SSIZE_T_DEFINED_
#define _SSIZE_T_DEFINED_
typedef int  ssize_t;
#endif

typedef __kernel_uid32_t        uid_t;

typedef __kernel_timer_t	timer_t;

#include <win/sys/types.h>

#endif
