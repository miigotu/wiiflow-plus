#ifndef CLIST
#define CLIST

#include <ogcsys.h>
#include <dirent.h>
#include <sys/types.h> 
#include <sys/param.h> 
#include <sys/stat.h> 
#include <unistd.h> 

#include "DeviceHandler.hpp"
#include "safe_vector.hpp"
#include "wbfs_ext.h"
#include "libwbfs/libwbfs.h"
#include "disc.h"
#include "text.hpp"
#include "cache.hpp"

typedef bool (*findtitle_callback_t)(void *callback_data, u8 *id, char *title, int size);

using namespace std;
template <typename T>
class CList
{
    public:
		 CList(){};
		~CList(){};
		void CountGames(string directory, bool wbfs_fs, u32 *cnt);
		void GetPaths(safe_vector<string> &pathlist, string containing, string directory, bool wbfs_fs = false);
		void GetHeaders(safe_vector<string> pathlist, safe_vector<T> &headerlist, findtitle_callback_t callback = NULL, void *callback_data = NULL);
	private:
		void Check_For_ID(u8 *id, string path, string one, string two);

};
#endif
