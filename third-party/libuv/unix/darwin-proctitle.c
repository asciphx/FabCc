  
#include "uv.h"
#include "internal.h"

#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <TargetConditionals.h>

#if !TARGET_OS_IPHONE
#include "darwin-stub.h"
#endif
 static int uv__pthread_setname_np(const char* name) { char namebuf[64];  int err;  strncpy(namebuf, name, sizeof(namebuf) - 1); namebuf[sizeof(namebuf) - 1] = '\0';  err = pthread_setname_np(namebuf); if (err) return UV__ERR(err);  return 0; }  int uv__set_process_title(const char* title) {
#if TARGET_OS_IPHONE
 return uv__pthread_setname_np(title);
#else
 CFStringRef (*pCFStringCreateWithCString)(CFAllocatorRef, const char*, CFStringEncoding); CFBundleRef (*pCFBundleGetBundleWithIdentifier)(CFStringRef); void *(*pCFBundleGetDataPointerForName)(CFBundleRef, CFStringRef); void *(*pCFBundleGetFunctionPointerForName)(CFBundleRef, CFStringRef); CFTypeRef (*pLSGetCurrentApplicationASN)(void); OSStatus (*pLSSetApplicationInformationItem)(int,  CFTypeRef,  CFStringRef,  CFStringRef,  CFDictionaryRef*); void* application_services_handle; void* core_foundation_handle; CFBundleRef launch_services_bundle; CFStringRef* display_name_key; CFDictionaryRef (*pCFBundleGetInfoDictionary)(CFBundleRef); CFBundleRef (*pCFBundleGetMainBundle)(void); CFDictionaryRef (*pLSApplicationCheckIn)(int, CFDictionaryRef); void (*pLSSetApplicationLaunchServicesServerConnectionStatus)(uint64_t, void*); CFTypeRef asn; int err;  err = UV_ENOENT; application_services_handle = dlopen("/System/Library/Frameworks/"  "ApplicationServices.framework/"  "Versions/A/ApplicationServices",  RTLD_LAZY | RTLD_LOCAL); core_foundation_handle = dlopen("/System/Library/Frameworks/" "CoreFoundation.framework/" "Versions/A/CoreFoundation", RTLD_LAZY | RTLD_LOCAL);  if (application_services_handle == NULL || core_foundation_handle == NULL) goto out;  *(void **)(&pCFStringCreateWithCString) = dlsym(core_foundation_handle, "CFStringCreateWithCString"); *(void **)(&pCFBundleGetBundleWithIdentifier) = dlsym(core_foundation_handle, "CFBundleGetBundleWithIdentifier"); *(void **)(&pCFBundleGetDataPointerForName) = dlsym(core_foundation_handle, "CFBundleGetDataPointerForName"); *(void **)(&pCFBundleGetFunctionPointerForName) = dlsym(core_foundation_handle, "CFBundleGetFunctionPointerForName");  if (pCFStringCreateWithCString == NULL || pCFBundleGetBundleWithIdentifier == NULL || pCFBundleGetDataPointerForName == NULL || pCFBundleGetFunctionPointerForName == NULL) { goto out; } 
#define S(s) pCFStringCreateWithCString(NULL, (s), kCFStringEncodingUTF8)
 launch_services_bundle = pCFBundleGetBundleWithIdentifier(S("com.apple.LaunchServices"));  if (launch_services_bundle == NULL) goto out;  *(void **)(&pLSGetCurrentApplicationASN) = pCFBundleGetFunctionPointerForName(launch_services_bundle,  S("_LSGetCurrentApplicationASN"));  if (pLSGetCurrentApplicationASN == NULL) goto out;  *(void **)(&pLSSetApplicationInformationItem) = pCFBundleGetFunctionPointerForName(launch_services_bundle,  S("_LSSetApplicationInformationItem"));  if (pLSSetApplicationInformationItem == NULL) goto out;  display_name_key = pCFBundleGetDataPointerForName(launch_services_bundle, S("_kLSDisplayNameKey"));  if (display_name_key == NULL || *display_name_key == NULL) goto out;  *(void **)(&pCFBundleGetInfoDictionary) = dlsym(core_foundation_handle,  "CFBundleGetInfoDictionary"); *(void **)(&pCFBundleGetMainBundle) = dlsym(core_foundation_handle,  "CFBundleGetMainBundle"); if (pCFBundleGetInfoDictionary == NULL || pCFBundleGetMainBundle == NULL) goto out;  *(void **)(&pLSApplicationCheckIn) = pCFBundleGetFunctionPointerForName( launch_services_bundle, S("_LSApplicationCheckIn"));  if (pLSApplicationCheckIn == NULL) goto out;  *(void **)(&pLSSetApplicationLaunchServicesServerConnectionStatus) = pCFBundleGetFunctionPointerForName( launch_services_bundle, S("_LSSetApplicationLaunchServicesServerConnectionStatus"));  if (pLSSetApplicationLaunchServicesServerConnectionStatus == NULL) goto out;  pLSSetApplicationLaunchServicesServerConnectionStatus(0, NULL);  pLSApplicationCheckIn(-2, pCFBundleGetInfoDictionary(pCFBundleGetMainBundle()));  asn = pLSGetCurrentApplicationASN();  err = UV_EBUSY; if (asn == NULL) goto out;  err = UV_EINVAL; if (pLSSetApplicationInformationItem(-2, asn,  *display_name_key,  S(title),  NULL) != noErr) { goto out; }  uv__pthread_setname_np(title);  err = 0; out: if (core_foundation_handle != NULL) dlclose(core_foundation_handle);  if (application_services_handle != NULL) dlclose(application_services_handle);  return err;
#endif 
} 