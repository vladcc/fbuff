#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "fbuff.h"
#include "test.h"
//------------------------------------------------------------------------------

void run_tests(void);
bool test_fbuff_init(void);
bool test_fbuff_read(void);
bool test_fbuff_set_offset(void);
bool test_fbuff_reset(void);
bool test_fbuff_free(void);
bool test_fbuff_fp(void);
bool test_fbuff_state(void);
bool test_fbuff_buff_size(void);
bool test_fbuff_file_size(void);
bool test_fbuff_data(void);
bool test_fbuff_last_read(void);
bool test_fbuff_all_read(void);

static ftest tests[] = {
    test_fbuff_init,
    test_fbuff_free,
    test_fbuff_read,
    test_fbuff_set_offset,
    test_fbuff_reset,
    test_fbuff_fp,
    test_fbuff_state,
    test_fbuff_buff_size,
    test_fbuff_file_size,
    test_fbuff_data,
    test_fbuff_last_read,
    test_fbuff_all_read,
};

//------------------------------------------------------------------------------

// test_file should contain only test_str and nothing more
static const char * test_file;
static const char rb[] = "rb";
static const char test_str[] = "The quick brown fox jumps over the lazy dog.\n";

FILE * efopen(const char * fname, const char * mode)
{
    FILE * fp = fopen(fname, mode);
    if (NULL == fp)
    {
        fprintf(stderr, "Err: couldn't open file < %s >\n", fname);
        exit(EXIT_FAILURE);
    }
    return fp;
}

#define tfopen() efopen(test_file, rb)
//------------------------------------------------------------------------------

bool test_fbuff_init(void)
{
    fbuff btest_;
    fbuff * btest = &btest_;
    FILE * tfile = tfopen();

    check(fbuff_init(NULL, tfile, 123) == FBUFF_BAD_ARG);
    check(fbuff_init(btest, NULL, 123) == FBUFF_BAD_ARG);
    check(fbuff_init(btest, tfile, -1) == FBUFF_BAD_ARG);
    check(fbuff_init(btest, tfile, -1) == FBUFF_BAD_ARG);
    check(fbuff_init(btest, tfile, 0) == FBUFF_BAD_ARG);

                                /* needs < 2gb ram for this to fail */
    //check(fbuff_init(btest, (FILE *)1, 2000000000) == FBUFF_BAD_ALLOC);

    btest->data = NULL;
    check(NULL == btest->data);
    check(fbuff_init(btest, tfile, 2) == 0);
    check(NULL != btest->data);

    check(tfile == btest->pfile);
    check(0 == btest->state);
    check(2 == btest->buff_size);
    check(0 == btest->last_read);
    check(0 == btest->all_bytes_read);

    int test_len = strlen(test_str);
    check(test_len == btest->file_size);
    btest->file_size = 0;
    check(0 == btest->file_size);
    free(btest->data);

    check(fbuff_init(btest, tfile, 24) == 0);
    check(tfile == btest->pfile);
    check(0 == btest->state);
    check(24 == btest->buff_size);
    check(0 == btest->last_read);
    check(0 == btest->all_bytes_read);
    check(test_len == btest->file_size);
    btest->file_size = 0;
    check(0 == btest->file_size);
    free(btest->data);

    check(fbuff_init(btest, tfile, 1024) == 0);
    check(tfile == btest->pfile);
    check(0 == btest->state);
    check(1024 == btest->buff_size);
    check(0 == btest->last_read);
    check(0 == btest->all_bytes_read);
    check(test_len == btest->file_size);
    btest->file_size = 0;
    check(0 == btest->file_size);
    free(btest->data);

    fclose(tfile);
	return true;
}
//------------------------------------------------------------------------------

bool test_fbuff_free(void)
{
    fbuff btest_;
    fbuff * btest = &btest_;

    check(fbuff_free(NULL) == FBUFF_BAD_ARG);

    FILE * tfile = tfopen();
    check(fbuff_init(btest, tfile, 24) == 0);
    check(fbuff_free(btest) == 0);

    check(NULL == btest->pfile);
    check(0 == btest->state);
    check(NULL == btest->data);
    check(0 == btest->buff_size);
    check(0 == btest->last_read);
    check(0 == btest->all_bytes_read);

    check(fbuff_init(btest, tfile, 24) == 0);
    fbuff_free_null(btest);

    check(NULL == btest);
    fclose(tfile);
	return true;
}
//------------------------------------------------------------------------------

