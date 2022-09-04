
#define _GNU_SOURCE 1

#include "uv.h"
#include "internal.h"

#include <hurd.h>
#include <hurd/process.h>
#include <mach/task_info.h>
#include <mach/vm_statistics.h>
#include <mach/vm_param.h>

#include <inttypes.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
int uv_exepath(char* buffer, size_t* size) { kern_return_t err; string_t exepath; ssize_t copied; if (buffer == NULL || size == NULL || *size == 0) return UV_EINVAL; if (*size - 1 > 0) { err = proc_get_exe(getproc(), getpid(), exepath); if (err) return UV__ERR(err); } copied = uv__strscpy(buffer, exepath, *size); *size = copied < 0 ? strlen(buffer) : (size_t) copied; return 0;}int uv_resident_set_memory(size_t* rss) { kern_return_t err; struct task_basic_info bi; mach_msg_type_number_t count; count = TASK_BASIC_INFO_COUNT; err = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t) &bi, &count); if (err) return UV__ERR(err); *rss = bi.resident_size; return 0;}uint64_t uv_get_free_memory(void) { kern_return_t err; struct vm_statistics vmstats; err = vm_statistics(mach_task_self(), &vmstats); if (err) return 0; return vmstats.free_count * vm_page_size;}uint64_t uv_get_total_memory(void) { kern_return_t err; host_basic_info_data_t hbi; mach_msg_type_number_t cnt; cnt = HOST_BASIC_INFO_COUNT; err = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t) &hbi, &cnt); if (err) return 0; return hbi.memory_size;}int uv_uptime(double* uptime) { char buf[128]; if (0 == uv__slurp("/proc/uptime", buf, sizeof(buf))) if (1 == sscanf(buf, "%lf", uptime)) return 0; return UV__ERR(EIO);}void uv_loadavg(double avg[3]) { char buf[128]; if (0 == uv__slurp("/proc/loadavg", buf, sizeof(buf))) if (3 == sscanf(buf, "%lf %lf %lf", &avg[0], &avg[1], &avg[2])) return; }int uv_cpu_info(uv_cpu_info_t** cpu_infos, int* count) { kern_return_t err; host_basic_info_data_t hbi; mach_msg_type_number_t cnt;  cnt = HOST_BASIC_INFO_COUNT; err = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t) &hbi, &cnt); if (err) { err = UV__ERR(err); goto abort; } *cpu_infos = uv__calloc(hbi.avail_cpus, sizeof(**cpu_infos)); if (*cpu_infos == NULL) { err = UV_ENOMEM; goto abort; } *count = hbi.avail_cpus; return 0; abort: *cpu_infos = NULL; *count = 0; return err;}uint64_t uv_get_constrained_memory(void) { return 0; }