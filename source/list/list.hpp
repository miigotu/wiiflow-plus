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
//#include "gecko.h"

using namespace std;

class CList
{
    public:
		static CList * Instance();
		static void DestroyInstance();
		void GetPaths(safe_vector<string> &pathlist, string containing, string directory);
		void GetHeaders(safe_vector<string> pathlist, safe_vector<dir_discHdr> &headerlist);
		void Init(string cachedir){m_cacheDir = cachedir;}
	private:
		bool update;
		bool wbfs_fs;
		string m_cacheDir;
		string m_database;
		void Check_For_ID(u8 *id, string path, string one, string two);

	protected:
		 CList(): update(false), wbfs_fs(false) {};
		~CList();

		static CList * instance;
};

#endif
