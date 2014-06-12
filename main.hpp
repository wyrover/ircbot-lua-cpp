#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>
#include <iterator>
#include <sstream>
#include <string.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include <inttypes.h>
#include <cinttypes>
#include <time.h>

#define GCC_VERSION (__GNUC__ * 10000 \
    + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#if GCC_VERSION >= 40300
#include <unordered_map>
#define hash_map unordered_map

#else
#include <ext/hash_map>
using namespace __gnu_cxx;
#endif

#define TRACE_MSG
//#define TRACE_MSG fprintf(stderr, "(%s) [%s:%d]\n", __PRETTY_FUNCTION__, __FILE__, __LINE__)

using namespace std;