bool test_fbuff_read(void)
{
    fbuff btest_;
    fbuff * btest = &btest_;

    check(fbuff_read(NULL, 123) == FBUFF_BAD_ARG);
    check(fbuff_read(btest, -123) == FBUFF_BAD_ARG);
    check(fbuff_read(btest, 0) == 0);

    int bsz = 5;
    FILE * tfile = tfopen();
    check(fbuff_init(btest, tfile, bsz) == 0);

    int cread = 3;
    check(fbuff_read(btest, cread) == cread);

    byte * out = NULL;
    check(fbuff_data(btest, &out) == 0);
    check(fbuff_last_read(btest) == cread);
    check(fbuff_all_read(btest) == cread);
    check(fbuff_state(btest) == 0);
    check(memcmp(out, "The", cread) == 0);

    cread = 6;
    check(fbuff_read(btest, cread) == FBUFF_BAD_ARG);

    check(fbuff_read(btest, FBUFF_FILL) == bsz);
    check(memcmp(out, " quic", bsz) == 0);
    check(fbuff_last_read(btest) == bsz);
    check(fbuff_all_read(btest) == 3+bsz);
    check(fbuff_state(btest) == 0);

    cread = 3;
    while (fbuff_state(btest) != FBUFF_EOF)
        check(fbuff_read(btest, cread) > 0);

    FILE * fp = NULL;
    check(fbuff_fp(btest, &fp) == 0);
    rewind(fp);

    int all = strlen(test_str);
    while (fbuff_state(btest) != FBUFF_EOF)
        check(fbuff_read(btest, FBUFF_FILL) == 0);

    check(fbuff_all_read(btest) == all);
    check(fbuff_file_size(btest) == all);

    rewind(fp);
    check(fbuff_reset(btest) == 0);
    while (fbuff_read(btest, FBUFF_FILL) > 0)
        continue;
    check(fbuff_all_read(btest) == all);
    check(fbuff_file_size(btest) == all);

    rewind(fp);
    fbuff_free(btest);

    bsz = 124;
    check(fbuff_init(btest, tfile, bsz) == 0);
    check(fbuff_data(btest, &out) == 0);

    check(fbuff_read(btest, FBUFF_FILL) == all);
    check(fbuff_state(btest) == FBUFF_EOF);
    check(fbuff_last_read(btest) == all);
    check(fbuff_all_read(btest) == all);
    check(memcmp(out, test_str, all) == 0);

    rewind(fp);
    check(fbuff_reset(btest) == 0);
    cread = all;
    check(fbuff_read(btest, cread) == cread);
    check(fbuff_state(btest) == 0);
    check(fbuff_last_read(btest) == all);
    check(fbuff_all_read(btest) == all);
    check(memcmp(out, test_str, all) == 0);
    check(fbuff_file_size(btest) == all);

    check(fbuff_read(btest, 50) == 0);
    check(fbuff_last_read(btest) == 0);
    check(fbuff_all_read(btest) == all);
    check(fbuff_state(btest) == FBUFF_EOF);
    check(memcmp(out, test_str, all) == 0);

    rewind(fp);
    fbuff_free(btest);

    bsz = 6;
    check(fbuff_init(btest, tfile, bsz) == 0);
    check(fbuff_data(btest, &out) == 0);

    while (fbuff_state(btest) != FBUFF_EOF)
        check(fbuff_read(btest, FBUFF_FILL) > 0);

    check(fbuff_last_read(btest) > 0);
    check(fbuff_read(btest, FBUFF_FILL) == 0);
    check(fbuff_last_read(btest) == 0);
    check(fbuff_all_read(btest) == all);
    int index = all-fbuff_last_read(btest);
    check(memcmp(out, &test_str[index], fbuff_last_read(btest)) == 0);

    rewind(fp);
    check(fbuff_reset(btest) == 0);
    while (fbuff_read(btest, FBUFF_FILL) > 0)
        continue;

    check(fbuff_last_read(btest) == 0);
    check(fbuff_state(btest) == FBUFF_EOF);
    check(fbuff_read(btest, FBUFF_FILL) == 0);
    check(fbuff_last_read(btest) == 0);
    check(fbuff_all_read(btest) == all);
    check(memcmp(out, &test_str[index], fbuff_last_read(btest)) == 0);

    rewind(fp);
    fbuff_free(btest);
    check(fbuff_init(btest, tfile, all) == 0);
    check(fbuff_data(btest, &out) == 0);
    check(fbuff_read(btest, FBUFF_FILL) == all);
    check(memcmp(out, test_str, fbuff_last_read(btest)) == 0);
    check(fbuff_data(btest, &out) == 0);
    check(fbuff_read(btest, FBUFF_FILL) == 0);
    check(fbuff_state(btest) == FBUFF_EOF);

    fbuff_free_null(btest);
    fclose(tfile);
	return true;
}
//------------------------------------------------------------------------------

