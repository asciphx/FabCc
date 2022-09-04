    
#include "uv.h"
#include "internal.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <net/if.h>
#include <sys/epoll.h>
#include <sys/param.h>
#include <sys/prctl.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define HAVE_IFADDRS_H 1

# if defined(__ANDROID_API__) && __ANDROID_API__ < 24
# undef HAVE_IFADDRS_H
#endif

#ifdef __UCLIBC__
# if __UCLIBC_MAJOR__ < 0 && __UCLIBC_MINOR__ < 9 && __UCLIBC_SUBLEVEL__ < 32
# undef HAVE_IFADDRS_H
# endif
#endif

#ifdef HAVE_IFADDRS_H
# include <ifaddrs.h>
# include <sys/socket.h>
# include <net/ethernet.h>
# include <netpacket/packet.h>
#endif 
 
#ifndef CLOCK_MONOTONIC_COARSE
# define CLOCK_MONOTONIC_COARSE 6
#endif
 
#ifndef CLOCK_BOOTTIME
# define CLOCK_BOOTTIME 7
#endif
 static int read_models(unsigned int numcpus, uv_cpu_info_t* ci); static int read_times(FILE* statfile_fp, unsigned int numcpus, uv_cpu_info_t* ci); static void read_speeds(unsigned int numcpus, uv_cpu_info_t* ci); static uint64_t read_cpufreq(unsigned int cpunum); int uv__platform_loop_init(uv_loop_t* loop) { loop->inotify_fd = -1; loop->inotify_watchers = NULL;  return uv__epoll_init(loop); }  int uv__io_fork(uv_loop_t* loop) { int err; void* old_watchers;  old_watchers = loop->inotify_watchers;  uv__close(loop->backend_fd); loop->backend_fd = -1; uv__platform_loop_delete(loop);  err = uv__platform_loop_init(loop); if (err) return err;  return uv__inotify_fork(loop, old_watchers); }  void uv__platform_loop_delete(uv_loop_t* loop) { if (loop->inotify_fd == -1) return; uv__io_stop(loop, &loop->inotify_read_watcher, POLLIN); uv__close(loop->inotify_fd); loop->inotify_fd = -1; } uint64_t uv__hrtime(uv_clocktype_t type) { static clock_t fast_clock_id = -1; struct timespec t; clock_t clock_id;  clock_id = CLOCK_MONOTONIC; if (type != UV_CLOCK_FAST) goto done;  clock_id = uv__load_relaxed(&fast_clock_id); if (clock_id != -1) goto done;  clock_id = CLOCK_MONOTONIC; if (0 == clock_getres(CLOCK_MONOTONIC_COARSE, &t)) if (t.tv_nsec <= 1 * 1000 * 1000) clock_id = CLOCK_MONOTONIC_COARSE;  uv__store_relaxed(&fast_clock_id, clock_id); done:  if (clock_gettime(clock_id, &t)) return 0; return t.tv_sec * (uint64_t) 1e9 + t.tv_nsec; }  int uv_resident_set_memory(size_t* rss) { char buf[1024]; const char* s; ssize_t n; long val; int fd; int i;  do fd = open("/proc/self/stat", O_RDONLY); while (fd == -1 && errno == EINTR);  if (fd == -1) return UV__ERR(errno);  do n = read(fd, buf, sizeof(buf) - 1); while (n == -1 && errno == EINTR);  uv__close(fd); if (n == -1) return UV__ERR(errno); buf[n] = '\0';  s = strchr(buf, ' '); if (s == NULL) goto err;  s += 1; if (*s != '(') goto err;  s = strchr(s, ')'); if (s == NULL) goto err;  for (i = 1; i <= 22; ++i) { s = strchr(s + 1, ' '); if (s == NULL) goto err; }  errno = 0; val = strtol(s, NULL, 10); if (errno != 0) goto err; if (val < 0) goto err;  *rss = val * getpagesize(); return 0; err: return UV_EINVAL; } int uv_uptime(double* uptime) { static volatile int no_clock_boottime; char buf[128]; struct timespec now; int r; if (0 == uv__slurp("/proc/uptime", buf, sizeof(buf))) if (1 == sscanf(buf, "%lf", uptime)) return 0;  if (no_clock_boottime) { retry_clock_gettime: r = clock_gettime(CLOCK_MONOTONIC, &now); } else if ((r = clock_gettime(CLOCK_BOOTTIME, &now)) && errno == EINVAL) { no_clock_boottime = 1; goto retry_clock_gettime; }  if (r) return UV__ERR(errno);  *uptime = now.tv_sec; return 0; }  static int uv__cpu_num(FILE* statfile_fp, unsigned int* numcpus) { unsigned int num; char buf[1024];  if (!fgets(buf, sizeof(buf), statfile_fp)) return UV_EIO;  num = 0; while (fgets(buf, sizeof(buf), statfile_fp)) { if (strncmp(buf, "cpu", 3)) break; ++num; }  if (num == 0) return UV_EIO;  *numcpus = num; return 0; }  int uv_cpu_info(uv_cpu_info_t** cpu_infos, int* count) { unsigned int numcpus; uv_cpu_info_t* ci; int err; FILE* statfile_fp;  *cpu_infos = NULL; *count = 0;  statfile_fp = uv__open_file("/proc/stat"); if (statfile_fp == NULL) return UV__ERR(errno);  err = uv__cpu_num(statfile_fp, &numcpus); if (err < 0) goto out;  err = UV_ENOMEM; ci = uv__calloc(numcpus, sizeof(*ci)); if (ci == NULL) goto out;  err = read_models(numcpus, ci); if (err == 0) err = read_times(statfile_fp, numcpus, ci);  if (err) { uv_free_cpu_info(ci, numcpus); goto out; }   if (ci[0].speed == 0) read_speeds(numcpus, ci);  *cpu_infos = ci; *count = numcpus; err = 0; out:  if (fclose(statfile_fp)) if (errno != EINTR && errno != EINPROGRESS) abort();  return err; }  static void read_speeds(unsigned int numcpus, uv_cpu_info_t* ci) { unsigned int num;  for (num = 0; num < numcpus; ++num) ci[num].speed = read_cpufreq(num) / 1000; } static int read_models(unsigned int numcpus, uv_cpu_info_t* ci) {
#if defined(__PPC__)
 static const char model_marker[] = "cpu\t\t: "; static const char speed_marker[] = "clock\t\t: ";
#else
 static const char model_marker[] = "model name\t: "; static const char speed_marker[] = "cpu MHz\t\t: ";
#endif
 const char* inferred_model; unsigned int model_idx; unsigned int speed_idx; unsigned int part_idx; char buf[1024]; char* model; FILE* fp; int model_id;  (void) &model_marker; (void) &speed_marker; (void) &speed_idx; (void) &part_idx; (void) &model; (void) &buf; (void) &fp; (void) &model_id;  model_idx = 0; speed_idx = 0; part_idx = 0; 
#if defined(__arm__) || defined(__i386__) || defined(__mips__) || defined(__aarch64__) || defined(__PPC__) || defined(__x86_64__)
  fp = uv__open_file("/proc/cpuinfo"); if (fp == NULL) return UV__ERR(errno);  while (fgets(buf, sizeof(buf), fp)) { if (model_idx < numcpus) { if (strncmp(buf, model_marker, sizeof(model_marker) - 1) == 0) { model = buf + sizeof(model_marker) - 1; model = uv__strndup(model, strlen(model) - 1);  if (model == NULL) { fclose(fp); return UV_ENOMEM; } ci[model_idx++].model = model; continue; } }
#if defined(__arm__) || defined(__mips__) || defined(__aarch64__)
 if (model_idx < numcpus) {
#if defined(__arm__)
  static const char model_marker[] = "Processor\t: ";
#elif defined(__aarch64__)
 static const char part_marker[] = "CPU part\t: ";  struct vendor_part { const int id; const char* name; };  static const struct vendor_part arm_chips[] = { { 0x811, "ARM810" }, { 0x920, "ARM920" }, { 0x922, "ARM922" }, { 0x926, "ARM926" }, { 0x940, "ARM940" }, { 0x946, "ARM946" }, { 0x966, "ARM966" }, { 0xa20, "ARM1020" }, { 0xa22, "ARM1022" }, { 0xa26, "ARM1026" }, { 0xb02, "ARM11 MPCore" }, { 0xb36, "ARM1136" }, { 0xb56, "ARM1156" }, { 0xb76, "ARM1176" }, { 0xc05, "Cortex-A5" }, { 0xc07, "Cortex-A7" }, { 0xc08, "Cortex-A8" }, { 0xc09, "Cortex-A9" }, { 0xc0d, "Cortex-A17" },  { 0xc0f, "Cortex-A15" }, { 0xc0e, "Cortex-A17" }, { 0xc14, "Cortex-R4" }, { 0xc15, "Cortex-R5" }, { 0xc17, "Cortex-R7" }, { 0xc18, "Cortex-R8" }, { 0xc20, "Cortex-M0" }, { 0xc21, "Cortex-M1" }, { 0xc23, "Cortex-M3" }, { 0xc24, "Cortex-M4" }, { 0xc27, "Cortex-M7" }, { 0xc60, "Cortex-M0+" }, { 0xd01, "Cortex-A32" }, { 0xd03, "Cortex-A53" }, { 0xd04, "Cortex-A35" }, { 0xd05, "Cortex-A55" }, { 0xd06, "Cortex-A65" }, { 0xd07, "Cortex-A57" }, { 0xd08, "Cortex-A72" }, { 0xd09, "Cortex-A73" }, { 0xd0a, "Cortex-A75" }, { 0xd0b, "Cortex-A76" }, { 0xd0c, "Neoverse-N1" }, { 0xd0d, "Cortex-A77" }, { 0xd0e, "Cortex-A76AE" }, { 0xd13, "Cortex-R52" }, { 0xd20, "Cortex-M23" }, { 0xd21, "Cortex-M33" }, { 0xd41, "Cortex-A78" }, { 0xd42, "Cortex-A78AE" }, { 0xd4a, "Neoverse-E1" }, { 0xd4b, "Cortex-A78C" }, };  if (strncmp(buf, part_marker, sizeof(part_marker) - 1) == 0) { model = buf + sizeof(part_marker) - 1;  errno = 0; model_id = strtol(model, NULL, 16); if ((errno != 0) || model_id < 0) { fclose(fp); return UV_EINVAL; }  for (part_idx = 0; part_idx < ARRAY_SIZE(arm_chips); ++part_idx) { if (model_id == arm_chips[part_idx].id) { model = uv__strdup(arm_chips[part_idx].name); if (model == NULL) { fclose(fp); return UV_ENOMEM; } ci[model_idx++].model = model; break; } } }
#else 
 static const char model_marker[] = "cpu model\t\t: ";
#endif
 if (strncmp(buf, model_marker, sizeof(model_marker) - 1) == 0) { model = buf + sizeof(model_marker) - 1; model = uv__strndup(model, strlen(model) - 1);  if (model == NULL) { fclose(fp); return UV_ENOMEM; } ci[model_idx++].model = model; continue; } }
#else 
 if (speed_idx < numcpus) { if (strncmp(buf, speed_marker, sizeof(speed_marker) - 1) == 0) { ci[speed_idx++].speed = atoi(buf + sizeof(speed_marker) - 1); continue; } }
#endif 
 }  fclose(fp);
#endif 
  inferred_model = "unknown"; if (model_idx > 0) inferred_model = ci[model_idx - 1].model;  while (model_idx < numcpus) { model = uv__strndup(inferred_model, strlen(inferred_model)); if (model == NULL) return UV_ENOMEM; ci[model_idx++].model = model; }  return 0; }  static int read_times(FILE* statfile_fp, unsigned int numcpus, uv_cpu_info_t* ci) { struct uv_cpu_times_s ts; unsigned int ticks; unsigned int multiplier; uint64_t user; uint64_t nice; uint64_t sys; uint64_t idle; uint64_t dummy; uint64_t irq; uint64_t num; uint64_t len; char buf[1024];  ticks = (unsigned int)sysconf(_SC_CLK_TCK); assert(ticks != (unsigned int) -1); assert(ticks != 0); multiplier = ((uint64_t)1000L / ticks);  rewind(statfile_fp);  if (!fgets(buf, sizeof(buf), statfile_fp)) abort();  num = 0;  while (fgets(buf, sizeof(buf), statfile_fp)) { if (num >= numcpus) break;  if (strncmp(buf, "cpu", 3)) break;  { unsigned int n; int r = sscanf(buf, "cpu%u ", &n); assert(r == 1); (void) r;  for (len = sizeof("cpu0"); n /= 10; ++len); }   if (6 != sscanf(buf + len, "%" PRIu64 " %" PRIu64 " %" PRIu64 "%" PRIu64 " %" PRIu64 " %" PRIu64, &user, &nice, &sys, &idle, &dummy, &irq)) abort();  ts.user = user * multiplier; ts.nice = nice * multiplier; ts.sys = sys * multiplier; ts.idle = idle * multiplier; ts.irq = irq * multiplier; ci[num++].cpu_times = ts; } assert(num == numcpus);  return 0; }  static uint64_t read_cpufreq(unsigned int cpunum) { uint64_t val; char buf[1024]; FILE* fp;  snprintf(buf,  sizeof(buf),  "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_cur_freq",  cpunum);  fp = uv__open_file(buf); if (fp == NULL) return 0;  if (fscanf(fp, "%" PRIu64, &val) != 1) val = 0;  fclose(fp);  return val; } 
