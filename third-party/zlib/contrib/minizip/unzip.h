
#ifndef _unz64_H
#define _unz64_H

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

#if defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
typedef struct TagunzFile__ { int unused; } unzFile__;typedef unzFile__ *unzFile;
#else
typedef voidp unzFile;
#endif

#define UNZ_OK (0)
#define UNZ_END_OF_LIST_OF_FILE (-100)
#define UNZ_ERRNO (Z_ERRNO)
#define UNZ_EOF (0)
#define UNZ_PARAMERROR (-102)
#define UNZ_BADZIPFILE (-103)
#define UNZ_INTERNALERROR (-104)
#define UNZ_CRCERROR (-105)
typedef struct tm_unz_s{ int tm_sec;  int tm_min;  int tm_hour; int tm_mday; int tm_mon;  int tm_year; } tm_unz;typedef struct unz_global_info64_s{ ZPOS64_T number_entry;  uLong size_comment; } unz_global_info64;typedef struct unz_global_info_s{ uLong number_entry;  uLong size_comment; } unz_global_info;typedef struct unz_file_info64_s{ uLong version; uLong version_needed;  uLong flag;  uLong compression_method;  uLong dosDate; uLong crc; ZPOS64_T compressed_size;  ZPOS64_T uncompressed_size; uLong size_filename; uLong size_file_extra; uLong size_file_comment; uLong disk_num_start;  uLong internal_fa; uLong external_fa; tm_unz tmu_date;} unz_file_info64;typedef struct unz_file_info_s{ uLong version; uLong version_needed;  uLong flag;  uLong compression_method;  uLong dosDate; uLong crc; uLong compressed_size; uLong uncompressed_size; uLong size_filename; uLong size_file_extra; uLong size_file_comment; uLong disk_num_start;  uLong internal_fa; uLong external_fa; tm_unz tmu_date;} unz_file_info;extern int ZEXPORT unzStringFileNameCompare OF ((const char* fileName1, const char* fileName2, int iCaseSensitivity));extern unzFile ZEXPORT unzOpen OF((const char *path));extern unzFile ZEXPORT unzOpen64 OF((const void *path));extern unzFile ZEXPORT unzOpen2 OF((const char *path, zlib_filefunc_def* pzlib_filefunc_def));extern unzFile ZEXPORT unzOpen2_64 OF((const void *path, zlib_filefunc64_def* pzlib_filefunc_def));extern int ZEXPORT unzClose OF((unzFile file));extern int ZEXPORT unzGetGlobalInfo OF((unzFile file, unz_global_info *pglobal_info));extern int ZEXPORT unzGetGlobalInfo64 OF((unzFile file, unz_global_info64 *pglobal_info));extern int ZEXPORT unzGetGlobalComment OF((unzFile file, char *szComment, uLong uSizeBuf));extern int ZEXPORT unzGoToFirstFile OF((unzFile file));extern int ZEXPORT unzGoToNextFile OF((unzFile file));extern int ZEXPORT unzLocateFile OF((unzFile file, const char *szFileName, int iCaseSensitivity));typedef struct unz_file_pos_s{ uLong pos_in_zip_directory;  uLong num_of_file; } unz_file_pos;extern int ZEXPORT unzGetFilePos( unzFile file, unz_file_pos* file_pos);extern int ZEXPORT unzGoToFilePos( unzFile file, unz_file_pos* file_pos);typedef struct unz64_file_pos_s{ ZPOS64_T pos_in_zip_directory;  ZPOS64_T num_of_file; } unz64_file_pos;extern int ZEXPORT unzGetFilePos64( unzFile file, unz64_file_pos* file_pos);extern int ZEXPORT unzGoToFilePos64( unzFile file, const unz64_file_pos* file_pos);extern int ZEXPORT unzGetCurrentFileInfo64 OF((unzFile file, unz_file_info64 *pfile_info, char *szFileName, uLong fileNameBufferSize, void *extraField, uLong extraFieldBufferSize, char *szComment, uLong commentBufferSize));extern int ZEXPORT unzGetCurrentFileInfo OF((unzFile file, unz_file_info *pfile_info, char *szFileName, uLong fileNameBufferSize, void *extraField, uLong extraFieldBufferSize, char *szComment, uLong commentBufferSize));extern ZPOS64_T ZEXPORT unzGetCurrentFileZStreamPos64 OF((unzFile file));extern int ZEXPORT unzOpenCurrentFile OF((unzFile file));extern int ZEXPORT unzOpenCurrentFilePassword OF((unzFile file, const char* password));extern int ZEXPORT unzOpenCurrentFile2 OF((unzFile file, int* method, int* level, int raw));extern int ZEXPORT unzOpenCurrentFile3 OF((unzFile file, int* method, int* level, int raw, const char* password));extern int ZEXPORT unzCloseCurrentFile OF((unzFile file));extern int ZEXPORT unzReadCurrentFile OF((unzFile file, voidp buf, unsigned len));extern z_off_t ZEXPORT unztell OF((unzFile file));extern ZPOS64_T ZEXPORT unztell64 OF((unzFile file));extern int ZEXPORT unzeof OF((unzFile file));extern int ZEXPORT unzGetLocalExtrafield OF((unzFile file, voidp buf, unsigned len));extern ZPOS64_T ZEXPORT unzGetOffset64 (unzFile file);extern uLong ZEXPORT unzGetOffset (unzFile file);extern int ZEXPORT unzSetOffset64 (unzFile file, ZPOS64_T pos);extern int ZEXPORT unzSetOffset (unzFile file, uLong pos);
#ifdef __cplusplus
}
#endif

#endif 