bool test_fbuff_set_offset(void)
{
    fbuff btest_;
    fbuff * btest = &btest_;

    check(fbuff_set_offset(NULL, 5) == FBUFF_BAD_ARG);

    int test_str_size = strlen(test_str);
    int bsz = 5;
    FILE * tfile = tfopen();
    check(fbuff_init(btest, tfile, bsz) == 0);
    check(fbuff_file_size(btest) == test_str_size);

    check(fbuff_set_offset(btest, bsz) == 0);
    check(ftell(tfile) == bsz);

    check(fbuff_read(btest, FBUFF_FILL) == bsz);
    check(ferror(tfile) == 0);
    check(fbuff_state(btest) == 0);
    byte * out;
    check(fbuff_data(btest, &out) == 0);
    check(memcmp(out, &test_str[bsz], bsz) == 0);
    check(ftell(tfile) == bsz+bsz);
    check(ferror(tfile) == 0);
    check(fbuff_file_size(btest) == test_str_size);

    check(fbuff_set_offset(btest, bsz) == 0);
    check(ftell(tfile) == bsz);
    check(fbuff_set_offset(btest, -6) == 0);

    int ofs = test_str_size - 6;
    check(ftell(tfile) == ofs);

    int read = 3;
    check(fbuff_read(btest, read) == read);
    check(fbuff_state(btest) == 0);

    check(fbuff_data(btest, &out) == 0);
    check(memcmp(out, &test_str[ofs], read) == 0);
    check(ftell(tfile) == ofs+read);
    check(fbuff_file_size(btest) == test_str_size);

    check(fbuff_read(btest, FBUFF_FILL) > 0);
    check(fbuff_state(btest) == FBUFF_EOF);

    check(fbuff_data(btest, &out) == 0);
    read = fbuff_last_read(btest);
    check(memcmp(out, &test_str[test_str_size - read], read) == 0);
    check(ftell(tfile) == test_str_size);
    check(fbuff_file_size(btest) == test_str_size);

    check(fbuff_set_offset(btest, 421) == FBUFF_BAD_OFFSET);
    check(fbuff_state(btest) == FBUFF_EOF);
    check(fbuff_read(btest, FBUFF_FILL) == 0);
    check(fbuff_last_read(btest) == 0);
    check(fbuff_state(btest) == FBUFF_EOF);
    check(fbuff_set_offset(btest, -231) == FBUFF_BAD_OFFSET);
    check(fbuff_state(btest) == FBUFF_EOF);
    check(fbuff_read(btest, FBUFF_FILL) == 0);
    check(fbuff_last_read(btest) == 0);
    check(fbuff_state(btest) == FBUFF_EOF);
    check(fbuff_file_size(btest) == test_str_size);

    fbuff_free_null(btest);
    fclose(tfile);
    return true;
}
//------------------------------------------------------------------------------

