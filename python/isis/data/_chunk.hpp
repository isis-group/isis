/*
 * _chunk.hpp
 *
 *  Created on: Oct 21, 2010
 *      Author: tuerke
 */

#ifndef CHUNK_HPP_
#define CHUNK_HPP_

#include "DataStorage/chunk.hpp"

namespace isis
{
namespace python
{
class _Chunk : public isis::data::Chunk, boost::python::wrapper<isis::data::Chunk>
{
public:
	//  _Chunk ( PyObject *p ) : self( p ) {}
	_Chunk ( PyObject *p, const isis::data::Chunk &base ) : isis::data::Chunk( base ), self( p ) {}

private:
	PyObject *self;
};
}
}
#endif /* CHUNK_HPP_ */
