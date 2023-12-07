
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "zlib.h"

#ifdef unix
# include <unistd.h>
#else
# include <direct.h>
# include <io.h>
#endif

#ifdef WIN32
#include <windows.h>
# ifndef F_OK
# define F_OK 0
# endif
# define mkdir(dirname,mode) _mkdir(dirname)
# ifdef _MSC_VER
# define access(path,mode) _access(path,mode)
# define chmod(path,mode) _chmod(path,mode)
# define strdup(str) _strdup(str)
# endif
#else
# include <utime.h>
#endif

#define REGTYPE '0' 
#define AREGTYPE '\0' 
#define LNKTYPE '1' 
#define SYMTYPE '2' 
#define CHRTYPE '3' 
#define BLKTYPE '4' 
#define DIRTYPE '5' 
#define FIFOTYPE '6' 
#define CONTTYPE '7' 

#define GNUTYPE_DUMPDIR 'D' 
#define GNUTYPE_LONGLINK 'K' 
#define GNUTYPE_LONGNAME 'L' 
#define GNUTYPE_MULTIVOL 'M' 
#define GNUTYPE_NAMES 'N' 
#define GNUTYPE_SPARSE 'S' 
#define GNUTYPE_VOLHDR 'V' 

#define BLOCKSIZE 512
#define SHORTNAMESIZE 100
struct tar_header{  char name[100];  char mode[8];  char uid[8]; char gid[8]; char size[12]; char mtime[12];  char chksum[8];  char typeflag; char linkname[100];  char magic[6]; char version[2]; char uname[32];  char gname[32];  char devmajor[8];  char devminor[8];  char prefix[155];  };union tar_buffer{ char buffer[BLOCKSIZE]; struct tar_header header;};struct attr_item{ struct attr_item *next; char *fname; int mode; time_t time;};enum { TGZ_EXTRACT, TGZ_LIST, TGZ_INVALID };char *TGZfname OF((const char *));void TGZnotfound OF((const char *));int getoct OF((char *, int));char *strtime OF((time_t *));int setfiletime OF((char *, time_t));void push_attr OF((struct attr_item **, char *, int, time_t));void restore_attr OF((struct attr_item **));int ExprMatch OF((char *, char *));int makedir OF((char *));int matchname OF((int, int, char **, char *));void error OF((const char *));int tar OF((gzFile, int, int, int, char **));void help OF((int));int main OF((int, char **));char *prog;const char *TGZsuffix[] = { "\0", ".tar", ".tar.gz", ".taz", ".tgz", NULL };char *TGZfname (const char *arcname){ static char buffer[1024]; int origlen,i; strcpy(buffer,arcname); origlen = strlen(buffer); for (i=0; TGZsuffix[i]; i++) { strcpy(buffer+origlen,TGZsuffix[i]); if (access(buffer,F_OK) == 0) return buffer; } return NULL;}void TGZnotfound (const char *arcname){ int i; fprintf(stderr,"%s: Couldn't find ",prog); for (i=0;TGZsuffix[i];i++) fprintf(stderr,(TGZsuffix[i+1]) ? "%s%s, " : "or %s%s\n", arcname, TGZsuffix[i]); exit(1);}int getoct (char *p,int width){ int result = 0; char c; while (width--) { c = *p++; if (c == 0) break; if (c == ' ') continue; if (c < '0' || c > '7') return -1; result = result * 8 + (c - '0'); } return result;}char *strtime (time_t *t){ struct tm *local; static char result[32]; local = localtime(t); sprintf(result,"%4d/%02d/%02d %02d:%02d:%02d", local->tm_year+1900, local->tm_mon+1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec); return result;}int setfiletime (char *fname,time_t ftime){
#ifdef WIN32
 static int isWinNT = -1; SYSTEMTIME st; FILETIME locft, modft; struct tm *loctm; HANDLE hFile; int result; loctm = localtime(&ftime); if (loctm == NULL) return -1; st.wYear = (WORD)loctm->tm_year + 1900; st.wMonth = (WORD)loctm->tm_mon + 1; st.wDayOfWeek = (WORD)loctm->tm_wday; st.wDay = (WORD)loctm->tm_mday; st.wHour = (WORD)loctm->tm_hour; st.wMinute = (WORD)loctm->tm_min; st.wSecond = (WORD)loctm->tm_sec; st.wMilliseconds = 0; if (!SystemTimeToFileTime(&st, &locft) || !LocalFileTimeToFileTime(&locft, &modft)) return -1; if (isWinNT < 0) isWinNT = (GetVersion() < 0x80000000) ? 1 : 0; hFile = CreateFile(fname, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, (isWinNT ? FILE_FLAG_BACKUP_SEMANTICS : 0), NULL); if (hFile == INVALID_HANDLE_VALUE) return -1; result = SetFileTime(hFile, NULL, NULL, &modft) ? 0 : -1; CloseHandle(hFile); return result;
#else
 struct utimbuf settime; settime.actime = settime.modtime = ftime; return utime(fname,&settime);
#endif
}void push_attr(struct attr_item **list,char *fname,int mode,time_t time){ struct attr_item *item; item = (struct attr_item *)malloc(sizeof(struct attr_item)); if (item == NULL) error("Out of memory"); item->fname = strdup(fname); item->mode = mode; item->time = time; item->next = *list; *list = item;}void restore_attr(struct attr_item **list){ struct attr_item *item, *prev; for (item = *list; item != NULL; ) { setfiletime(item->fname,item->time); chmod(item->fname,item->mode); prev = item; item = item->next; free(prev); } *list = NULL;}
#define ISSPECIAL(c) (((c) == '*') || ((c) == '/'))
int ExprMatch (char *string,char *expr){ while (1) { if (ISSPECIAL(*expr)) { if (*expr == '/') { if (*string != '\\' && *string != '/') return 0; string ++; expr++; } else if (*expr == '*') { if (*expr ++ == 0) return 1; while (*++string != *expr) if (*string == 0) return 0; } } else { if (*string != *expr) return 0; if (*expr++ == 0) return 1; string++; } }}int makedir (char *newdir){ char *buffer = strdup(newdir); char *p; int len = strlen(buffer); if (len <= 0) { free(buffer); return 0; } if (buffer[len-1] == '/') { buffer[len-1] = '\0'; } if (mkdir(buffer, 0755) == 0) { free(buffer); return 1; } p = buffer+1; while (1) { char hold; while(*p && *p != '\\' && *p != '/') p++; hold = *p; *p = 0; if ((mkdir(buffer, 0755) == -1) && (errno == ENOENT)) { fprintf(stderr,"%s: Couldn't create directory %s\n",prog,buffer); free(buffer); return 0; } if (hold == 0) break; *p++ = hold; } free(buffer); return 1;}int matchname (int arg,int argc,char **argv,char *fname){ if (arg == argc) return 1; while (arg < argc) if (ExprMatch(fname,argv[arg++])) return 1; return 0; }int tar (gzFile in,int action,int arg,int argc,char **argv){ union tar_buffer buffer; int len; int err; int getheader = 1; int remaining = 0; FILE *outfile = NULL; char fname[BLOCKSIZE]; int tarmode; time_t tartime; struct attr_item *attributes = NULL; if (action == TGZ_LIST) printf(" date time size file\n" " ---------- -------- --------- -------------------------------------\n"); while (1) { len = gzread(in, &buffer, BLOCKSIZE); if (len < 0) error(gzerror(in, &err)); if (len != BLOCKSIZE) { action = TGZ_INVALID; remaining = 0; } if (getheader >= 1) { if (len == 0 || buffer.header.name[0] == 0) break; tarmode = getoct(buffer.header.mode,8); tartime = (time_t)getoct(buffer.header.mtime,12); if (tarmode == -1 || tartime == (time_t)-1) { buffer.header.name[0] = 0; action = TGZ_INVALID; } if (getheader == 1) { strncpy(fname,buffer.header.name,SHORTNAMESIZE); if (fname[SHORTNAMESIZE-1] != 0) fname[SHORTNAMESIZE] = 0; } else { if (strncmp(fname,buffer.header.name,SHORTNAMESIZE-1) != 0) error("bad long name"); getheader = 1; } switch (buffer.header.typeflag) { case DIRTYPE: if (action == TGZ_LIST) printf(" %s <dir> %s\n",strtime(&tartime),fname); if (action == TGZ_EXTRACT) { makedir(fname); push_attr(&attributes,fname,tarmode,tartime); } break; case REGTYPE: case AREGTYPE: remaining = getoct(buffer.header.size,12); if (remaining == -1) { action = TGZ_INVALID; break; } if (action == TGZ_LIST) printf(" %s %9d %s\n",strtime(&tartime),remaining,fname); else if (action == TGZ_EXTRACT) { if (matchname(arg,argc,argv,fname)) { outfile = fopen(fname,"wb"); if (outfile == NULL) { char *p = strrchr(fname, '/'); if (p != NULL) { *p = '\0'; makedir(fname); *p = '/'; outfile = fopen(fname,"wb"); } } if (outfile != NULL) printf("Extracting %s\n",fname); else fprintf(stderr, "%s: Couldn't create %s",prog,fname); } else outfile = NULL; } getheader = 0; break; case GNUTYPE_LONGLINK: case GNUTYPE_LONGNAME: remaining = getoct(buffer.header.size,12); if (remaining < 0 || remaining >= BLOCKSIZE) { action = TGZ_INVALID; break; } len = gzread(in, fname, BLOCKSIZE); if (len < 0) error(gzerror(in, &err)); if (fname[BLOCKSIZE-1] != 0 || (int)strlen(fname) > remaining) { action = TGZ_INVALID; break; } getheader = 2; break; default: if (action == TGZ_LIST) printf(" %s <---> %s\n",strtime(&tartime),fname); break; } } else { unsigned int bytes = (remaining > BLOCKSIZE) ? BLOCKSIZE : remaining; if (outfile != NULL) { if (fwrite(&buffer,sizeof(char),bytes,outfile) != bytes) { fprintf(stderr, "%s: Error writing %s -- skipping\n",prog,fname); fclose(outfile); outfile = NULL; remove(fname); } } remaining -= bytes; } if (remaining == 0) { getheader = 1; if (outfile != NULL) { fclose(outfile); outfile = NULL; if (action != TGZ_INVALID) push_attr(&attributes,fname,tarmode,tartime); } } if (action == TGZ_INVALID) { error("broken archive"); break; } } restore_attr(&attributes); if (gzclose(in) != Z_OK) error("failed gzclose"); return 0;}void help(int exitval){ printf("untgz version 0.2.1\n" " using zlib version %s\n\n", zlibVersion()); printf("Usage: untgz file.tgz extract all files\n" " untgz file.tgz fname ... extract selected files\n" " untgz -l file.tgz list archive contents\n" " untgz -h display this help\n"); exit(exitval);}void error(const char *msg){ fprintf(stderr, "%s: %s\n", prog, msg); exit(1);}
#if defined(WIN32) && defined(__GNUC__)
int _CRT_glob = 0; 
#endif
int main(int argc,char **argv){ int action = TGZ_EXTRACT; int arg = 1; char *TGZfile; gzFile *f; prog = strrchr(argv[0],'\\'); if (prog == NULL) { prog = strrchr(argv[0],'/'); if (prog == NULL) { prog = strrchr(argv[0],':'); if (prog == NULL) prog = argv[0]; else prog++; } else prog++; } else prog++; if (argc == 1) help(0); if (strcmp(argv[arg],"-l") == 0) { action = TGZ_LIST; if (argc == ++arg) help(0); } else if (strcmp(argv[arg],"-h") == 0) { help(0); } if ((TGZfile = TGZfname(argv[arg])) == NULL) TGZnotfound(argv[arg]); ++arg; if ((action == TGZ_LIST) && (arg != argc)) help(1); switch(action) { case TGZ_LIST: case TGZ_EXTRACT: f = gzopen(TGZfile,"rb"); if (f == NULL) { fprintf(stderr,"%s: Couldn't gzopen %s\n",prog,TGZfile); return 1; } exit(tar(f, action, arg, argc, argv)); break; default: error("Unknown option"); exit(1); } return 0;}