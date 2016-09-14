#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "../../data/image.hpp"

namespace isis{
namespace qt5{
	/**
	 * Display an Image using Qt5.
	 * This function will create an show a SimpleImageView to display the given Image.
	 * If if a QApplication is there already (e.g. IOQtApplication::init was called) it uses its event loop. In that case display it is not blocking.
	 * \Note The QApplication has just to be there, IOQtApplication::getQApplication().exec() can be called later.
	 * If no QApplication is available display will create (an run) its own. In that case display it is blocking.
	 */
	void display(data::Image img, std::string title="");
}
}

#endif //DISPLAY_HPP
