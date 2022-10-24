/*
 * bbs-fw
 *
 * Copyright (C) Daniel Nilsson, 2022.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _VERSION_H_
#define _VERSION_H_

#define VERSION_MAJOR		1
#define VERSION_MINOR		3
#define VERSION_PATCH		0


#if defined(BBSHD)
	#define CTRL_TYPE		1
#elif defined(BBS02)
	#define CTRL_TYPE		2
#elif defined(TSDZ2)
	#define CTRL_TYPE		3
#else
	#define CTRL_TYPE		0
#endif

#endif
