/*
 * isisReg3DApplication.h
 *
 *  Created on: Feb 11, 2009
 *      Author: tuerke
 */

#ifndef ISISREG3DAPPLICATION_H_
#define ISISREG3DAPPLICATION_H_

#include "isisApplication.h"

namespace isis {

class Reg3DApplication : public Application {
public:
	typedef Reg3DApplication Self;
	typedef Application Superclass;

protected:
	Reg3DApplication();
	virtual ~Reg3DApplication();

};

}

#endif /* ISISREG3DAPPLICATION_H_ */
