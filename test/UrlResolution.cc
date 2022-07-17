#include <iostream>
#include <cassert>
//#include <str.hh>
static char STD_HEX[0x7e] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
0, 0, 0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
std::string DecodeURL(const char* d) {
  std::string s(d); char* o = (char*)s.data(), * c = (char*)s.data();
  const char* e = c + s.length(); while (c < e) {
	if (*c == '%' && c < e - 2) {
	  *o = (STD_HEX[c[1]] << 4) | STD_HEX[c[2]]; c += 2;
	} else if (o != c) *o = *c; ++o; ++c;
  } return std::string(s.c_str(), o - s.c_str());// 0x67
}
static char _HEX_[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x2d , 0x2e, 0, 0x30 , 0x31, 0x32, 0x33, 0x34,
0x35, 0x36, 0x37, 0x38, 0x39, 0, 0, 0, 0, 0, 0, 0, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,
0, 0, 0, 0, 0x5f, 0, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d,
0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0, 0, 0, 0x7e, 0 };
std::string EncodeURL(const std::string& s) {
  std::string r; for (char c : s) {
	if (c > '\377') {
	  if (_HEX_[c])r.push_back(c);
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
	  if (_HEX_[*c])r.push_back(*c);
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
const char* ccc = "http%3A%2F%2Fcontoso.sharepoint.com%2FFinance%2FProfit%20and%20Loss%20Statement.xlsx";
int main() {
  std::string xlsx = "http%3A%2F%2Fcontoso.sharepoint.com%2FFinance%2FProfit";
  std::string ss = DecodeURL(ccc);
  std::cout << ss << std::endl;
  ss = EncodeURL(ss);
  std::cout << ss << std::endl;
  std::cout << ccc << std::endl;
  assert(ccc == ss);
  return 0;
}
