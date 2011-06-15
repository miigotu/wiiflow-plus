#include "ios_base.h"

u32 hashes[info_number][5] = {
{0xafbfc2c1, 0x28c59142, 0x953b1c6c, 0x809a984f, 0x427c9270}, // 222 38 v4
{0xb72b71cd, 0xf42b2730, 0x3b9a4f2c, 0x41128ef9, 0x26f6dbcc}, // 222 38+37 v4
{0xd6f1472f, 0x68740b4c, 0xbdf0078d, 0xb8ebb00a, 0x8c9afe2b}, // 222 38+60 v4
// v5.0
{0xfa8ad097, 0x6c18542a, 0x5691bdec, 0xd0c47a6a, 0xbb857b08}, // 222 38 v5.0
{0x07b9c8f2, 0xa0dbad4d, 0xa6ee0761, 0x7c371591, 0x4e4c63ec}, // 223 37 v5.0
{0x0d0a12e0, 0x16065574, 0x844e39b4, 0x2d2dbdf1, 0x5b497869}, // 223 57 v5.0
{0x0aa83bf0, 0x8fbd610f, 0x87bae3c1, 0x06f43826, 0x39524429}, // 223 60 v5.0
// v5.1
{0xf865dfa5, 0xa909e4fb, 0x345f48ea, 0x804f5a64, 0x3704dd5a}, // 222 38 v5.1
{0x889511aa, 0xdde5c849, 0xe30e6d20, 0x9687c95a, 0xb935342b}, // 223 37 v5.1
{0x0584100d, 0xc3364080, 0xc3b8ffd0, 0x8c351aee, 0xf4632159}, // 223 57 v5.1
{0x1cb7981c, 0xd791a2bf, 0x736395d6, 0x0224e181, 0x38324674}, // 223 60 v5.1
// modmii special
{0x00dc1209, 0x944c39db, 0x59eec2ab, 0x0212b86c, 0x7076cd3b}, // 249 56 r21+r19 modmii
{0x00298dc2, 0x58fdae1a, 0x233b0958, 0x17269047, 0x8188633d}, // 249 57 r21+r19 modmii
// d2x v1
{0x00ed2993, 0x0bae0cb2, 0xc7e430a2, 0x2e6eaf18, 0x156a9a70}, // 249 37 r21-d2x-v1
{0x00d74607, 0x2d3fe23e, 0x47ecb019, 0x0f5d4380, 0x37ea6b50}, // 249 38 r21-d2x-v1
{0x003d11ce, 0x4eb3b8bb, 0xe2c02fda, 0x5f879e74, 0x44a257de}, // 249 56 r21-d2x-v1
{0x00ba4b4f, 0x27803366, 0x8d9121fa, 0x954eb5d5, 0x92242691}, // 249 57 r21-d2x-v1
// d2x v2
{0x00475dce, 0x81a744dd, 0xf24157e4, 0x870fa3d8, 0xfc39fa8a}, // 249 37 r21-d2x-v2
{0x00711af6, 0x017c48d4, 0xea0267d3, 0x1666600b, 0x38a8fe16}, // 249 38 r21-d2x-v2
{0x00815782, 0x8604fe34, 0x474653b5, 0xbdbc5651, 0xf43b427a}, // 249 56 r21-d2x-v2
{0x00d8e857, 0x8c96eb52, 0x4d006568, 0x95cf5415, 0xdb7712e8}, // 249 57 r21-d2x-v2
// d2x v3
{0x0054e91c, 0xe022e307, 0x26d72e03, 0x53b6e157, 0x42adbe49}, // 249 37 r21-d2x-v3
{0x000bd035, 0xe649cc22, 0x8bf647c5, 0xe0710e6a, 0xd79a5355}, // 249 38 r21-d2x-v3
{0x00b8ca9c, 0x9b4053a3, 0x8de94a72, 0x1192fce5, 0x098e7404}, // 249 56 r21-d2x-v3
{0x00e8e05f, 0x2aa4cd1e, 0x8c8f5529, 0x498f259b, 0xfa41258e}, // 249 57 r21-d2x-v3
{0x0028dbf1, 0x3827be46, 0x28c82eb2, 0x122325c3, 0xc72dbd46}, // 249 58 r21-d2x-v3
// d2x v3 (r21003)
{0x006ec958, 0xbc59364d, 0x5b2f58a0, 0xf8feeac4, 0x89a0b697}, // 249 37 r21-d2x-v3
{0x00fe6ad4, 0xbbf9a5e2, 0xeb2b0110, 0xc1fddbdf, 0xfb634350}, // 249 38 r21-d2x-v3
{0x00ecc3a8, 0xec2d3b64, 0x506314e3, 0x740274ef, 0x6505cc75}, // 249 56 r21-d2x-v3
{0x008e68fd, 0xa1221185, 0xc09a1a26, 0xfeb09ead, 0xf375c2f2}, // 249 57 r21-d2x-v3
{0x006237ad, 0xda4cb0e1, 0xa97e4b41, 0xf1bb24a2, 0xd663b7f7}, // 249 58 r21-d2x-v3
// d2x v4 (r21004) beta 2
{0x00733fa4, 0x1d3e6245, 0xb0311e24, 0x675868b1, 0x353d882c}, // 249 37 r21-d2x-v4-b2
{0x007b9fca, 0x0a6f40c5, 0xccd37b25, 0x1c49064b, 0x1041ddb3}, // 249 38 r21-d2x-v4-b2
{0x000d243c, 0x07a183df, 0x0592ce22, 0x2bb81b46, 0x64cce4a7}, // 249 56 r21-d2x-v4-b2
{0x00c7b39a, 0xed42a4a0, 0xcc125669, 0xf23c1f6e, 0x2244cb9b}, // 249 57 r21-d2x-v4-b2
{0x00120cb0, 0x4cb9b4b1, 0xbe02e0e6, 0x30bfcb95, 0xfbfcaba5}, // 249 58 r21-d2x-v4-b2
// d2x v4 (r21004) release / hb installer
{0xba124a8e, 0x73f5b2cb, 0x5e2439be, 0x76629335, 0xa3f36418}, // 249 37 r21-d2x-v4
{0x2e5b15a4, 0x6e638735, 0x6d3d7403, 0xa78cdcc0, 0xe62a106b}, // 249 38 r21-d2x-v4
{0xf13c731c, 0x3d77c021, 0x48c3119f, 0x7679939f, 0xbde8857f}, // 249 53 r21-d2x-v4
{0x292464d3, 0xf94267d3, 0x849fd15c, 0x03200bde, 0xe8e0d6e8}, // 249 55 r21-d2x-v4
{0xfefb9f74, 0x9961dd76, 0xe5416e0f, 0x7df6a95b, 0x923d2561}, // 249 56 r21-d2x-v4
{0xb7272b2f, 0x72bdab83, 0x31d0639f, 0xfabc719d, 0xad818d91}, // 249 57 r21-d2x-v4
{0x91a7d59f, 0x4ae0671a, 0x9bdf2593, 0xf7535426, 0x85af4073}, // 249 58 r21-d2x-v4
// d2x v4 (r21004) release / modmii
{0x00de8cad, 0x17183381, 0x889a1299, 0x834a85eb, 0x45b59e05}, // 249 37 r21-d2x-v4
{0x0007e951, 0x173de10f, 0x0324b33f, 0xaa1f93c7, 0x28461fbe}, // 249 38 r21-d2x-v4
{0x00f92f4f, 0x8f989389, 0xcd9e2732, 0x7752e50b, 0xa47fde40}, // 249 53 r21-d2x-v4
{0x0056c457, 0x99c9a90f, 0xf0d9d994, 0x0724362a, 0xbe8ac29f}, // 249 55 r21-d2x-v4
{0x00a1872f, 0x412f94cf, 0xd90c818b, 0xde15681e, 0x63c41b52}, // 249 56 r21-d2x-v4
{0x00b06c85, 0xab7a94c2, 0x674785fc, 0x8f133335, 0xc9b84d49}, // 249 57 r21-d2x-v4
{0x000530f4, 0x0c472b29, 0xb8f22f5a, 0x752b0613, 0x109bace1}, // 249 58 r21-d2x-v4
// modmii 249
{0x005b6439, 0xf4a2e0b7, 0xfce05f75, 0xdb1a66ce, 0x7a0811c1}, // 249 38 r17 modmii	
{0x007da65a, 0x1b57b279, 0x06a5443f, 0xc61fd6cb, 0x4ea9866a}, // 249 37 r19 modmii
{0x00cb38fc, 0x3abc550f, 0xc128f0aa, 0xa1dc96b8, 0x3d3ed542}, // 249 38 r19 modmii
{0x00cc7ee1, 0xc1af6682, 0x8a5a2b6f, 0xc417eb3d, 0x607377ec}, // 249 57 r19 modmii
{0x00e96e3f, 0xe861fd7a, 0x092d0fcb, 0xa65af414, 0xd375d6bb}, // 249 38 r20 modmii 
{0x009e5dde, 0x2589d21d, 0x4db9dfaa, 0x765c4279, 0xc4a5ba75}, // 249 56 r20 modmii
{0x003c1bd8, 0x7830d7dc, 0x79e74949, 0x9609bb13, 0x4b5c5072}, // 249 57 r20 modmii
{0x000f5724, 0xe6002c66, 0x6718313c, 0x8c4ec895, 0x478480ce}, // 249 37 r21 modmii
{0x00c61cad, 0x30328d5d, 0xe69eb487, 0x27f77d5e, 0xc3c47d0d}, // 249 38 r21 modmii
{0x00994d20, 0xbe74e78b, 0x00d4f00c, 0xfc9da208, 0x262c5f90}, // 249 56 r21 modmii
{0x009fb539, 0x5a56f778, 0x329fbfd7, 0xc3a8ff58, 0x6fdb010b}, // 249 57 r21 modmii
{0x002a7dfe, 0xdc36d6d9, 0x9af35191, 0x54862ecb, 0xd8087cb3}, // 249 58 r21 modmii
// modmii 250
{0x00186ce5, 0xe6ced602, 0x552e621d, 0xaf882fb8, 0xa479e47b}, // 250 37 r19 modmii
{0x00e2ea30, 0x56c4568d, 0x4f2c165d, 0xc00471bd, 0x6939c9f1}, // 250 38 r19 modmii
{0x005849db, 0xa1fc4519, 0x530a963b, 0x31810e9e, 0xea1f207a}, // 250 57 r19 modmii
{0x00641c3c, 0xa7346fa0, 0x0fa518a6, 0xeeac8097, 0x60eb2e87}, // 250 38 r20 modmii
{0x009c97f0, 0xda7da42e, 0xd3320862, 0xb33d22db, 0xdc80b9e2}, // 250 56 r20 modmii
{0x006d76ce, 0x0a294191, 0x62c9705d, 0x355ec87b, 0xff152dd5}, // 250 57 r20 modmii
{0x006937f9, 0x8fdccb08, 0x4f9396b3, 0xc91b8761, 0x8ff1f1bb}, // 250 37 r21 modmii
{0x004ba2f2, 0x9e4269c6, 0xe5d91fd0, 0x2e6410db, 0x772b1986}, // 250 38 r21 modmii
{0x00bca8d8, 0x82105397, 0xb140ddb2, 0x8774d57a, 0x66418504}, // 250 56 r21 modmii
{0x000ac379, 0x719a8850, 0x469445e7, 0xcf51108e, 0xb832d628}, // 250 57 r21 modmii
{0x008567af, 0xe41cdb0b, 0xce85dc29, 0x10970d12, 0xe0b608f3}, // 250 58 r21 modmii
// modmii hermes
{0x00ec3a7b, 0xc9869c77, 0x013cf962, 0x4eef5726, 0xda9d1488}, // 222 38 v4 modmii
{0x0073251f, 0x88b53db8, 0x390234af, 0x26910ff6, 0x041f3d3f}, // 223 38+37 v4 modmii
{0x00d801b3, 0xe280e6e2, 0x1c99b236, 0x470ed5a9, 0xf3544f86}, // 222 38 v5.0 modmii
{0x007d38f5, 0xb6b841b4, 0xf57579db, 0xa47526fe, 0xc3b3d12a}, // 223 37 v5.0 modmii
{0x00f08071, 0x2672d68b, 0xc63bed5a, 0x06ae3b3c, 0xcff913d7}, // 223 57 v5.0 modmii
// Waninkoko
{0x074dfb39, 0x90a5da61, 0x67488616, 0x68ccb747, 0x3a5b59b3}, // IOS249 r21 Base: 36
{0x6956a016, 0x59542728, 0x8d2efade, 0xad8ed01e, 0xe7f9a780}, // IOS249 r21 Base: 37
{0xdc8b23e6, 0x9d95fefe, 0xac10668a, 0x6891a729, 0x2bdfbca0}, // IOS249 r21 Base: 38
{0xaa2cdd40, 0xd628bc2e, 0x96335184, 0x1b51404c, 0x6592b992}, // IOS249 r21 Base: 53
{0x4a3d6d15, 0x014f5216, 0x84d65ffe, 0x6daa0114, 0x973231cf}, // IOS249 r21 Base: 55
{0xca883eb0, 0x3fe8e45c, 0x97cc140c, 0x2e2d7533, 0x5b369ba5}, // IOS249 r21 Base: 56
{0x469831dc, 0x918acc3e, 0x81b58a9a, 0x4493dc2c, 0xaa5e57a0}, // IOS249 r21 Base: 57
{0xe5af138b, 0x029201c7, 0x0c1241e7, 0x9d6a5d43, 0x37a1456a}, // IOS249 r21 Base: 58
{0x0fdee208, 0xf1d031d3, 0x6fedb797, 0xede8d534, 0xd3b77838}, // IOS249 r21 Base: 60
{0xaf588570, 0x13955a32, 0x001296aa, 0x5f30e37f, 0x0be91316}, // IOS249 r21 Base: 61
{0x50deaba2, 0x9328755c, 0x7c2deac8, 0x385ecb49, 0x65ea3b2b}, // IOS249 r21 Base: 70
{0x811b6a0b, 0xe26b9419, 0x7ffd4930, 0xdccd6ed3, 0x6ea2cdd2}, // IOS249 r21 Base: 80
{0x30aeadfe, 0x8b6ea668, 0x446578c7, 0x91f0832e, 0xb33c08ac}, // cIOS36 rev20
{0xba0461a2, 0xaa26eed0, 0x482c1a7a, 0x59a97d94, 0xa607773e}, // cIOS37 rev20
{0xb694a33e, 0xf5040583, 0x0d540460, 0x2a450f3c, 0x69a68148}, // cIOS38 rev20
{0xf6058710, 0xfe78a2d8, 0x44e6397f, 0x14a61501, 0x66c352cf}, // cIOS53 rev20
{0xfa07fb10, 0x52ffb607, 0xcf1fc572, 0xf94ce42e, 0xa2f5b523}, // cIOS55 rev20
{0xe30acf09, 0xbcc32544, 0x490aec18, 0xc276cee6, 0x5e5f6bab}, // cIOS56 rev20
{0x595ef1a3, 0x57d0cd99, 0x21b6bf6b, 0x432f6342, 0x605ae60d}, // cIOS57 rev20
{0x687a2698, 0x3efe5a08, 0xc01f6ae3, 0x3d8a1637, 0xadab6d48}, // cIOS60 rev20
{0xea6610e4, 0xa6beae66, 0x887be72d, 0x5da3415b, 0xa470523c}, // cIOS61 rev20
{0x64e1af0e, 0xf7167fd7, 0x0c696306, 0xa2035b2d, 0x6047c736}, // cIOS70 rev20
{0x0df93ca9, 0x833cf61f, 0xb3b79277, 0xf4c93cd2, 0xcd8eae17}, // cIOS80 rev20
{0x0a49cd80, 0x6f8f87ff, 0xac9a10aa, 0xefec9c1d, 0x676965b9}, // cIOS37 rev19
{0x09179764, 0xeecf7f2e, 0x7631e504, 0x13b4b7aa, 0xca5fc1ab}, // cIOS38 rev19
{0x6010d5cf, 0x396415b7, 0x3c3915e9, 0x83ded6e3, 0x8f418d54}, // cIOS57 rev19
{0x589d6c4f, 0x6bcbd80a, 0xe768f258, 0xc53a322c, 0xd143f8cd}, // cIOS60 rev19
{0x8969e0bf, 0x7f9b2391, 0x31ecfd88, 0x1c6f76eb, 0xf9418fe6}, // cIOS70 rev19
{0x3c968e54, 0x9e915458, 0x9ecc3bda, 0x16d0a0d4, 0x8cac7917}, // cIOS37 rev18
{0xe811bca8, 0xe1df1e93, 0x779c40e6, 0x2006e807, 0xd4403b97}, // cIOS38 rev18
{0x697676f0, 0x7a133b19, 0x881f512f, 0x2017b349, 0x6243c037}, // cIOS57 rev18
{0x34ec540b, 0xd1fb5a5e, 0x4ae7f069, 0xd0a39b9a, 0xb1a1445f}, // cIOS60 rev18
{0xd98a4dd9, 0xff426ddb, 0x1afebc55, 0x30f75489, 0x40b27ade}, // cIOS70 rev18
{0x620c57c7, 0xd155b67f, 0xa451e2ba, 0xfb5534d7, 0xaa457878}, // cIOSrev13b
{0x20e60607, 0x4e02c484, 0x2bbc5758, 0xee2b40fc, 0x35a68b0a}  // cIOSrev13a
};

