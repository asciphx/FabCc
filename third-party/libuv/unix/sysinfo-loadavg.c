  
#include "uv.h"
#include "internal.h"

#include <stdint.h>
#include <sys/sysinfo.h>
 void uv_loadavg(double avg[3]) { struct sysinfo info;  if (sysinfo(&info) < 0) return;  avg[0] = (double) info.loads[0] / 65536.0; avg[1] = (double) info.loads[1] / 65536.0; avg[2] = (double) info.loads[2] / 65536.0; } 