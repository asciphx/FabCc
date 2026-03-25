#ifndef PTHREAD_H
#define PTHREAD_H
#ifdef _WIN32
#include "windows.h"
typedef HANDLE pthread_t;
static void thread_create(pthread_t* t, void* (*func)(void*), void* arg) {
  *t = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, NULL);
}
static void thread_join(pthread_t t) {
  WaitForSingleObject(t, INFINITE);
  CloseHandle(t);
}
#else
#include <pthread.h>
static void thread_create(pthread_t* t, void* (*func)(void*), void* arg) {
  pthread_create(t, NULL, func, arg);
}
static void thread_join(pthread_t t) {
  pthread_join(t, NULL);
}
#endif
#endif