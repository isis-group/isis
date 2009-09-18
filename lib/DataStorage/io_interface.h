#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif
	
#if defined(__STDC__) || defined(__cplusplus)
	extern void isis_io_load(const char* filename,const char* dialect);
#else
	extern void isis_io_load();
#endif
	
#ifdef __cplusplus
}
#endif

#endif //IO_INTERFACE_H