#pragma once
#include <unordered_map>
#include "h/common.h"
namespace fc {
  static std::unordered_map<std::string_view, std::string_view>
    content_any_types = {
    {"3ds","image/x-3ds"},
    {"3gp","video/3gpp"},
    {"7z","application/x-7z-compressed"},
    {"aac","audio/x-aac"},
    {"ai","application/postscript"},
    {"apk","application/vnd.android.package-archive"},
    {"asm","text/x-asm"},
    {"au","audio/basic"},
    {"avi","video/x-msvideo"},
    {"bat","application/x-msdownload"},
    {"bin","application/octet-stream"},
    {"bmp","image/bmp"},
    {"c","text/x-c"},
    {"cc","text/x-c"},
    {"conf","text/plain"},
    {"cpp","text/x-c"},
    {"css","text/css"},
    {"curl","text/vnd.curl"},
    {"dart","application/vnd.dart"},
    {"dms","application/octet-stream"},
    {"dna","application/vnd.dna"},
    {"dts","audio/vnd.dts"},
    {"dump","application/octet-stream"},
    {"ecma","application/ecmascript"},
    {"eva","application/x-eva"},
    {"f","text/x-fortran"},
    {"flv","video/x-flv"},
    {"gif","image/gif"},
    {"h","text/x-c"},
    {"h261","video/h261"},
    {"h263","video/h263"},
    {"h264","video/h264"},
    {"hh","text/x-c"},
    {"htm","text/html"},
    {"html","text/html"},
    {"ico","image/x-icon"},
    {"ink","application/inkml+xml"},
    {"ipfix","application/ipfix"},
    {"iso","application/x-iso9660-image"},
    {"jar","application/java-archive"},
    {"java","text/x-java-source"},
    {"jpeg","image/jpeg"},
    {"jpg","image/jpeg"},
    {"jpgv","video/jpeg"},
    {"js","text/javascript"},
    {"json","application/json"},
    {"jsonml","application/jsonml+json"},
    {"kar","audio/midi"},
    {"ktx","image/ktx"},
    {"lanim","application/octet-stream"},
    {"latex","application/x-latex"},
    {"lmesh","application/octet-stream"},
    {"list","text/plain"},
    {"log","text/plain"},
    {"m1v","video/mpeg"},
    {"m21","application/mp21"},
    {"m2a","audio/mpeg"},
    {"m2v","video/mpeg"},
    {"m3a","audio/mpeg"},
    {"m3u","audio/x-mpegurl"},
    {"m3u8","application/vnd.apple.mpegurl"},
    {"m4a","audio/mp4"},
    {"m4u","video/vnd.mpegurl"},
    {"m4v","video/x-m4v"},
    {"mesh","model/mesh"},
    {"meta4","application/metalink4+xml"},
    {"metalink","application/metalink+xml"},
    {"mid","audio/midi"},
    {"midi","audio/midi"},
    {"mie","application/x-mie"},
    {"mif","application/vnd.mif"},
    {"mj2","video/mj2"},
    {"mjp2","video/mj2"},
    {"mk3d","video/x-matroska"},
    {"mka","audio/x-matroska"},
    {"mks","video/x-matroska"},
    {"mkv","video/x-matroska"},
    {"mng","video/x-mng"},
    {"mods","application/mods+xml"},
    {"mov","video/quicktime"},
    {"movie","video/x-sgi-movie"},
    {"mp2","audio/mpeg"},
    {"mp21","application/mp21"},
    {"mp2a","audio/mpeg"},
    {"mp3","audio/mpeg"},
    {"mp4","video/mp4"},
    {"mp4a","audio/mp4"},
    {"mp4s","application/mp4"},
    {"mp4v","video/mp4"},
    {"mpe","video/mpeg"},
    {"mpeg","video/mpeg"},
    {"mpg","video/mpeg"},
    {"mpg4","video/mp4"},
    {"mpga","audio/mpeg"},
    {"msh","model/mesh"},
    {"msi","application/x-msdownload"},
    {"mxu","video/vnd.mpegurl"},
    {"n3","text/n3"},
    {"nb","application/mathematica"},
    {"nc","application/x-netcdf"},
    {"ncx","application/x-dtbncx+xml"},
    {"nfo","text/x-nfo"},
    {"npx","image/vnd.net-fpx"},
    {"obj","application/x-tgif"},
    {"p","text/x-pascal"},
    {"pas","text/x-pascal"},
    {"pdb","application/vnd.palm"},
    {"pdf","application/pdf"},
    {"pfr","application/font-tdpfr"},
    {"pic","image/x-pict"},
    {"pkg","application/octet-stream"},
    {"pls","application/pls+xml"},
    {"png","image/png"},
    {"ppt","application/vnd.ms-powerpoint"},
    {"psd","image/vnd.adobe.photoshop"},
    {"qt","video/quicktime"},
    {"rar","application/x-rar-compressed"},
    {"rgb","image/x-rgb"},
    {"rip","audio/vnd.rip"},
    {"rmi","audio/midi"},
    {"rmp","audio/x-pn-realaudio-plugin"},
    {"rmvb","application/vnd.rn-realmedia-vbr"},
    {"roff","text/troff"},
    {"rs","application/rls-services+xml"},
    {"rsd","application/rsd+xml"},
    {"rss","application/rss+xml"},
    {"rtf","application/rtf"},
    {"rtx","text/richtext"},
    {"s","text/x-asm"},
    {"sh","application/x-sh"},
    {"so","application/octet-stream"},
    {"sql","application/x-sql"},
    {"sse","application/vnd.kodak-descriptor"},
    {"ssf","application/vnd.epson.ssf"},
    {"ssml","application/ssml+xml"},
    {"st","application/vnd.sailingtracker.track"},
    {"svg","image/svg+xml"},
    {"swa","application/x-director"},
    {"swf","application/x-shockwave-flash"},
    {"t","text/troff"},
    {"tar","application/x-tar"},
    {"text","text/plain"},
    {"tga","image/x-tga"},
    {"tif","image/tiff"},
    {"tiff","image/tiff"},
    {"tr","text/troff"},
    {"ttc","font/collection"},
    {"ttf","font/ttf"},
    {"ttl","text/turtle"},
    {"txt","text/plain"},
    {"uri","text/uri-list"},
    {"uris","text/uri-list"},
    {"urls","text/uri-list"},
    {"vtu","model/vnd.vtu"},
    {"wasm","application/wasm"},
    {"wav","audio/x-wav"},
    {"wbmp","image/vnd.wap.wbmp"},
    {"wdp","image/vnd.ms-photo"},
    {"weba","audio/webm"},
    {"webm","video/webm"},
    {"webp","image/webp"},
    {"wm","video/x-ms-wm"},
    {"wma","audio/x-ms-wma"},
    {"wml","text/vnd.wap.wml"},
    {"wmls","text/vnd.wap.wmlscript"},
    {"wmv","video/x-ms-wmv"},
    {"wmx","video/x-ms-wmx"},
    {"woff","font/woff"},
    {"woff2","font/woff2"},
    {"wps","application/vnd.ms-works"},
    {"xls","application/vnd.ms-excel"},
    {"xml","application/xml"},
    {"zip","application/zip"}
  };
}
