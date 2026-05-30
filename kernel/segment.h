/*
 * segment.h
 *
 * Copyright (C) 2003 by Simon Nieuviarts
 * Copyright (C) 2026 by Grégory Mounié (C23ification)
 *
 * Segment selectors.
 */

#ifndef __SEGMENT_H__
#define __SEGMENT_H__

constexpr unsigned short BASE_TSS = 0x08;
constexpr unsigned short KERNEL_CS = 0x10;	/* Kernel's PL0 code segment */
constexpr unsigned short KERNEL_DS = 0x18;	/* Kernel's PL0 data segment */
constexpr unsigned short USER_CS = 0x43;	/* User's code descriptor, RPL=3 */
constexpr unsigned short USER_DS = 0x4b;	/* User's data descriptor, RPL=3 */
constexpr unsigned short TRAP_TSS_BASE = 0x50;

#endif