char bases[info_number][6] = { 
{"38"}, {"38+37"}, {"38+60"},
// v5.0
{"38"}, {"37"}, {"57"}, {"60"},
// v5.1
{"38"}, {"37"}, {"57"}, {"60"},
// modmii special
{"56"}, {"57"},
// d2x v1
{"37"}, {"38"}, {"56"}, {"57"},
// d2x v2
{"37"}, {"38"}, {"56"}, {"57"},
// d2x v3
{"37"}, {"38"}, {"56"}, {"57"}, {"58"},
// d2x v3 (r21003)
{"37"}, {"38"}, {"56"}, {"57"}, {"58"},
// d2x v4 (r21004) beta 2
{"37"}, {"38"}, {"56"}, {"57"}, {"58"},
// d2x v4 (r21004) release / hb installer
{"37"}, {"38"}, {"53"}, {"55"}, {"56"}, {"57"}, {"58"},
// d2x v4 (r21004) release / modmii
{"37"}, {"38"}, {"53"}, {"55"}, {"56"}, {"57"}, {"58"},
// modmii 249
{"38"},	{"37"}, {"38"}, {"57"}, {"38"}, {"56"},
{"57"}, {"37"}, {"38"}, {"56"}, {"57"}, {"58"},
// modmii 250
{"37"}, {"38"}, {"57"}, {"38"}, {"56"}, {"57"},
{"37"}, {"38"}, {"56"}, {"57"}, {"58"}, 
// modmii hermes
{"38"}, {"38+37"}, {"38"}, {"37"}, {"57"},
{"36"}, {"37"}, {"38"}, {"53"}, {"55"}, {"56"}, {"57"}, {"58"}, {"60"}, {"61"}, {"70"}, {"80"}, 	// rev 21
{"36"}, {"37"}, {"38"}, {"53"}, {"55"}, {"56"}, {"57"}, {"60"}, {"61"}, {"70"}, {"80"}, 			// rev 20
{"37"}, {"38"}, {"57"}, {"60"}, {"70"}, 																// rev 19
{"37"}, {"38"}, {"57"}, {"60"}, {"70"},																// rev 18
{"??"}, {"??"}																							// rev 13
};

