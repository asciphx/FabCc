#pragma once // (default config)
#ifndef DEFAULT_ENABLE_LOGGING
#define DEFAULT_ENABLE_LOGGING 0
#endif
#define DEFAULT_LOG_LEVEL 3//DEBUG=1, INFO, WARNING, ERR, CRITICAL
#define DEFAULT_PORT 8080

//#define DISABLE_HOME
//#define ENABLE_SSL
#define FILE_TIME "max-age=54000,immutable"//Static resource cache seconds(= 900 minute)
#define STATIC_DIRECTORY "static/"//Make sure you bring '/' with you at the end
#define UPLOAD_DIRECTORY "upload/"//Make sure you bring '/' with you at the end
#define SERVER_NAME "FabCc/0.2"//the server name config
#define HOME_PAGE "index.html"//default home page(app.home(?))

//Cors config
#define AccessControlAllowOrigin "*"
#define AccessControlAllowCredentials "true"
#define AccessControlAllowHeaders "content-type,cache-control,x-requested-with,authorization"
#define AccessControlAllowMethods "GET,POST,DELETE,PUT,OPTIONS,HEAD"
#define SHOW_SERVER_NAME 1
#define CACHE_MENU_TIME_SECOND 5


#if defined(_MSC_VER)
#if _MSC_VER < 1900
#define MSVC_WORKAROUND
#define constexpr const
#define noexcept throw()
#endif
#endif

