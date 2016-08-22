#ifndef _WINDOWS_LINUX_H
#define _WINDOWS_LINUX_H

#ifdef _WIN32
#define SPRINTF(x,...) sprintf_s(x,256,__VA_ARGS__)
#define FPRINTF(x,...) fprintf_s(x,__VA_ARGS__)
#define FOPEN(_File,_Filename,_Mode) fopen_s(_File,_Filename,_Mode)
#define FREAD(x,...) fread_s(x,256,__VA_ARGS__)
#define fopens(_File,_Filename,_Mode) fopen_s(_File,_Filename,_Mode)
#define fprintfs(x,...) fprintf_s(x,__VA_ARGS__)
#define sprintfs(x,...) sprintf_s(x,256,__VA_ARGS__)
#define sscanfs(x,...) sscanf_s(x,__VA_ARGS__)
#elif __linux__
#define SPRINTF(x,y,...) sprintf(x,y ## __VA_ARGS__)
#define FPRINTF(x,y,...) fprintf(x,y ## __VA_ARGS__)
#define FOPEN(_File,_Filename,_Mode) (*_File=fopen(_Filename,_Mode))
#define FREAD(x,...) fread(x,__VA_ARGS__)
#define fopens(_File,_Filename,_Mode) (*_File=fopen(_Filename,_Mode))
#define fprintfs(x,...) fprintf_s(x,__VA_ARGS__)
#define sprintfs(x,...) sprintf(x,__VA_ARGS__)
#define sscanfs(x,...) sscanf(x,__VA_ARGS__)
#else
ERROR
#endif

#endif
