#ifndef QT5_COMMON_HPP
#define QT5_COMMON_HPP

#include <QImage>
#include "../../data/image.hpp"

namespace isis{
	struct Qt5Log {static const char *name() {return "Qt5";}; enum {use = _ENABLE_LOG};};
	struct Qt5Debug {static const char *name() {return "Qt5Debug";}; enum {use = _ENABLE_DEBUG};};

namespace qt5{

	typedef Qt5Debug Debug;
	typedef Qt5Log Runtime;

	template<typename HANDLE> void enableLog( LogLevel level )
	{
		ENABLE_LOG( Qt5Log, HANDLE, level );
		ENABLE_LOG( Qt5Debug, HANDLE, level );
	}

	QImage makeQImage(const data::ValueArrayBase &data,size_t line_length,data::scaling_pair scaling = data::scaling_pair() );
	
	/**
	 * Display an Image using Qt5.
	 * This function will create an show a SimpleImageView to display the given Image.
	 * If if a QApplication is there already (e.g. IOQtApplication::init was called) it uses its event loop. In that case display it is not blocking.
	 * \Note The QApplication has just to be there, IOQtApplication::getQApplication().exec() can be called later.
	 * If no QApplication is available display will create (an run) its own. In that case display it is blocking.
	 */
	void display(data::Image img, QString title="");
}
}

#endif //QT5_COMMON_HPP
