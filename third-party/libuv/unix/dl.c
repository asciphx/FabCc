  
#include "uv.h"
#include "internal.h"

#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <locale.h>
 static int uv__dlerror(uv_lib_t* lib);  int uv_dlopen(const char* filename, uv_lib_t* lib) { dlerror();  lib->errmsg = NULL; lib->handle = dlopen(filename, RTLD_LAZY); return lib->handle ? 0 : uv__dlerror(lib); }  void uv_dlclose(uv_lib_t* lib) { uv__free(lib->errmsg); lib->errmsg = NULL;  if (lib->handle) { dlclose(lib->handle); lib->handle = NULL; } }  int uv_dlsym(uv_lib_t* lib, const char* name, void** ptr) { dlerror();  *ptr = dlsym(lib->handle, name); return *ptr ? 0 : uv__dlerror(lib); }  const char* uv_dlerror(const uv_lib_t* lib) { return lib->errmsg ? lib->errmsg : "no error"; }  static int uv__dlerror(uv_lib_t* lib) { const char* errmsg;  uv__free(lib->errmsg);  errmsg = dlerror();  if (errmsg) { lib->errmsg = uv__strdup(errmsg); return -1; } else { lib->errmsg = NULL; return 0; } } 