#ifdef HAVE_IFADDRS_H
static int uv__ifaddr_exclude(struct ifaddrs *ent, int exclude_type) { if (!((ent->ifa_flags & IFF_UP) && (ent->ifa_flags & IFF_RUNNING))) return 1; if (ent->ifa_addr == NULL) return 1;  if (ent->ifa_addr->sa_family == PF_PACKET) return exclude_type; return !exclude_type; }
#endif
 int uv_interface_addresses(uv_interface_address_t** addresses, int* count) {
#ifndef HAVE_IFADDRS_H
 *count = 0; *addresses = NULL; return UV_ENOSYS;
#else
 struct ifaddrs *addrs, *ent; uv_interface_address_t* address; int i; struct sockaddr_ll *sll;  *count = 0; *addresses = NULL;  if (getifaddrs(&addrs)) return UV__ERR(errno);  for (ent = addrs; ent != NULL; ent = ent->ifa_next) { if (uv__ifaddr_exclude(ent, UV__EXCLUDE_IFADDR)) continue;  (*count)++; }  if (*count == 0) { freeifaddrs(addrs); return 0; }  *addresses = uv__calloc(*count, sizeof(**addresses)); if (!(*addresses)) { freeifaddrs(addrs); return UV_ENOMEM; }  address = *addresses;  for (ent = addrs; ent != NULL; ent = ent->ifa_next) { if (uv__ifaddr_exclude(ent, UV__EXCLUDE_IFADDR)) continue;  address->name = uv__strdup(ent->ifa_name);  if (ent->ifa_addr->sa_family == AF_INET6) { address->address.address6 = *((struct sockaddr_in6*) ent->ifa_addr); } else { address->address.address4 = *((struct sockaddr_in*) ent->ifa_addr); }  if (ent->ifa_netmask->sa_family == AF_INET6) { address->netmask.netmask6 = *((struct sockaddr_in6*) ent->ifa_netmask); } else { address->netmask.netmask4 = *((struct sockaddr_in*) ent->ifa_netmask); }  address->is_internal = !!(ent->ifa_flags & IFF_LOOPBACK);  ++address; }  for (ent = addrs; ent != NULL; ent = ent->ifa_next) { if (uv__ifaddr_exclude(ent, UV__EXCLUDE_IFPHYS)) continue;  address = *addresses;  for (i = 0; i < (*count); ++i) { size_t namelen = strlen(ent->ifa_name); if (strncmp(address->name, ent->ifa_name, namelen) == 0 && (address->name[namelen] == 0 || address->name[namelen] == ':')) { sll = (struct sockaddr_ll*)ent->ifa_addr; memcpy(address->phys_addr, sll->sll_addr, sizeof(address->phys_addr)); } ++address; } }  freeifaddrs(addrs);  return 0;
#endif
}  void uv_free_interface_addresses(uv_interface_address_t* addresses, int count) { int i;  for (i = 0; i < count; ++i) { uv__free(addresses[i].name); }  uv__free(addresses); }  void uv__set_process_title(const char* title) {
#if defined(PR_SET_NAME)
 prctl(PR_SET_NAME, title); 
#endif
}  static uint64_t uv__read_proc_meminfo(const char* what) { uint64_t rc; char* p; char buf[4096]; if (uv__slurp("/proc/meminfo", buf, sizeof(buf))) return 0;  p = strstr(buf, what);  if (p == NULL) return 0;  p += strlen(what);  rc = 0; sscanf(p, "%" PRIu64 " kB", &rc);  return rc * 1024; }  uint64_t uv_get_free_memory(void) { struct sysinfo info; uint64_t rc;  rc = uv__read_proc_meminfo("MemAvailable:");  if (rc != 0) return rc;  if (0 == sysinfo(&info)) return (uint64_t) info.freeram * info.mem_unit;  return 0; }  uint64_t uv_get_total_memory(void) { struct sysinfo info; uint64_t rc;  rc = uv__read_proc_meminfo("MemTotal:");  if (rc != 0) return rc;  if (0 == sysinfo(&info)) return (uint64_t) info.totalram * info.mem_unit;  return 0; }  static uint64_t uv__read_cgroups_uint64(const char* cgroup, const char* param) { char filename[256]; char buf[32];  uint64_t rc;  rc = 0; snprintf(filename, sizeof(filename), "/sys/fs/cgroup/%s/%s", cgroup, param); if (0 == uv__slurp(filename, buf, sizeof(buf))) sscanf(buf, "%" PRIu64, &rc);  return rc; }  uint64_t uv_get_constrained_memory(void) {  return uv__read_cgroups_uint64("memory", "memory.limit_in_bytes"); }  void uv_loadavg(double avg[3]) { struct sysinfo info; char buf[128]; if (0 == uv__slurp("/proc/loadavg", buf, sizeof(buf))) if (3 == sscanf(buf, "%lf %lf %lf", &avg[0], &avg[1], &avg[2])) return;  if (sysinfo(&info) < 0) return;  avg[0] = (double) info.loads[0] / 65536.0; avg[1] = (double) info.loads[1] / 65536.0; avg[2] = (double) info.loads[2] / 65536.0; } 