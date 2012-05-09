#include "filter.hpp"

namespace isis
{
namespace filter
{
namespace _internal
{
FilterBase::FilterBase()
	: m_inputIsSet(false)
{}
}


bool hasOMPSupport()
{
#ifdef _OPENMP
	return true;
#else
	return false;
#endif
}

#ifdef _OPENMP
void setNumberOfOMPThreads ( const uint16_t& threads )
{
	omp_set_num_threads( threads );
}

void setUseAllAvailableThreadsOMP()
{
	omp_set_num_threads( omp_get_num_procs() );
}

uint16_t getNumberOfAvailableThreadsOMP() {
	return omp_get_num_procs();
}
#endif


}
}
