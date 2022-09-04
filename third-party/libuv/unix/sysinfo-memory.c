  
#include "uv.h"
#include "internal.h"

#include <stdint.h>
#include <sys/sysinfo.h>
 uint64_t uv_get_free_memory(void) { struct sysinfo info;  if (sysinfo(&info) == 0) return (uint64_t) info.freeram * info.mem_unit; return 0; } uint64_t uv_get_total_memory(void) { struct sysinfo info;  if (sysinfo(&info) == 0) return (uint64_t) info.totalram * info.mem_unit; return 0; } 