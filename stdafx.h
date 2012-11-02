// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <string.h>

// TODO: reference additional headers your program requires here
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include "mongoose.h"
#include "sqlite3.h"

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 \
							+ __GNUC_MINOR__ * 100 \
							+ __GNUC_PATCHLEVEL__)
#endif
