#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fbuff.h"

#ifdef FBUFF_NO_CHECKS
#define check(expr, val)
#else
#define check(expr, val) while (expr) return (val)
#endif
//------------------------------------------------------------------------------

static long int fbuff_get_fsize(fbuff * fb)
{
    check(NULL == fb, FBUFF_BAD_ARG);

    long int fsize = 0;

#ifdef NO_SEEK_END
    long int read = 0;
    while ((read = fread(fb->data, sizeof(*(fb->data)), fb->buff_size,
            fb->pfile)) > 0)
        fsize += read;
#else
    fseek(fb->pfile, 0, SEEK_END);
    fsize = ftell(fb->pfile);
#endif

    if (ferror(fb->pfile) || fbuff_set_offset(fb, 0) != 0)
    {
        fb->state = FBUFF_FERR;
        return FBUFF_FERR;
    }
    fb->file_size = fsize;
    return fsize;
}
//------------------------------------------------------------------------------

int fbuff_init(fbuff * pfb, FILE * fp, int buff_size)
{
    check(NULL == pfb || NULL == fp || buff_size < 1, FBUFF_BAD_ARG);
    pfb->pfile = fp;
    pfb->file_size = 0;
    pfb->state = 0;
    pfb->buff_size = 0;
    pfb->last_read = 0;
    pfb->all_bytes_read = 0;

    if (NULL == (pfb->data = malloc(buff_size)))
        return FBUFF_BAD_ALLOC;
     pfb->buff_size = buff_size;

    if (fbuff_get_fsize(pfb) < 0)
        return FBUFF_FERR;

    return 0;
}
//------------------------------------------------------------------------------

int fbuff_free(fbuff * fb)
{
    check(NULL == fb, FBUFF_BAD_ARG);
    free(fb->data);
    memset(fb, 0, sizeof(*fb));
    return 0;
}
//------------------------------------------------------------------------------

int fbuff_read(fbuff * fb, int nbytes)
{
    if (FBUFF_FILL == nbytes)
        nbytes = fb->buff_size;

    check(NULL == fb || nbytes < 0 || nbytes > fb->buff_size, FBUFF_BAD_ARG);
    if (0 == nbytes)
        return 0;

    fb->last_read = fread(fb->data, sizeof(*(fb->data)), nbytes, fb->pfile);

    if (ferror(fb->pfile))
        fb->state = FBUFF_FERR;
    if (feof(fb->pfile))
        fb->state = FBUFF_EOF;

    fb->all_bytes_read += fb->last_read;
    return fb->last_read;
}
//------------------------------------------------------------------------------

int fbuff_reset(fbuff * fb)
{
    check(NULL == fb, FBUFF_BAD_ARG);
    fb->state = fb->last_read = fb->all_bytes_read = 0;
    return fbuff_set_offset(fb, 0);
}
//------------------------------------------------------------------------------

int fbuff_set_offset(fbuff * fb, long int offset)
{
    check(NULL == fb, FBUFF_BAD_ARG);

    if (offset < 0)
        offset = fb->file_size + offset;

    if (offset < 0 || offset > fb->file_size)
        return FBUFF_BAD_OFFSET;

    if(fseek(fb->pfile, offset, SEEK_SET) != 0)
    {
        fb->state = FBUFF_FERR;
        return FBUFF_FERR;
    }

    return 0;
}
//------------------------------------------------------------------------------

int fbuff_fp(fbuff * fb, FILE ** out)
{
    check(NULL == fb || NULL == out, FBUFF_BAD_ARG);
    *out = fb->pfile;
    return 0;
}
//------------------------------------------------------------------------------

int fbuff_state(fbuff * fb)
{
    check(NULL == fb, FBUFF_BAD_ARG);
    return fb->state;
}
//------------------------------------------------------------------------------

int fbuff_buff_size(fbuff * fb)
{
    check(NULL == fb, FBUFF_BAD_ARG);
    return fb->buff_size;
}
//------------------------------------------------------------------------------

long int fbuff_file_size(fbuff * fb)
{
    check(NULL == fb, FBUFF_BAD_ARG);
    return fb->file_size;
}
//------------------------------------------------------------------------------

int fbuff_data(fbuff * fb, byte ** out)
{
    check(NULL == fb || NULL == out, FBUFF_BAD_ARG);
    *out = fb->data;
    return 0;
}
//------------------------------------------------------------------------------

int fbuff_last_read(fbuff * fb)
{
    check(NULL == fb, FBUFF_BAD_ARG);
    return fb->last_read;
}
//------------------------------------------------------------------------------

long int fbuff_all_read(fbuff * fb)
{
    check(NULL == fb, FBUFF_BAD_ARG);
    return fb->all_bytes_read;
}
//------------------------------------------------------------------------------
