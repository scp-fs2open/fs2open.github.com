#pragma once

// This header is used for compiling the precompiled header file that is included in the entire code project

// Standard C headers
#include <cstdio>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cinttypes>
#include <cassert>
#include <cstdarg>

// Standard C++ headers
#include <algorithm>
#include <array>
#include <bitset>
#include <condition_variable>
#include <csetjmp>
#include <cstdarg>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// For legacy CMake version we use a different kind of PCH handling which does not handle internal include paths
// properly so the FSO specific headers are only included for the CMake native PCH support
#ifdef CMAKE_PCH
// FSO specific headers
#include "globalincs/pstypes.h"
#endif
