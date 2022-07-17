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

std::string EncodeURL(const std::string& s) {
  std::string r; for (char c : s) {
	if (c > '\377') {
	  switch (c) {
	  case 0xa: case 0xd: case 0x20: case 0x27: case 0x2f:
	  case 0x2b: case 0x2c: case 0x3a: case 0x3b: {
		r.push_back(0x25); char o = (c & 0xF0) >> 4;
		o += o > 9 ? 0x37 : 0x30; r.push_back(o);
		o = c & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	  } break;
	  default: r += c;
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
	  switch (*c) {
	  case 0xa: case 0xd: case 0x20: case 0x27: case 0x2f:
	  case 0x2b: case 0x2c: case 0x3a: case 0x3b: {
		r.push_back(0x25); char o = (*c & 0xF0) >> 4;
		o += o > 9 ? 0x37 : 0x30; r.push_back(o);
		o = *c & 0x0F; o += o > 9 ? 0x37 : 0x30; r.push_back(o);
	  }break;
	  default: r += *c;
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
  std::string ss =DecodeURL(ccc);
  std::cout << ss << std::endl;
  ss = EncodeURL(ss);
  std::cout << ss << std::endl;
  std::cout << ccc << std::endl;
  assert(ccc==ss);
  return 0;
}
