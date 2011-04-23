#include "typeptr.hpp"

namespace isis{
namespace data{
	// specialisation for complex - there shall be no scaling - and we cannot compute minmax
	template<> scaling_pair ValuePtr<std::complex<float> >::getScalingTo( unsigned short /*typeID*/, autoscaleOption /*scaleopt*/ )const{
		return scaling_pair(util::Value<float>( 1 ),util::Value<float>( 0 ) );
	}
	template<> scaling_pair ValuePtr<std::complex<double> >::getScalingTo( unsigned short /*typeID*/, autoscaleOption /*scaleopt*/ )const{
		return scaling_pair(util::Value<double>( 1 ),util::Value<double>( 0 ) );
	}
}
}

