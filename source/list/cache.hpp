#ifndef CCACHE
#define CCACHE

#include <sys/types.h>
#include <ogcsys.h>
#include <fstream>
#include <deque>
#include "disc.h"

//#include "gecko.h"
using namespace std;

const char io[4][3] = {
	"wb",
	"rb"
};

enum CMode
{
	SAVE,
	LOAD,
};

class CCache
{
	public:
		 CCache(deque<dir_discHdr> &list, string path, CMode mode);								/* Load/Save All */
		~CCache();
	private:
		void SaveAll(deque<dir_discHdr> list);
		void LoadAll(deque<dir_discHdr> &list);
		void LoadOne(dir_discHdr &tmp, u32 index);

		FILE *cache;
		string filename;
};
#endif