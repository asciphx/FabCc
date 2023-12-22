#ifndef CONF_H
#define CONF_H
#ifndef DEFAULT_ENABLE_LOGGING
#define DEFAULT_ENABLE_LOGGING 0
#endif
#if _DEBUG
#    define DEBUG printf
#else
#    define DEBUG(...)
#endif
#if DEFAULT_ENABLE_LOGGING
#define LOG_GER(_) std::cout << _;
#else
#define LOG_GER(_)
#endif
//#define  $_(_)  #@_

//#define DISABLE_HOME
//#define ENABLE_SSL
#define DEFAULT_PORT 8080
#define FILE_TIME "max-age=66666,immutable"//Static resource cache seconds(= 0.77 days)
#define STATIC_DIRECTORY "static/"//Make sure you bring '/' with you at the end
#define UPLOAD_DIRECTORY "upload/"//Make sure you bring '/' with you at the end
#define SERVER_NAME "Nod/1.0"//the server name config
#define HOME_PAGE "index.html"//default home page(app.home(?))

//Cors config
#define AccessControlAllowOrigin "*"
//#define AccessControlAllowCredentials "true"
//#define AccessControlAllowHeaders "content-type,cache-control,x-requested-with"
//#define AccessControlAllowMethods "GET,POST,DELETE,PUT,PATCH"
#define SHOW_SERVER_NAME 1
#define CACHE_HTML_TIME_SECOND 6

#if defined(_MSC_VER)
#if _MSC_VER < 1900
#define MSVC_WORKAROUND
#define constexpr const
#define noexcept throw()
#endif
#if defined(_MSC_VER) || defined(_WIN32)
#include <locale.h>
#define WIN32_LEAN_AND_MEAN
#endif
#define TYPE_GET(t, ptr, member) (t*)(ptr)-((size_t)&reinterpret_cast<char const volatile&>(((t*)0)->member))
#endif
#endif

