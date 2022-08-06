
#include "gzguts.h"
int ZEXPORT gzclose(file) gzFile file;{
#ifndef NO_GZCOMPRESS
 gz_statep state; if (file == NULL) return Z_STREAM_ERROR; state = (gz_statep)file; return state->mode == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#else
 return gzclose_r(file);
#endif
}