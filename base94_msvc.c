
#include <limits.h>

#if UCHAR_MAX != 255
#error sizeof (char) != 1
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

#if defined(__MSDOS__) || defined(_WIN32)
#   include <io.h>
#   include <fcntl.h>
#endif

#include "base94.h"

#define MODE_AUTO   0
#define MODE_ENCODE 1
#define MODE_DECODE 2

#define BASE94_EXTENSION ".b94"

static const int ENCODE_SIZE_MAP[] = { 0, 2, 3, 4, 5, 7, 8, 9, 10, 11, };
static const int DECODE_SIZE_MAP[] = { 0, 0, 1, 2, 3, 4, 4, 5, 6, 7, 8, 9 };

static const char* PROGRAM;

int base94_encode_stream(FILE *input, FILE *output, int max_line_length) {

	int error;
	unsigned char plain[9];
	unsigned char code[11];
	size_t size;
	unsigned int i;

	int line_length = 0;

	for (; ; ) {

		// input
		size = fread(plain, 1, sizeof plain, input);
		if (ferror(input)) {
			error = errno;
	        fprintf(stderr, "%s: can't read data.\n", PROGRAM);
			return error;
		}
		if (size == 0) {
			break;
		}

		if (size < sizeof plain) {
			memset(plain + (sizeof plain - size), 0, sizeof plain - size);
		}

		// encode
		base94_encode(plain, code);
		size = ENCODE_SIZE_MAP[size];

		// output
		for (i = 0; i < size; ++i) {
			
			if (putc(code[i], output) == EOF) {
				if (ferror(output)) {
					error = errno;
				} else {
					error = EIO;
				}
				fprintf(stderr, "%s: can't write data.\n", PROGRAM);
				return error;
			}

			if (max_line_length > 0) {
				++line_length;
				if (line_length >= max_line_length) {

					if (putc('\n', output) == EOF) {
						if (ferror(output)) {
							error = errno;
						} else {
							error = EIO;
						}
						fprintf(stderr, "%s: can't write data.\n", PROGRAM);
						return error;
					}

					line_length  = 0;
				}
			}

		}

		//result = fwrite(code, 1, size, output);
		//if (ferror(output)) {
		//	error = errno;
	 //       fprintf(stderr, "%s: can't write data.\n", PROGRAM);
		//	return error;
		//}
		//if (size != result) {
	 //       fprintf(stderr, "%s: can't write data completely.\n", PROGRAM);
		//	return EIO;
		//}

	}

	return 0;

}

int base94_decode_stream(FILE *input, FILE *output) {

	int error;
	unsigned char plain[9];
	unsigned char code[11];
	size_t size, result;
	int c;

	for (; ; ) {

		// input

		// collect 11 characters
		size = 0;

		while (size < sizeof code) {

			c = getc(input);
			if (c == EOF) {
				if (ferror(input)) {
					error = errno;
					fprintf(stderr, "%s: can't read data.\n", PROGRAM);
					return error;
				}
				break; // EOF
			}

			if (c >= 33 && c <= 126) {
				code[size] = (unsigned char)c;
				++size;
			} else if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				// skip
			} else {
				fprintf(stderr, "%s: invalid base94 byte %d, skipped.\n", PROGRAM, c);
			}

		}

		if (size == 0) {
			break;
		}

		if (size < sizeof code) {
			memset(code + size, 126, sizeof code - size);
		}

		// decode
		base94_decode(code, plain);
		size = DECODE_SIZE_MAP[size];

		// output
		result = fwrite(plain, 1, size, output);
		if (ferror(output)) {
			error = errno;
	        fprintf(stderr, "%s: can't write data.\n", PROGRAM);
			return error;
		}
		if (size != result) {
	        fprintf(stderr, "%s: can't write data completely.\n", PROGRAM);
			return EIO;
		}

	}

	return 0;

}

