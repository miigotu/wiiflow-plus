#include <ogcsys.h>

#include "d2x.h"
#include "sys.h"
#include "mload_modules.h"
#include "gecko.h"

bool disable_return_to_patch = false;

s32 is_ios_d2x()
{
        int ios_rev = IOS_GetRevision();
        if (is_ios_type(IOS_TYPE_WANIN)) {
                if (ios_rev > 21000 && ios_rev < 25000) {
                        int rev = ios_rev % 100;
                        return rev;
                }
        }
        return 0;
}

void block_ios_reload()
{
	if (is_ios_type(IOS_TYPE_HERMES)) {
		disableIOSReload();
	} else if (is_ios_d2x() >= 5) {
                // d2x ios reload block
                int es_fd = IOS_Open("/dev/es", 0);
                if (es_fd < 0) {
                        gprintf("Couldn't open ES module\n");
                        return;
                }
                static ioctlv vector[0x08] ATTRIBUTE_ALIGN(32);
                static int mode ATTRIBUTE_ALIGN(32);
                static int ios ATTRIBUTE_ALIGN(32);
                mode = 2;
                ios = IOS_GetVersion();
                vector[0].data = &mode;
                vector[0].len = 4;
                vector[1].data = &ios;
                vector[1].len = 4;
                int ret = IOS_Ioctlv(es_fd, 0xA0, 2, 0, vector);
                IOS_Close(es_fd);
                gprintf("d2x ios reload block %d: %d\n", ios, ret);
                if (ret < 0) {
                        gprintf("d2x IOS reload block FAILED!\n");
                }
	} else {
		gprintf("IOS Reload Block is not supported with this IOS\n");
	}
}

u8 return_to_channel(u32 id)
{
	gprintf("d2x_return_to_channel %08x %d\n", id, is_ios_d2x());
	if (!id) return 0;
	if (is_ios_d2x() < 4) return 0;	

        static u64 sm_title_id  ATTRIBUTE_ALIGN(32);
        // title id to be launched in place of the system menu
        sm_title_id = (0x00010001ULL << 32) | id;

	int ret;
	signed_blob *buf;
	u32 filesize;

        // Check if the title exists NeoGamma wants the cIOS to return to
	extern s32 GetTMD(u64 TicketID, signed_blob **Output, u32 *Length);
        ret = GetTMD(sm_title_id, &buf, &filesize);
	if (buf != NULL) free(buf);

	if (ret < 0) return 0;

	static ioctlv vector[0x08] ATTRIBUTE_ALIGN(32);
	vector[0].data = &sm_title_id;
	vector[0].len = 8;

        int es_fd = IOS_Open("/dev/es", 0);
        if (es_fd < 0)
        {
                gprintf("Couldn't open ES module(2)\n");
                return 0;
        }
	ret = IOS_Ioctlv(es_fd, 0xA1, 1, 0, vector);
	IOS_Close(es_fd);

	gprintf("d2x return to channel (%08x %d)\n", id, ret);

	if (ret >= 0)
	{
		disable_return_to_patch = true;
	}
	return 1;
}

