
#ifndef _zip12_H
#define _zip12_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ZLIB_H
#include "zlib.h"
#endif

#ifndef _ZLIBIOAPI_H
#include "ioapi.h"
#endif

#ifdef HAVE_BZIP2
#include "bzlib.h"
#endif

#define Z_BZIP2ED 12

#if defined(STRICTZIP) || defined(STRICTZIPUNZIP)
typedef struct TagzipFile__ { int unused; } zipFile__;typedef zipFile__ *zipFile;
#else
typedef voidp zipFile;
#endif

#define ZIP_OK (0)
#define ZIP_EOF (0)
#define ZIP_ERRNO (Z_ERRNO)
#define ZIP_PARAMERROR (-102)
#define ZIP_BADZIPFILE (-103)
#define ZIP_INTERNALERROR (-104)

#ifndef DEF_MEM_LEVEL
# if MAX_MEM_LEVEL >= 8
# define DEF_MEM_LEVEL 8
# else
# define DEF_MEM_LEVEL MAX_MEM_LEVEL
# endif
#endif
typedef struct tm_zip_s{ int tm_sec;  int tm_min;  int tm_hour; int tm_mday; int tm_mon;  int tm_year; } tm_zip;typedef struct{ tm_zip tmz_date;  uLong dosDate;  uLong internal_fa; uLong external_fa; } zip_fileinfo;typedef const char* zipcharpc;
#define APPEND_STATUS_CREATE (0)
#define APPEND_STATUS_CREATEAFTER (1)
#define APPEND_STATUS_ADDINZIP (2)
extern zipFile ZEXPORT zipOpen OF((const char *pathname, int append));extern zipFile ZEXPORT zipOpen64 OF((const void *pathname, int append));extern zipFile ZEXPORT zipOpen2 OF((const char *pathname, int append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc_def));extern zipFile ZEXPORT zipOpen2_64 OF((const void *pathname, int append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def));extern zipFile ZEXPORT zipOpen3 OF((const void *pathname, int append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def));extern int ZEXPORT zipOpenNewFileInZip OF((zipFile file, const char* filename, const zip_fileinfo* zipfi, const void* extrafield_local, uInt size_extrafield_local, const void* extrafield_global, uInt size_extrafield_global, const char* comment, int method, int level));extern int ZEXPORT zipOpenNewFileInZip64 OF((zipFile file, const char* filename, const zip_fileinfo* zipfi, const void* extrafield_local, uInt size_extrafield_local, const void* extrafield_global, uInt size_extrafield_global, const char* comment, int method, int level, int zip64));extern int ZEXPORT zipOpenNewFileInZip2 OF((zipFile file, const char* filename, const zip_fileinfo* zipfi, const void* extrafield_local, uInt size_extrafield_local, const void* extrafield_global, uInt size_extrafield_global, const char* comment, int method, int level, int raw));extern int ZEXPORT zipOpenNewFileInZip2_64 OF((zipFile file, const char* filename, const zip_fileinfo* zipfi, const void* extrafield_local, uInt size_extrafield_local, const void* extrafield_global, uInt size_extrafield_global, const char* comment, int method, int level, int raw, int zip64));extern int ZEXPORT zipOpenNewFileInZip3 OF((zipFile file, const char* filename, const zip_fileinfo* zipfi, const void* extrafield_local, uInt size_extrafield_local, const void* extrafield_global, uInt size_extrafield_global, const char* comment, int method, int level, int raw, int windowBits, int memLevel, int strategy, const char* password, uLong crcForCrypting));extern int ZEXPORT zipOpenNewFileInZip3_64 OF((zipFile file, const char* filename, const zip_fileinfo* zipfi, const void* extrafield_local, uInt size_extrafield_local, const void* extrafield_global, uInt size_extrafield_global, const char* comment, int method, int level, int raw, int windowBits, int memLevel, int strategy, const char* password, uLong crcForCrypting, int zip64 ));extern int ZEXPORT zipOpenNewFileInZip4 OF((zipFile file, const char* filename, const zip_fileinfo* zipfi, const void* extrafield_local, uInt size_extrafield_local, const void* extrafield_global, uInt size_extrafield_global, const char* comment, int method, int level, int raw, int windowBits, int memLevel, int strategy, const char* password, uLong crcForCrypting, uLong versionMadeBy, uLong flagBase ));extern int ZEXPORT zipOpenNewFileInZip4_64 OF((zipFile file, const char* filename, const zip_fileinfo* zipfi, const void* extrafield_local, uInt size_extrafield_local, const void* extrafield_global, uInt size_extrafield_global, const char* comment, int method, int level, int raw, int windowBits, int memLevel, int strategy, const char* password, uLong crcForCrypting, uLong versionMadeBy, uLong flagBase, int zip64 ));extern int ZEXPORT zipWriteInFileInZip OF((zipFile file, const void* buf, unsigned len));extern int ZEXPORT zipCloseFileInZip OF((zipFile file));extern int ZEXPORT zipCloseFileInZipRaw OF((zipFile file, uLong uncompressed_size, uLong crc32));extern int ZEXPORT zipCloseFileInZipRaw64 OF((zipFile file, ZPOS64_T uncompressed_size, uLong crc32));extern int ZEXPORT zipClose OF((zipFile file, const char* global_comment));extern int ZEXPORT zipRemoveExtraInfoBlock OF((char* pData, int* dataLen, short sHeader));
#ifdef __cplusplus
}
#endif

#endif 