/*
int test() {

	unsigned char plain[9];
	unsigned char plain2[9];
	unsigned char code[11];
	int i, j;
	int total = 0, bad = 0;

	srand(0);
	memset(plain, 0, 9);

	for (i = 0; i < 1000000; ++i) {

		for (j = 0; j < 9; ++j) {
			plain[j] = rand() & 0xff;
		}

		base94_encode(plain, code);
		base94_decode(code, plain2);

		if (memcmp(plain, plain2, 9) != 0) {
			printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x -",
					plain[0], plain[1], plain[2], plain[3], plain[4], plain[5], plain[6], plain[7], plain[8], plain[9]);
			printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
					plain2[0], plain2[1], plain2[2], plain2[3], plain2[4], plain2[5], plain2[6], plain2[7], plain2[8], plain2[9]);
		}

	}

	for (i = 0; i < 1000000; ++i) {

		for (j = 0; j < 9; ++j) {
			plain[j] = rand() & 0xff;
		}

		for (j = 8; j > 0; --j) { // 8-1

			plain[j] = 0;
			base94_encode(plain, code);
			memset(code + ENCODE_SIZE_MAP[j], 126, sizeof code - ENCODE_SIZE_MAP[j]);
			base94_decode(code, plain2);

			++total;

			if (memcmp(plain, plain2, j) != 0) {
				printf("%d: %02x %02x %02x %02x %02x %02x %02x %02x %02x - ",
						j, plain[0], plain[1], plain[2], plain[3], plain[4], plain[5], plain[6], plain[7], plain[8], plain[9]);
				printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						plain2[0], plain2[1], plain2[2], plain2[3], plain2[4], plain2[5], plain2[6], plain2[7], plain2[8], plain2[9]);
				++bad;
			}

		}

	}

	printf("Total: %d, Bad: %d\n", total, bad);
	puts("Press enter to exit,\n");
	getchar();

	return 0;
}
*/

int main(int argc, char *argv[]) {

	FILE *in = stdin;
	FILE *out = stdout;
	FILE *in_file = NULL;
	FILE *out_file = NULL;
	int error = 0;
	int mode = MODE_AUTO;

	PROGRAM = strrchr(argv[0], '\\');
	if (PROGRAM) {
		++PROGRAM;
	} else {
		PROGRAM = argv[0];
	}

//	return test();

	if (argc == 2) {

		const char *source = argv[1];
		char destination[256];

		// strip directory components
		const char *extension = strrchr(source, '\\');
		if (extension) {
			++extension;
		} else {
			extension = source;
		}

		// delect extension
		extension = strrchr(extension, '.');
		if (extension) {
			mode = strcasecmp(extension, BASE94_EXTENSION) == 0 ? MODE_DECODE : MODE_ENCODE;
		} else {
			mode = MODE_ENCODE;
		}

		if (mode == MODE_DECODE) {
			// decode
			strcpy(destination, source);
			destination[strlen(source) - strlen(BASE94_EXTENSION)] = '\0'; // strip .b94
		} else {
			// encode
			strcpy(destination, source);
			strcat(destination, BASE94_EXTENSION); // append .b94
		}

		in_file = fopen(source, "r"); 
		if (in_file == NULL) {
			error = errno;
			fprintf(stderr, "%s: can't open file '%s'.\n", PROGRAM, source);
			return error;
		}
		out_file = fopen(destination, "w"); 
		if (out_file == NULL) {
			error = errno;
			fprintf(stderr, "%s: can't open file '%s'.\n", PROGRAM, destination);
			fclose(out_file);
			return error;
		}

		in = in_file;
		out = out_file;
	}

	if (mode == MODE_AUTO) {
		if (strcasecmp(PROGRAM, "unbase94") == 0 || strcasecmp(PROGRAM, "unbase94.exe") == 0) {
			mode = MODE_DECODE;
		} else {
			mode = MODE_ENCODE;
		}
	}
	
    mode = MODE_DECODE;

	switch (mode) {

		case MODE_ENCODE:
			/* set to binary mode for Windows */
#if defined(__MSDOS__) || defined(_WIN32)
			if (_setmode(_fileno(in), _O_BINARY) == -1) {
				fprintf(stderr, "%s: can't change input to binary mode.\n", PROGRAM);
				return EIO;
			}
#endif
			error = base94_encode_stream(in, out, 76);
			break;

		case MODE_DECODE:
			/* set to binary mode for Windows */
#if defined(__MSDOS__) || defined(_WIN32)
			if (_setmode(_fileno(out), _O_BINARY) == -1) {
				fprintf(stderr, "%s: can't change output to binary mode.\n", PROGRAM);
				return EIO;
			}
#endif
			error = base94_decode_stream(in, out);
			break;

	}

	if (in_file) {
		fclose(in_file);
	}
	if (out_file) {
		fclose(out_file);
	}
	
	return error;

}

