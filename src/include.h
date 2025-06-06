#pragma once

// avoid deprecated warnings for many string functions in VC++ 2005
#pragma warning(disable:4996)

/** include all the necessary standard C and C++ libraries */
#include <iostream>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <assert.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <list>
#include <fstream>
#include <typeinfo>
#include <numeric>

using namespace std;

/** Predefine classes that have circular dependencies */
class SimContext;
class Tracer;
class RunStats;
class CostStats;
class SummaryStats;
class Patient;

#include "SimContext.h"
#include "Tracer.h"
#include "RunStats.h"
#include "CostStats.h"
#include "SummaryStats.h"
#include "StateUpdater.h"
#include "BeginWeekUpdater.h"
#include "EndWeekUpdater.h"
//#include "WeeklyUpdaters.h"
#include "CMVInfectionUpdater.h"
#include "PrenatalTestUpdater.h"
#include "MaternalBiologyUpdater.h"
#include "PregnancyUpdater.h"
#include "Patient.h"

#include "mtrand.h"
#include "CmvUtil.h"



/** Include platofrm specific header files */
#if defined(_LINUX)
    #include <sys/io.h>
#endif
#if defined(__APPLE__)
	#include <unistd.h>
	#include <mach-o/dyld.h>
#endif
#if defined(_WIN32)
	#include <io.h>
	#include <direct.h>	//for _mkdir and _CHDIR
#else
	#include <sys/io.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	//used to find files in a directory
	#include <glob.h>
#endif