char revs[info_number][16] = {
{"v4"},
{"v4"},
{"v4"},
// v5.0
{"v5.0"},
{"v5.0"},
{"v5.0"},
{"v5.0"},
// v5.1
{"v5.1"},
{"v5.1"},
{"v5.1"},
{"v5.1"},
// modmii special
{"r21+r19 modmii"},
{"r21+r19 modmii"},
// d2x v1
{"r21-d2x-v1"},
{"r21-d2x-v1"},
{"r21-d2x-v1"},
{"r21-d2x-v1"},
// d2x v2
{"r21-d2x-v2"},
{"r21-d2x-v2"},
{"r21-d2x-v2"},
{"r21-d2x-v2"},
// d2x v3
{"r21-d2x-v3"},
{"r21-d2x-v3"},
{"r21-d2x-v3"},
{"r21-d2x-v3"},
{"r21-d2x-v3"},
// d2x v3 (r21003)
{"r21-d2x-v3"},
{"r21-d2x-v3"},
{"r21-d2x-v3"},
{"r21-d2x-v3"},
{"r21-d2x-v3"},
// d2x v4 (r21004) beta 2
{"r21-d2x-v4-b2"},
{"r21-d2x-v4-b2"},
{"r21-d2x-v4-b2"},
{"r21-d2x-v4-b2"},
{"r21-d2x-v4-b2"},
// d2x v4 (r21004) release / hb installer
{"r21-d2x-v4"},
{"r21-d2x-v4"},
{"r21-d2x-v4"},
{"r21-d2x-v4"},
{"r21-d2x-v4"},
{"r21-d2x-v4"},
{"r21-d2x-v4"},
// d2x v4 (r21004) release / modmii
{"r21-d2x-v4"},
{"r21-d2x-v4"},
{"r21-d2x-v4"},
{"r21-d2x-v4"},
{"r21-d2x-v4"},
{"r21-d2x-v4"},
{"r21-d2x-v4"},
// modmii 249
{"r17 modmii"},	
{"r19 modmii"},
{"r19 modmii"},
{"r19 modmii"},
{"r20 modmii"}, 
{"r20 modmii"},
{"r20 modmii"},
{"r21 modmii"},
{"r21 modmii"},
{"r21 modmii"},
{"r21 modmii"},
{"r21 modmii"},
// modmii 250
{"r19 modmii"},
{"r19 modmii"},
{"r19 modmii"},
{"r20 modmii"},
{"r20 modmii"},
{"r20 modmii"},
{"r21 modmii"},
{"r21 modmii"},
{"r21 modmii"},
{"r21 modmii"},
{"r21 modmii"},
// modmii hermes
{"v4 modmii"},
{"v4 modmii"},
{"v5.0 modmii"},
{"v5.0 modmii"},
{"v5.0 modmii"},
{"r21"},
{"r21"},
{"r21"},
{"r21"},
{"r21"},
{"r21"},
{"r21"},
{"r21"},
{"r21"},
{"r21"},
{"r21"},
{"r21"},
{"r20"},
{"r20"},
{"r20"},
{"r20"},
{"r20"},
{"r20"},
{"r20"},
{"r20"},
{"r20"},
{"r20"},
{"r20"},
{"r19"},
{"r19"},
{"r19"},
{"r19"},
{"r19"},
{"r18"},
{"r18"},
{"r18"},
{"r18"},
{"r18"},
{"r13b"},
{"r13a"}
};
