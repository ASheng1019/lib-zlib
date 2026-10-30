/* SPDX-License-Identifier: BSD-3-Clause */
#include <uk/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

static void check(int condition, int line)
{
	if (!condition) {
		printf("ZLIB_CONTRACT FAIL line=%d\n", line);
		exit(EXIT_FAILURE);
	}
}

#define CHECK(x) check((x), __LINE__)

int main(void)
{
	static const unsigned char plain[] = "independent zlib port contract";
	unsigned char compressed[256], restored[256];
	uLongf clen = sizeof(compressed), rlen = sizeof(restored);
	z_stream encoder = {0}, decoder = {0};
	int rc;

	CHECK(compress2(compressed, &clen, plain, sizeof(plain), 6) == Z_OK);
	CHECK(uncompress(restored, &rlen, compressed, clen) == Z_OK);
	CHECK(rlen == sizeof(plain) && !memcmp(plain, restored, rlen));
	compressed[clen - 1] ^= 1;
	rlen = sizeof(restored);
	CHECK(uncompress(restored, &rlen, compressed, clen) == Z_DATA_ERROR);
	CHECK(crc32(0, (const Bytef *)"123456789", 9) == 0xcbf43926UL);
	rc = deflateInit2(&encoder, 6, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
	CHECK(rc == Z_OK);
	encoder.next_in = (Bytef *)plain;
	encoder.avail_in = sizeof(plain);
	encoder.next_out = compressed;
	encoder.avail_out = sizeof(compressed);
	CHECK(deflate(&encoder, Z_FINISH) == Z_STREAM_END);
	clen = encoder.total_out;
	CHECK(deflateEnd(&encoder) == Z_OK);
	CHECK(inflateInit2(&decoder, 31) == Z_OK);
	decoder.next_in = compressed;
	decoder.avail_in = clen;
	decoder.next_out = restored;
	decoder.avail_out = sizeof(restored);
	CHECK(inflate(&decoder, Z_FINISH) == Z_STREAM_END);
	CHECK(decoder.total_out == sizeof(plain));
	CHECK(!memcmp(plain, restored, sizeof(plain)));
	CHECK(inflateEnd(&decoder) == Z_OK);
#if CONFIG_LIBZLIB_GZFILE
	{
		gzFile file = gzopen("/contract.gz", "wb");

		CHECK(!!file);
		CHECK(gzwrite(file, plain, sizeof(plain)) == sizeof(plain));
		CHECK(gzclose(file) == Z_OK);
		file = gzopen("/contract.gz", "rb");
		CHECK(!!file);
		rc = gzread(file, restored, sizeof(restored));
		CHECK(rc == sizeof(plain));
		CHECK(!memcmp(plain, restored, sizeof(plain)));
		CHECK(gzread(file, restored, sizeof(restored)) == 0);
		CHECK(gzeof(file));
		CHECK(gzclose(file) == Z_OK);
		CHECK(remove("/contract.gz") == 0);
	}
	puts("ZLIB_CONTRACT PASS memory=1 gzip_stream=1 gzip_file=1");
#else
	puts("ZLIB_CONTRACT PASS memory=1 gzip_stream=1 gzip_file=0");
#endif
	return 0;
}
