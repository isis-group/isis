//
// C++ Interface: isisodinimageio
//
// Description:
//
//
// Author: Thomas Proeger <proeger@cbs.mpg.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef is_isisOdinImageIO_h
#define is_isisOdinImageIO_h

#include <itkImageIOBase.h>

namespace isis {

/**
 * \class isisOdinImageIO
 * \brief An ImageIO derivative which provides access to the FileIO framework of the odin library.
 *
 * @author Thomas Proeger <proeger@cbs.mpg.de>
*/
class isisOdinImageIO : public ImageIOBase {
public:
    
    isisOdinImageIO();

    virtual bool CanReadFile(const char*);
    virtual bool CanWriteFile(const char*);

    virtual void ReadImageInformation(void);
    virtual void Read(void* buffer);

    virtual void WriteImageInformation(void);
    virtual void Write(void* buffer);

    ~isisOdinImageIO();

};

}

#endif
