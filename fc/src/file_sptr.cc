#include "file_sptr.hh"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include "hh/directory.hh"
#else
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <iostream>
namespace fc {
  file_sptr::file_sptr(const std::string& path, _Fsize_t length, long long modified_time) {
    const char* psz = path.c_str();
    modified_time_ = modified_time;
    size_ = length;
#ifdef _WIN32
    int path_len = ::MultiByteToWideChar(CP_UTF8, 0, psz, -1, NULL, 0);
    WCHAR* pwsz = new WCHAR[path_len]; ::MultiByteToWideChar(CP_UTF8, 0, psz, -1, pwsz, path_len);
    fd_ = ::CreateFileW(pwsz, FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
      OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, NULL); delete[] pwsz; pwsz = nullptr;
    if (fd_ != INVALID_HANDLE_VALUE) {
      if (length == 0) {
        LARGE_INTEGER fileSize; ::GetFileSizeEx(fd_, &fileSize); size_ = fileSize.QuadPart;
#else
    fd_ = ::open(psz, O_RDONLY);
    if (fd_ != -1) {
      if (length == 0) {
        struct stat64 st; if (stat64(psz, &st) == 0)size_ = st.st_size;
#endif
      }
    }
  }
  file_sptr::~file_sptr() {
#ifdef _WIN32
    if (fd_ != nullptr)CloseHandle(fd_);
#else
    ::close(fd_);
#endif
  }
  _CTX_TASK(void) file_sptr::read_chunk(std::function<_CTX_TASK(void)(_Fhandle fd)> sink) { co_await sink(fd_); co_return; }
} // namespace fc