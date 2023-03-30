#include "lwip/apps/fs.h"
#include "lwip/def.h"

#ifndef FS_FILE_FLAGS_HEADER_INCLUDED
#define FS_FILE_FLAGS_HEADER_INCLUDED 1
#endif
#ifndef FS_FILE_FLAGS_HEADER_PERSISTENT
#define FS_FILE_FLAGS_HEADER_PERSISTENT 0
#endif
/* FSDATA_FILE_ALIGNMENT: 0=off, 1=by variable, 2=by include */
#ifndef FSDATA_FILE_ALIGNMENT
#define FSDATA_FILE_ALIGNMENT 0
#endif
#ifndef FSDATA_ALIGN_PRE
#define FSDATA_ALIGN_PRE
#endif
#ifndef FSDATA_ALIGN_POST
#define FSDATA_ALIGN_POST
#endif
#if FSDATA_FILE_ALIGNMENT == 2
#include "fsdata_alignment.h"
#endif

#define file_NULL (struct fsdata_file *)NULL

static const unsigned char FSDATA_ALIGN_PRE
    data__index_html[] FSDATA_ALIGN_POST = {
        "/cgi.html\0"
        "HTTP/1.1 200 OK\n"
        "Server: lwIP/2.0.3d\n"
        "Content-Length: 9\n"
        "Connection: keep-alive\n"
        "Content-Type: text/plain\n"
        "\nSuccess\n\n"};

const struct fsdata_file file__index_html[] = {{
    file_NULL,
    data__index_html,
    data__index_html + 10,
    sizeof(data__index_html) - 10,
    FS_FILE_FLAGS_HEADER_INCLUDED | FS_FILE_FLAGS_HEADER_PERSISTENT |
        FS_FILE_FLAGS_HEADER_HTTPVER_1_1,
}};

#define FS_ROOT file__index_html
#define FS_NUMFILES 1
