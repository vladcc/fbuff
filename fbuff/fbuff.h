/**
    A file buffer

    Handles random access reading from an opened file.

    fseek() with SEEK_END is used once in fbuff_init() in order to get the size
    of the file. The size of the file is used to calculate the offset whenever
    fbuff_set_offset() is called. If NO_SEEK_END is defined on compilation, the
    file size is obtained by reading the file into the buffer in buffer size
    chunks until EOF. The max size of the buffer is 2GB. All fbuff_ functions
    perform argument checking and return FBUFF_BAD_ARG when an invalid value
    is encountered. This is disabled if FBUFF_NO_CHECKS is defined on compile
    time.

    fbuff does not open or close files, it uses an already valid file pointer.
    Any open/close operations must happen outside.

    ver. 1.0
    Author: Vladimir Dinev
    2018-07-09
*/

#ifndef FBUFF_H
#define FBUFF_H

#include <stdio.h>

//#define NO_SEEK_END

enum {
    FBUFF_BAD_ARG     = -1,
    FBUFF_BAD_ALLOC   = -2,
    FBUFF_EOF         = -3,
    FBUFF_FERR        = -4,
    FBUFF_BAD_OFFSET  = -5
};
/** Return codes. */

typedef unsigned char byte;
typedef struct fbuff {
    FILE * pfile;
    long int file_size;
    int state;
    byte * data;
    int buff_size;
    int last_read;
    long int all_bytes_read;
} fbuff;
/** Don't use members directly. */

int fbuff_init(fbuff * pfb, FILE * fp, int buff_size);
/**
Returns:
Checks enabled
    FBUFF_BAD_ARG when pfb is NULL, fp is NULL, or buff_size is < 1.

Always
    FBUFF_BAD_ALLOC if memory allocation fails.
    FBUFF_FERR if getting the file size fails.
    0 on success.

Description: Initializes a fbuff with a size of buff_size bytes, ready to access
the file pointed to by fp.
*/

#define FBUFF_FILL -1
int fbuff_read(fbuff * fb, int nbytes);
/**
Returns:
Checks enabled
    FBUFF_BAD_ARG when fb is NULL, nbytes is < 0 and different from FBUFF_FILL,
    or when nbytes is > buff_size.

Always
    FBUFF_ERR if ferror() returns true.
    FBUFF_EOF if feof() returns true.
    The number of bytes read otherwise.

Description: Reads nbytes number of bytes inside the buffer. If nbytes is set to
FBUFF_FILL, then the function attempts to read buff_size bytes.
*/

int fbuff_set_offset(fbuff * fb, long int offset);
/**
Returns:
Checks enabled
    FBUFF_BAD_ARG when fb is NULL.

Always
    FBUFF_BAD_OFFSET when offset points outside the bounds of the file.
    FBUFF_FERR if fseek() fails.
    0 on success.

Description: Changes the offset in the file. offset can be negative, in which
case the file position is set to -(offset) bytes before eof.
*/

int fbuff_reset(fbuff * fb);
/**
Returns:
Checks enabled
    FBUFF_BAD_ARG when fb is NULL.

Always
    Same values as fbuff_set_offset().

Description: Zeroes out the buffer state, last read, all read information, and
rewinds the file back to its start.
*/

int fbuff_free(fbuff * fb);
#define fbuff_free_null(pfb) fbuff_free((pfb)), (pfb) = NULL
/**
Returns:
Checks enabled
    FBUFF_BAD_ARG when fb is NULL.

Always
    0 on success.

Description: Frees the memory allocated for the buffer and zeroes out all buffer
data members. Note that fbuff_free_null() disregards the return value of
fbuff_free().
*/


int fbuff_fp(fbuff * fb, FILE ** out);
/**
Returns:
Checks enabled
    FBUFF_BAD_ARG when fb or out is NULL.

Always
    0 on success.

Description: Sets the value of out to the value of the FILE * associated with
the buffer.
*/

int fbuff_state(fbuff * fb);
/**
Returns:
Checks enabled
    FBUFF_BAD_ARG when fb is NULL.

Always
    FBUFF_FERR if an error has occurred.
    FBUFF_EOF if eof has been reached.
    0 otherwise.

Description: Returns the state of the buffer. 0 is the default.
*/

int fbuff_buff_size(fbuff * fb);
/**
Returns:
Checks enabled
    FBUFF_BAD_ARG when fb is NULL.

Always
    The buffer size on success.

Description: Returns the buffer size.
*/

long int fbuff_file_size(fbuff * fb);
/**
Returns:
Checks enabled
    FBUFF_BAD_ARG when fb is NULL.

Always
    The file size on success.

Description: Returns the file size of the file associated with the buffer.
*/

int fbuff_data(fbuff * fb, byte ** out);
/**
Returns:
Checks enabled
    FBUFF_BAD_ARG when fb or out is NULL.

Always
    0 on success.

Description: Sets out pointing to the buffer containing the data read from the
file. This address does not change during the life of a fbuff.
*/

int fbuff_last_read(fbuff * fb);
/**
Returns:
Checks enabled
    FBUFF_BAD_ARG when fb is NULL.

Always
    The number of bytes read from the last read operation.

Description: Returns the result of the last read.
*/

long int fbuff_all_read(fbuff * fb);
/**
Returns:
Checks enabled
    FBUFF_BAD_ARG when fb is NULL.

Always
    The total number of bytes read from the file so far.

Description: Returns the result of all read operations performed.
*/
#endif
