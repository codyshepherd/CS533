#include "scheduler.h"
#include <aio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

ssize_t read_wrap(int fd, void * buf, size_t count) {

    struct aiocb * a = malloc(sizeof(struct aiocb));

}
