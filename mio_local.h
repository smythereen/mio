/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/* Copyright (c) 2026 John R. Smythe Jr. */
#ifndef MIO_LOCAL_H
#define MIO_LOCAL_H
#include <mem_io.h>
/* Private dispatch tables; use MioRegisterHandler(), not direct table writes. */
extern MioItemHandler MioValueIn[ MAX_MTYPE + 1 ];
extern MioItemHandler MioValueOut[ MAX_MTYPE + 1 ];
#endif
