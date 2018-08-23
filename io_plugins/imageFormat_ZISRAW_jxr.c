#include "imageFormat_ZISRAW_jxr.h"
#define __ANSI__
#include <JXRGlue.h>
#include <windowsmediaphoto.h>

struct isis_type_map isis_types;

// struct JpegXRGlobal *initLib(){
// 	struct JpegXRGlobal ret;
// 	PKCreateFactory(&ret.pFactory, PK_SDK_VERSION);
// 	PKCreateCodecFactory(&ret.pCodecFactory, WMP_SDK_VERSION);
// }

const GUID *selectColorFormat(PKImageDecode* pDecoder,unsigned short *isis_type){
	if(pDecoder->WMP.wmiI.cfColorFormat==Y_ONLY){ // grayscale
		switch(pDecoder->WMP.wmiI.bdBitDepth){
			case BD_16S:
			case BD_16F:
			case BD_16: 
				*isis_type = isis_types.scalar.u16bit;     
				return &GUID_PKPixelFormat16bppGray;
				break;
			case BD_32S:
			case BD_32: 
				*isis_type = isis_types.scalar.u32bit;     
				return &GUID_PKPixelFormat32bppGrayFixedPoint;
				break;
			case BD_32F:
				*isis_type = isis_types.scalar.float32bit; 
				return &GUID_PKPixelFormat32bppGrayFloat;
				break;
			case BD_8:  
			default: 
				*isis_type = isis_types.scalar.u8bit;         
				return &GUID_PKPixelFormat8bppGray;
				break;
		}
	} else { //RGB
		switch(pDecoder->WMP.wmiI.bdBitDepth){
			case BD_16S:
			case BD_16F:
			case BD_16: 
			case BD_32S:
			case BD_32: 
			case BD_32F:
				*isis_type = isis_types.color.c48bit; 
				return &GUID_PKPixelFormat48bppRGB;break;
			case BD_8:  
			default: 
				*isis_type = isis_types.color.c24bit; 
				return &GUID_PKPixelFormat24bppRGB;
				break;
		}
	}
}

void jxr_decode(const void *in, size_t in_size, void **out, size_t out_size[2], unsigned short *isis_type, int verbose){
	ERR e;
	struct WMPStream* inStream;
	PKImageDecode* pDecoder;

	e=CreateWS_Memory(&inStream,in,in_size);

	e=PKCodecFactory_CreateCodec(&IID_PKImageWmpDecode, (void **) &pDecoder);
	e=pDecoder->Initialize(pDecoder, inStream);
	
	PKPixelInfo PIto,PIfrom;
	PIto.pGUIDPixFmt = selectColorFormat(pDecoder,isis_type);
	PIfrom.pGUIDPixFmt = &pDecoder->guidPixFormat;
	
	if(IsEqualGUID(PIto.pGUIDPixFmt, &GUID_PKPixelFormat8bppGray) || IsEqualGUID(PIto.pGUIDPixFmt, &GUID_PKPixelFormat16bppGray)){ // ** => Y transcoding
        pDecoder->guidPixFormat = *PIto.pGUIDPixFmt;
        pDecoder->WMP.wmiI.cfColorFormat = Y_ONLY;
    } else if(IsEqualGUID(PIto.pGUIDPixFmt, &GUID_PKPixelFormat24bppRGB) && pDecoder->WMP.wmiI.cfColorFormat == CMYK){ // CMYK = > RGB
        pDecoder->WMP.wmiI.cfColorFormat = CF_RGB;
        pDecoder->guidPixFormat = *PIto.pGUIDPixFmt;
        pDecoder->WMP.wmiI.bRGB = 1; //RGB
    }
    PixelFormatLookup(&PIto, LOOKUP_FORWARD);
	PixelFormatLookup(&PIfrom, LOOKUP_FORWARD);

//     pDecoder->WMP.bIgnoreOverlap = TRUE;
	pDecoder->WMP.wmiSCP.uAlphaMode = 0;

    pDecoder->WMP.wmiI.cfColorFormat = PIto.cfColorFormat; 
    pDecoder->WMP.wmiI.bdBitDepth = PIto.bdBitDepth;
    pDecoder->WMP.wmiI.cBitsPerUnit = PIto.cbitUnit;
	
    pDecoder->WMP.wmiI.cROIWidth  = pDecoder->WMP.wmiI.cThumbnailWidth = pDecoder->WMP.wmiI.cWidth;
    pDecoder->WMP.wmiI.cROIHeight = pDecoder->WMP.wmiI.cThumbnailHeight = pDecoder->WMP.wmiI.cHeight;
    pDecoder->WMP.wmiI.bSkipFlexbits = FALSE;

    pDecoder->WMP.wmiSCP.bVerbose = verbose;

	U32 cFrame;
	pDecoder->GetFrameCount(pDecoder, &cFrame);
	assert(cFrame==1);
	
	PKFormatConverter* pConverter = NULL;

	Float rX = 0.0, rY = 0.0;
	pDecoder->GetResolution(pDecoder, &rX, &rY);
	PKRect rect = {0, 0, pDecoder->WMP.wmiI.cROIWidth, pDecoder->WMP.wmiI.cROIHeight};

	//================================
	e=PKCodecFactory_CreateFormatConverter(&pConverter);
	e=PKFormatConverter_Initialize(pConverter,pDecoder,NULL,*PIto.pGUIDPixFmt);
		
	U32 cbStrideFrom = (BD_1 == PIfrom.bdBitDepth ? ((PIfrom.cbitUnit * rect.Width + 7) >> 3) : (((PIfrom.cbitUnit + 7) >> 3) * rect.Width)); ;
	if (&GUID_PKPixelFormat12bppYUV420 == PIfrom.pGUIDPixFmt || &GUID_PKPixelFormat16bppYUV422 == PIfrom.pGUIDPixFmt) 
		cbStrideFrom >>= 1;

	U32 cbStrideTo =   (BD_1 == PIto.bdBitDepth ?   ((PIto.cbitUnit * rect.Width + 7) >> 3) : (((PIto.cbitUnit + 7) >> 3) * rect.Width)); 
	if (&GUID_PKPixelFormat12bppYUV420 == PIto.pGUIDPixFmt || &GUID_PKPixelFormat16bppYUV422 == PIto.pGUIDPixFmt) 
		cbStrideTo >>= 1;

	U32 cbStride = max(cbStrideFrom, cbStrideTo);

	out_size[0]=cbStride;
	out_size[1]=pDecoder->WMP.wmiI.cROIHeight;
	PKAllocAligned((void **) out, out_size[0]*out_size[1], 128);// todo do proper PKFree instead of isis-default-free

	pConverter->Copy(pConverter, &rect, *out, cbStride);

	pDecoder->Release(&pDecoder);
}


