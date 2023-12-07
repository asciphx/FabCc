
#include "zfstream.h"
int main() {  gzofstream os( 1, ios::out );  os << "Hello, Mommy" << endl; os << setcompressionlevel( Z_NO_COMPRESSION ); os << "hello, hello, hi, ho!" << endl; setcompressionlevel( os, Z_DEFAULT_COMPRESSION ) << "I'm compressing again" << endl; os.close(); return 0;}