bool test_fbuff_reset(void)
{
    fbuff btest_;
    fbuff * btest = &btest_;

    check(fbuff_reset(NULL) == FBUFF_BAD_ARG);

    int bsz = 1024;
    int all = strlen(test_str);
    FILE * tfile = tfopen();
    check(fbuff_init(btest, tfile, bsz) == 0);
    check(fbuff_read(btest, FBUFF_FILL) == all);
    check(ftell(tfile) == all);
    check(btest->last_read == all);
    check(btest->all_bytes_read == all);
    check(btest->file_size == all);
    check(btest->state == FBUFF_EOF);

    check(fbuff_reset(btest) == 0);
    check(ftell(tfile) == 0);
    check(btest->state == 0);
    check(btest->last_read == 0);
    check(btest->all_bytes_read == 0);
    check(btest->file_size == all);

    fbuff_free_null(btest);
    fclose(tfile);
    return true;
}
//------------------------------------------------------------------------------

bool test_fbuff_fp(void)
{
    fbuff btest_;
    fbuff * btest = &btest_;

    check(fbuff_fp(NULL, (FILE **)1) == FBUFF_BAD_ARG);
    check(fbuff_fp(btest, NULL) == FBUFF_BAD_ARG);

    btest->pfile = (FILE *)123;
    check(btest->pfile == (FILE *)123);

    FILE * out = NULL;
    check(NULL == out);
    check(fbuff_fp(btest, &out) == 0);
    check((FILE *)123 == out);

    return true;
}
//------------------------------------------------------------------------------

bool test_fbuff_state(void)
{
    fbuff btest_;
    fbuff * btest = &btest_;

    check(fbuff_state(NULL) == FBUFF_BAD_ARG);

    btest->state = 123;
    check(123 == btest->state);

    check(fbuff_state(btest) == 123);

	return true;
}
//------------------------------------------------------------------------------

bool test_fbuff_buff_size(void)
{
    fbuff btest_;
    fbuff * btest = &btest_;

    check(fbuff_buff_size(NULL) == FBUFF_BAD_ARG);

    btest->buff_size = 123;
    check(123 == btest->buff_size);

    check(fbuff_buff_size(btest) == 123);

	return true;
}
//------------------------------------------------------------------------------

bool test_fbuff_file_size(void)
{
    fbuff btest_;
    fbuff * btest = &btest_;

    check(fbuff_file_size(NULL) == FBUFF_BAD_ARG);

    btest->file_size = 321;
    check(321 == btest->file_size);

    check(fbuff_file_size(btest) == 321);

	return true;
}
//------------------------------------------------------------------------------

bool test_fbuff_data(void)
{
    fbuff btest_;
    fbuff * btest = &btest_;

    check(fbuff_data(NULL, (byte **)1) == FBUFF_BAD_ARG);
    check(fbuff_data(btest, NULL) == FBUFF_BAD_ARG);

    btest->data = (byte *)1234;
    check(btest->data == (byte *)1234);

    byte * out = NULL;
    check(NULL == out);
    check(fbuff_data(btest, &out) == 0);
    check((byte *)1234 == out);

	return true;
}
//------------------------------------------------------------------------------

bool test_fbuff_last_read(void)
{
    fbuff btest_;
    fbuff * btest = &btest_;

    check(fbuff_last_read(NULL) == FBUFF_BAD_ARG);

    btest->last_read = 12345;
    check(12345 == btest->last_read);

    check(fbuff_last_read(btest) == 12345);

	return true;
}
//------------------------------------------------------------------------------

bool test_fbuff_all_read(void)
{
    fbuff btest_;
    fbuff * btest = &btest_;

    check(fbuff_all_read(NULL) == FBUFF_BAD_ARG);

    btest->all_bytes_read = 123456;
    check(123456 == btest->all_bytes_read);

    check(fbuff_all_read(btest) == 123456);

	return true;
}
//------------------------------------------------------------------------------

void run_tests(void)
{
    int i, end = sizeof(tests)/sizeof(*tests);

    int passed = 0;
    for (i = 0; i < end; ++i)
        if (tests[i]())
            ++passed;

    if (passed != end)
        putchar('\n');

    report(passed, end-passed);
    return;
}
//------------------------------------------------------------------------------

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Use: %s <test file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    test_file = argv[1];
    run_tests();
    return 0;
}
//------------------------------------------------------------------------------
