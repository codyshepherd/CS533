#include "scheduler.h"
#include <aio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

/*
struct aiocb, aio_read, aio_error, aio_return
EINPROGRESS
lseek, SEEK_CUR, SEEK_END, SEEK_SET
memset
*/


ssize_t read_wrap(int fd, void * buf, size_t count) {

    struct aiocb * a = malloc(sizeof(struct aiocb));

    off_t offst = lseek(fd, 0, SEEK_CUR);
    if(offst>=0)
        a->aio_offset = offst;
    else
        a->aio_offset = 0;

    a->aio_fildes = fd;
    a->aio_buf = buf;
    a->aio_nbytes = count;
    a->aio_reqprio = 1; //??
    a->aio_sigevent.sigev_notify = SIGEV_NONE;
    //a->aio_lio_opcode  // not needed
    
    aio_read(a);
    while(aio_error(a) == EINPROGRESS)
        yield();
    
    ssize_t ret = aio_return(a);
    free(a);
    return ret;
}
