#include <directory.hh>
namespace fc {
  bool crossplatform_realpath(const std::string& path, char out_buffer[CROSSPLATFORM_MAX_PATH]) {
	// Check if file exists by real file path.
#if defined _WIN32
	return 0 != GetFullPathNameA(path.c_str(), CROSSPLATFORM_MAX_PATH, out_buffer, nullptr);
#else
	return nullptr != realpath(path.c_str(), out_buffer);
#endif
  }
  int create_directory(const std::string& path) {
#if defined _WIN32
#if _MSC_VER < 1900
	int ret = mkdir(path.c_str());
#else
	int ret = _mkdir(path.c_str());
#endif
#else
	int ret = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
	if (ret && errno == EEXIST) {
	  DEBUG("\"%s\" aleardy exist\n", path.c_str()); return 1;
	} else if (ret) {
	  DEBUG("create error: %d, :%s\n", ret, strerror(errno)); return -1;
	}
	return 0;
  }
  bool is_regular_file(const std::string& path) {
	struct stat ps; if (-1 == stat(path.c_str(), &ps)) return false; return ps.st_mode & S_IFREG;
  }
  bool is_directory(const std::string& path) {
	struct stat ps; if (-1 == stat(path.c_str(), &ps)) return false; return ps.st_mode & S_IFDIR;
  }
  bool exists(const char* path) {
#ifdef _WIN32
	return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
#else
	struct stat attr; return ::lstat(path, &attr) == 0;
#endif
  }
  bool starts_with(const char* pre, const char* str) {
	size_t lp = strlen(pre), ls = strlen(str); return ls < lp ? false : memcmp(pre, str, lp) == 0;
  }

  // Send a file.
  void send_file(const char* path) {
#ifndef _WIN32 // Linux / Macos version with sendfile
	// Open the file in non blocking mode.
	int fd = open(path, O_RDONLY);
	if (fd == -1) throw http_error::not_found("File not found.");
	size_t file_size = lseek(fd, (size_t)0, SEEK_END);
	// Set content type header.
	size_t ext_pos = std::string_view(path).rfind('.');
	std::string_view content_type("");
	if (ext_pos != std::string::npos) {
	  auto type_itr = content_types.find(std::string_view(path).substr(ext_pos + 1).data());
	  if (type_itr != content_types.end()) {
		content_type = type_itr->second;
		set_header("Content-Type", content_type);
	  }
	}
	// Writing the http headers.
	response_written_ = true;
	format_top_headers(output_stream);
	headers_stream.flush(); // flushes headers to output_stream.
	output_stream << "Content-Length: " << file_size << "\r\n\r\n";
	output_stream.flush();
	off_t offset = 0;
	lseek(fd, (size_t)0, 0);
	while (offset < file_size) {
#if __APPLE__ // sendfile on macos is slightly different...
	  off_t nwritten = 0;
	  int ret = ::sendfile(fd, socket_fd, offset, &nwritten, nullptr, 0);
	  offset += nwritten;
	  if (ret == 0 && nwritten == 0) break; // end of file.
#else
	  int ret = ::sendfile(socket_fd, fd, &offset, file_size - offset);
#endif
	  if (ret != -1) {
		if (offset < file_size) {
		  continue; // this->fiber.sink.yield();
		}
	  } else if (errno == EAGAIN) {
		this->fiber.sink.yield();
	  } else {
		close(fd);
		std::cerr << "Internal error: sendfile failed: " << strerror(errno) << std::endl;
		throw http_error::not_found("Internal error: sendfile failed.");
	  }
	}
	close(fd);
#else // Windows impl with basic read write.
	// Open file.
	FILE* fd;
	//if ((fd = fopen(path, "r")) == NULL) // C4996
	//  throw http_error::not_found("File not found.");
	fseek(fd, 0L, SEEK_END);
	// Get file size.
	long file_size = ftell(fd);
	// Writing the http headers.
	//response_written_ = true;
	//format_top_headers(output_stream);
	//headers_stream.flush(); // flushes to output_stream.
	//output_stream << "Content-Length: " << file_size << "\r\n\r\n"; // Add body
	//output_stream.flush();
	rewind(fd);
	// Read the file and write it to the socket.
	size_t nread = 1;
	size_t offset = 0;
	while (nread != 0) {
	  char buffer[4096];
	  nread = _fread_nolock(buffer, sizeof(buffer), 1, fd);
	  offset += sizeof(buffer);
	  //this->fiber.write(buffer, sizeof(buffer));
	}
	char buffer[4096];
	nread = _fread_nolock(buffer, file_size - offset, 1, fd);
	//this->fiber.write(buffer, file_size - offset);
	//if (!feof(fd)) throw http_error::not_found("Internal error: Could not reach the end of file.");
	fclose(fd);
#endif
  }
} // namespace fc
