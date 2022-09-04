
#ifndef UV_WIN_FS_FD_HASH_INL_H_
#define UV_WIN_FS_FD_HASH_INL_H_

#include "uv.h"
#include "internal.h"

#define UV__FD_HASH_SIZE 256
#define UV__FD_HASH_GROUP_SIZE 16
struct uv__fd_info_s { int flags; BOOLEAN is_directory; HANDLE mapping; LARGE_INTEGER size; LARGE_INTEGER current_pos;};struct uv__fd_hash_entry_s { uv_file fd; struct uv__fd_info_s info;};struct uv__fd_hash_entry_group_s { struct uv__fd_hash_entry_s entries[UV__FD_HASH_GROUP_SIZE]; struct uv__fd_hash_entry_group_s* next;};struct uv__fd_hash_bucket_s { size_t size; struct uv__fd_hash_entry_group_s* data;};static uv_mutex_t uv__fd_hash_mutex;static struct uv__fd_hash_entry_group_s uv__fd_hash_entry_initial[UV__FD_HASH_SIZE * UV__FD_HASH_GROUP_SIZE];static struct uv__fd_hash_bucket_s uv__fd_hash[UV__FD_HASH_SIZE];INLINE static void uv__fd_hash_init(void) { size_t i; int err; err = uv_mutex_init(&uv__fd_hash_mutex); if (err) { uv_fatal_error(err, "uv_mutex_init"); } for (i = 0; i < ARRAY_SIZE(uv__fd_hash); ++i) { uv__fd_hash[i].size = 0; uv__fd_hash[i].data = uv__fd_hash_entry_initial + i * UV__FD_HASH_GROUP_SIZE; }}
#define FIND_COMMON_VARIABLES  unsigned i;  unsigned bucket = fd % ARRAY_SIZE(uv__fd_hash);  struct uv__fd_hash_entry_s* entry_ptr = NULL;  struct uv__fd_hash_entry_group_s* group_ptr;   struct uv__fd_hash_bucket_s* bucket_ptr = &uv__fd_hash[bucket];


#define FIND_IN_GROUP_PTR(group_size)  do {   for (i = 0; i < group_size; ++i) {   if (group_ptr->entries[i].fd == fd) {  entry_ptr = &group_ptr->entries[i];  break;   }  }  } while (0)


#define FIND_IN_BUCKET_PTR()   do {   size_t first_group_size = bucket_ptr->size % UV__FD_HASH_GROUP_SIZE;   if (bucket_ptr->size != 0 && first_group_size == 0)  first_group_size = UV__FD_HASH_GROUP_SIZE;   group_ptr = bucket_ptr->data;  FIND_IN_GROUP_PTR(first_group_size);   for (group_ptr = group_ptr->next;   group_ptr != NULL && entry_ptr == NULL;    group_ptr = group_ptr->next)  FIND_IN_GROUP_PTR(UV__FD_HASH_GROUP_SIZE);   } while (0)

INLINE static int uv__fd_hash_get(int fd, struct uv__fd_info_s* info) { FIND_COMMON_VARIABLES uv_mutex_lock(&uv__fd_hash_mutex); FIND_IN_BUCKET_PTR(); if (entry_ptr != NULL) { *info = entry_ptr->info; } uv_mutex_unlock(&uv__fd_hash_mutex); return entry_ptr != NULL;}INLINE static void uv__fd_hash_add(int fd, struct uv__fd_info_s* info) { FIND_COMMON_VARIABLES uv_mutex_lock(&uv__fd_hash_mutex); FIND_IN_BUCKET_PTR(); if (entry_ptr == NULL) { i = bucket_ptr->size % UV__FD_HASH_GROUP_SIZE; if (bucket_ptr->size != 0 && i == 0) { struct uv__fd_hash_entry_group_s* new_group_ptr = uv__malloc(sizeof(*new_group_ptr)); if (new_group_ptr == NULL) { uv_fatal_error(ERROR_OUTOFMEMORY, "uv__malloc"); } new_group_ptr->next = bucket_ptr->data; bucket_ptr->data = new_group_ptr; } bucket_ptr->size += 1; entry_ptr = &bucket_ptr->data->entries[i]; entry_ptr->fd = fd; } entry_ptr->info = *info; uv_mutex_unlock(&uv__fd_hash_mutex);}INLINE static int uv__fd_hash_remove(int fd, struct uv__fd_info_s* info) { FIND_COMMON_VARIABLES uv_mutex_lock(&uv__fd_hash_mutex); FIND_IN_BUCKET_PTR(); if (entry_ptr != NULL) { *info = entry_ptr->info; bucket_ptr->size -= 1; i = bucket_ptr->size % UV__FD_HASH_GROUP_SIZE; if (entry_ptr != &bucket_ptr->data->entries[i]) { *entry_ptr = bucket_ptr->data->entries[i]; } if (bucket_ptr->size != 0 && bucket_ptr->size % UV__FD_HASH_GROUP_SIZE == 0) { struct uv__fd_hash_entry_group_s* old_group_ptr = bucket_ptr->data; bucket_ptr->data = old_group_ptr->next; uv__free(old_group_ptr); } } uv_mutex_unlock(&uv__fd_hash_mutex); return entry_ptr != NULL;}
#undef FIND_COMMON_VARIABLES
#undef FIND_IN_GROUP_PTR
#undef FIND_IN_BUCKET_PTR

#endif 
