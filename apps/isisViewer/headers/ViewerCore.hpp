#ifndef VIEWERCORE_HPP
#define VIEWERCORE_HPP

#include "ViewerCoreBase.hpp"

namespace isis
{
namespace viewer
{

class ViewerCore : public ViewerCoreBase
{
public:
	ViewerCore();
	
// 	template <typename WDIGET_TYPE>
// 	bool initDisplays()
// 	{
// 		if( getWidgets().empty() )
// 		{
// 			LOG(Runtime, error) << "There are no widgets registered. You have to call registerWidget prior calling initDisplays!";
// 			return false;
// 		}
// 		WidgetMap tmpMap = getWidgets();
// 		BOOST_FOREACH( WidgetMap::reference widgetRef, tmpMap )
// 		{
// 			getWidgetAs<WDIGET_TYPE>(widgetRef.first)->initWidget();
// 		}
// 		return true;
// 	}


private:

};


}
} // end namespace






#endif
