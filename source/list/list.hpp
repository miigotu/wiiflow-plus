#ifndef CLIST
#define CLIST

#include <ogcsys.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

#include "DeviceHandler.hpp"
#include <deque>
#include "wbfs_ext.h"
#include "libwbfs/libwbfs.h"
#include "disc.h"
#include "text.hpp"
#include "cache.hpp"

using namespace std;

class CList
{
    public:
		void GetPaths(vector<string> &pathlist, string containing, string directory, bool wbfs_fs = false);
		void GetHeaders(vector<string> pathlist, deque<dir_discHdr> &headerlist, string, string);
		void GetChannels(deque<dir_discHdr> &headerlist, string, u32, string);
};
#endif
