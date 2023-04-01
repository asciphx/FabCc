#include "file_sptr.hh"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
namespace fc {
  file_sptr::file_sptr(const std::string& path, size_t length, long long modified_time) {
    const char* psz = path.c_str();
    modified_time_ = modified_time;
    size_ = 0;
    ptr_ = nullptr;
#ifdef _WIN32
    int path_len = ::MultiByteToWideChar(CP_UTF8, 0, psz, -1, NULL, 0);
    WCHAR* pwsz = new WCHAR[path_len];
    ::MultiByteToWideChar(CP_UTF8, 0, psz, -1, pwsz, path_len);
    HANDLE h_file = ::CreateFileW(pwsz, FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h_file != INVALID_HANDLE_VALUE) {
      DWORD high = sizeof(length) > 4 ? (DWORD)(length >> 0x20) : 0;//32M
      DWORD low = (DWORD)length;
      if (length == 0) {
        low = ::GetFileSize(h_file, &high);
        length = (static_cast<DWORD64>(high) << 0x20) | low;//32M
      }
      assert(!(sizeof(length) == 4 && high != 0));
      HANDLE h_map = ::CreateFileMapping(h_file, NULL, PAGE_READONLY, high, low, NULL);
      if (h_map != NULL) {
        ptr_ = (char*)::MapViewOfFile(h_map, FILE_MAP_READ, 0, 0, length);
        ::CloseHandle(h_map);
      }
      ::CloseHandle(h_file);
    }
    delete[] pwsz;
#else
    int fd = ::open(psz, O_RDONLY);
    if (fd != -1) {
      if (length == 0) {
        struct stat st;
        if (::fstat(fd, &st) == 0)
          length = st.st_size;
      }
      ptr_ = (char*)::mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
      if (ptr_ == (char*)-1LL)
        ptr_ = nullptr;
      ::close(fd);
    }
#endif
    size_ = length;
  }
  file_sptr::~file_sptr() {
#ifdef _WIN32
    if (ptr_ != NULL)
      ::UnmapViewOfFile(ptr_);
#else
    if (ptr_ != nullptr)
      ::munmap(ptr_, size_);
#endif
  }
  int file_sptr::read_chunk(int64_t offset, size_t size, std::function<void(const char* c, size_t l, std::function<void()> f)> sink) {
    if (ptr_ != nullptr && offset < (int64_t)size_) {
      size_t max_size = size_ - offset;
      if (size > max_size)
        size = max_size;
      sink(ptr_ + offset, size, [p_this = shared_from_this()]() {});
      return static_cast<int>(size);
    } else {
      return EOF;
    }
  }
} // namespace fc