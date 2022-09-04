
#include "uv.h"
#include "internal.h"

#include <sys/sysinfo.h>
#include <unistd.h>
int uv_uptime(double* uptime) { struct sysinfo info; if (sysinfo(&info) < 0) return UV__ERR(errno); *uptime = info.uptime; return 0;}int uv_resident_set_memory(size_t* rss) { *rss = 0; return 0;}int uv_cpu_info(uv_cpu_info_t** cpu_infos, int* count) { *cpu_infos = NULL; *count = 0; return UV_ENOSYS;}uint64_t uv_get_constrained_memory(void) { return 0; }