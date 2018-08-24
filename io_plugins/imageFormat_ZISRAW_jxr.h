#ifndef ZIS_RAW_JXR_H
#define ZIS_RAW_JXR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void jxr_decode(const void *in, size_t in_size, void *out, size_t out_size, unsigned short type, int verbose);
struct isis_type_map{
	struct {unsigned short u8bit,u16bit,u32bit,float32bit;}scalar;
	struct {unsigned short c24bit,c48bit;}color;
};
extern struct isis_type_map isis_types;

#ifdef __cplusplus
}
#endif

#endif //ZIS_RAW_JXR_H
