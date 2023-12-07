
#ifndef ZSTREAM__H
#define ZSTREAM__H

#include <strstream.h>
#include <string.h>
#include <stdio.h>
#include "zlib.h"

#if defined(_WIN32)
# include <fcntl.h>
# include <io.h>
# define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
# define SET_BINARY_MODE(file)
#endif
class zstringlen {public: zstringlen(class izstream&); zstringlen(class ozstream&, const char*); size_t value() const { return val.word; }private: struct Val { unsigned char byte; size_t word; } val;};class izstream{ public: izstream() : m_fp(0) {} izstream(FILE* fp) : m_fp(0) { open(fp); } izstream(const char* name) : m_fp(0) { open(name); } ~izstream() { close(); } void open(const char* name) { if (m_fp) close(); m_fp = ::gzopen(name, "rb"); } void open(FILE* fp) { SET_BINARY_MODE(fp); if (m_fp) close(); m_fp = ::gzdopen(fileno(fp), "rb"); } int close() { int r = ::gzclose(m_fp); m_fp = 0; return r; } int read(void* buf, size_t len) { return ::gzread(m_fp, buf, len); }  const char* error(int* errnum) { return ::gzerror(m_fp, errnum); } gzFile fp() { return m_fp; } private: gzFile m_fp;};template <class T, class Items>inline int read(izstream& zs, T* x, Items items) { return ::gzread(zs.fp(), x, items*sizeof(T));}template <class T>inline izstream& operator>(izstream& zs, T& x) { ::gzread(zs.fp(), &x, sizeof(T)); return zs;}inline zstringlen::zstringlen(izstream& zs) { zs > val.byte; if (val.byte == 255) zs > val.word; else val.word = val.byte;}inline izstream& operator>(izstream& zs, char* x) { zstringlen len(zs); ::gzread(zs.fp(), x, len.value()); x[len.value()] = '\0'; return zs;}inline char* read_string(izstream& zs) { zstringlen len(zs); char* x = new char[len.value()+1]; ::gzread(zs.fp(), x, len.value()); x[len.value()] = '\0'; return x;}class ozstream{ public: ozstream() : m_fp(0), m_os(0) { } ozstream(FILE* fp, int level = Z_DEFAULT_COMPRESSION) : m_fp(0), m_os(0) { open(fp, level); } ozstream(const char* name, int level = Z_DEFAULT_COMPRESSION) : m_fp(0), m_os(0) { open(name, level); } ~ozstream() { close(); } void open(const char* name, int level = Z_DEFAULT_COMPRESSION) { char mode[4] = "wb\0"; if (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level; if (m_fp) close(); m_fp = ::gzopen(name, mode); }  void open(FILE* fp, int level = Z_DEFAULT_COMPRESSION) { SET_BINARY_MODE(fp); char mode[4] = "wb\0"; if (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level; if (m_fp) close(); m_fp = ::gzdopen(fileno(fp), mode); } int close() { if (m_os) { ::gzwrite(m_fp, m_os->str(), m_os->pcount()); delete[] m_os->str(); delete m_os; m_os = 0; } int r = ::gzclose(m_fp); m_fp = 0; return r; } int write(const void* buf, size_t len) { return ::gzwrite(m_fp, (voidp) buf, len); } int flush(int _flush) { os_flush(); return ::gzflush(m_fp, _flush); } const char* error(int* errnum) { return ::gzerror(m_fp, errnum); } gzFile fp() { return m_fp; } ostream& os() { if (m_os == 0) m_os = new ostrstream; return *m_os; } void os_flush() { if (m_os && m_os->pcount()>0) { ostrstream* oss = new ostrstream; oss->fill(m_os->fill()); oss->flags(m_os->flags()); oss->precision(m_os->precision()); oss->width(m_os->width()); ::gzwrite(m_fp, m_os->str(), m_os->pcount()); delete[] m_os->str(); delete m_os; m_os = oss; } } private: gzFile m_fp; ostrstream* m_os;};template <class T, class Items>inline int write(ozstream& zs, const T* x, Items items) { return ::gzwrite(zs.fp(), (voidp) x, items*sizeof(T));}template <class T>inline ozstream& operator<(ozstream& zs, const T& x) { ::gzwrite(zs.fp(), (voidp) &x, sizeof(T)); return zs;}inline zstringlen::zstringlen(ozstream& zs, const char* x) { val.byte = 255; val.word = ::strlen(x); if (val.word < 255) zs < (val.byte = val.word); else zs < val;}inline ozstream& operator<(ozstream& zs, const char* x) { zstringlen len(zs, x); ::gzwrite(zs.fp(), (voidp) x, len.value()); return zs;}
#ifdef _MSC_VER
inline ozstream& operator<(ozstream& zs, char* const& x) { return zs < (const char*) x;}
#endif
template <class T>inline ostream& operator<<(ozstream& zs, const T& x) { zs.os_flush(); return zs.os() << x;}
#endif
