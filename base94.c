
#include "base94.h"

void base94_encode(const unsigned char *plain, unsigned char *code) {

	// high * 2^64 | low

	unsigned long long value 
			= ((unsigned long long)plain[1] << 56) | ((unsigned long long)plain[2] << 48)
			| ((unsigned long long)plain[3] << 40) | ((unsigned long long)plain[4] << 32)
			| ((unsigned long)plain[5] << 24) | ((unsigned long)plain[6] << 16) 
			| ((unsigned long)plain[7] <<  8) |  plain[8];

	unsigned long remainder;
	int i;

	// 2^64 = 2087680406712262 * 94^2 + 4584

	remainder = value % 8836 + (unsigned long)plain[0] * 4584;
	value = value / 8836 + plain[0] * 2087680406712262ul;

	code[10] = 33 + remainder % 94;
	remainder /= 94;
	code[9]  = 33 + remainder % 94;
	value += remainder / 94;

	for (i = 8; i >= 0; --i) {
		code[i] = 33 + value % 94;
		value /= 94;
	}

}

void base94_decode(const unsigned char *code, unsigned char *plain) {

	// high * 94^9 | low

	unsigned long long value = 0;
	unsigned long high;
	int i;
	
	for (i = 2; i < 11; ++i) {
		value *= 94;
		value += code[i] - 33;
	}
	high = (code[0] - 33) * 94 + (code[1] - 33);

	// 94^9 = 2238260946205534 * 2^8
	plain[8] = value & 0xff;
	value >>= 8;
	value += high * 2238260946205534;

	plain[0] = (value >> 56) & 0xff;
	plain[1] = (value >> 48) & 0xff;
	plain[2] = (value >> 40) & 0xff;
	plain[3] = (value >> 32) & 0xff;
	plain[4] = (value >> 24) & 0xff;
	plain[5] = (value >> 16) & 0xff;
	plain[6] = (value >>  8) & 0xff;
	plain[7] = value & 0xff;

}


