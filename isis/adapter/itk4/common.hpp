#ifndef ITK_COMMON_HPP
#define ITK_COMMON_HPP

#include "../../core/log.hpp"
#include "../../core/image.hpp"


namespace isis{
	struct ITKLog {static const char *name() {return "ITK";}; enum {use = _ENABLE_LOG};};
	struct ITKDebug {static const char *name() {return "ITKDebug";}; enum {use = _ENABLE_DEBUG};};

namespace itk4{

	typedef ITKDebug Debug;
	typedef ITKLog Runtime;

	template<typename HANDLE> void enableLog( LogLevel level )
	{
		ENABLE_LOG( ITKLog, HANDLE, level );
		ENABLE_LOG( ITKDebug, HANDLE, level );
	}
	
	data::Image resample(data::Image source,util::vector4<size_t> newsize);
	data::Image rotate(data::Image src, std::pair<int,int> rotation_plain, float angle, bool pix_center=false);
	data::Image translate(data::Image src, util::fvector3 translation);
}
}

#endif //ITK_COMMON_HPP
