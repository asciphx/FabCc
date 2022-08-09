#include <iostream>
#include <cassert>
//#include <str.hh>
static const char _X[] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,-1,
 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
std::string DecodeURL(std::string& s) {
  char* o = (char*)s.c_str(), * c = (char*)s.c_str();
  const char* e = c + s.size(); while (c < e) {
	if (*c == '%' && c < e - 2 && _X[c[1]] != -1 && _X[c[2]] != -1) {
	  *o = (_X[c[1]] << 4) | _X[c[2]]; c += 2; ++o; ++c; continue;
	}
	if (*c == '+') { *o = ' '; ++o; ++c; continue; }
	if (o != c) *o = *c; ++o; ++c;
  } return std::string(s.data(), o - s.data());
}
std::string DecodeURL(const char* s) {
  size_t l = strlen(s); char* o = (char*)s, * c = (char*)s;
  const char* e = c + l; while (c < e) {
	if (*c == '%' && c < e - 2 && _X[c[1]] != -1 && _X[c[2]] != -1) {
	  *o = (_X[c[1]] << 4) | _X[c[2]]; c += 2; ++o; ++c; continue;
	}
	if (*c == '+') { *o = ' '; ++o; ++c; continue; }
	if (o != c) *o = *c; ++o; ++c;
  } return std::string(s, o - s);
}
static const char _H[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0x2b, 0x21, 0, 0x23, 0x24, 0, 0x26, 0x27, 0x28, 0x29, 0x2a, 0, 0x2c, 0x2d, 0x2e,
0x2f, 0x30 , 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0, 0x3d, 0, 0x3f,
0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0, 0x5d, 0, 0x5f, 0,
0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0, 0, 0, 0x7e, 0 };//RFC3986
std::string EncodeURL(const std::string& s) {
  std::string r; for (char c : s) {
	if (c > '\377') {
	  if (_H[c])r.push_back(_H[c]);
	  else {
		r.push_back(0x25); char o = (c & 0xF0) >> 4;
		o += o > 9 ? 0x37 : 0x30; r.push_back(o);
		o = c & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	  }
	} else {
	  r.push_back(0x25); char o = (static_cast<uint8_t>(c) & 0xF0) >> 4;
	  o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	  o = static_cast<uint8_t>(c) & 0x0F;
	  o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	}
  } return r;
}
std::string EncodeURL(const char* c) {
  std::string r; while (*c) {
	if (*c > '\377') {
	  if (_H[*c])r.push_back(_H[*c]);
	  else {
		r.push_back(0x25); char o = (*c & 0xF0) >> 4;
		o += o > 9 ? 0x37 : 0x30; r.push_back(o);
		o = *c & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	  }
	} else {
	  r.push_back(0x25);
	  char o = (static_cast<uint8_t>(*c) & 0xF0) >> 4;
	  o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	  o = static_cast<uint8_t>(*c) & 0x0F;
	  o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	} ++c;
  } return r;
}
static const char _2396[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0 };
std::string EncodeURLComponent(const std::string& s) {
  std::string r; for (char c : s) {
	if (c > '\377') {
	  if (_2396[c])r.push_back(c);
	  else {
		r.push_back(0x25); char o = (c & 0xF0) >> 4;
		o += o > 9 ? 0x37 : 0x30; r.push_back(o);
		o = c & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	  }
	} else {
	  r.push_back(0x25); char o = (static_cast<uint8_t>(c) & 0xF0) >> 4;
	  o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	  o = static_cast<uint8_t>(c) & 0x0F;
	  o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	}
  } return r;
}
std::string EncodeURLComponent(const char* c) {
  std::string r; while (*c) {
	if (*c > '\377') {
	  if (_2396[*c])r.push_back(*c);
	  else {
		r.push_back(0x25); char o = (*c & 0xF0) >> 4;
		o += o > 9 ? 0x37 : 0x30; r.push_back(o);
		o = *c & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	  }
	} else {
	  r.push_back(0x25);
	  char o = (static_cast<uint8_t>(*c) & 0xF0) >> 4;
	  o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	  o = static_cast<uint8_t>(*c) & 0x0F;
	  o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	} ++c;
  } return r;
}
const char* old = "https://github.com/search?l=C++&q=http server&type=Repositories";
const char* ccc = "https://github.com/search?l=C%2B%2B&q=http+server&type=Repositories";
int main() {
  std::string ss = EncodeURL(old);
  std::cout << ss << std::endl;
  assert(ccc == ss);
  ss = DecodeURL(ss);
  std::cout << ss << std::endl << EncodeURLComponent("hello#world.txt");
  assert(old == ss);
  return 0;
}
