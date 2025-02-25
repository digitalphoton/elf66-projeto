#include <sys/stat.h>

#include "FreeRTOS.h"
#include "task.h"

int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;

    return 0;
}

int _close(int file) {
    return -1;
}

int _isatty(int file) {
    return -1;
}

off_t _lseek(int file, off_t pos, int whence) {
    return -1;
}
