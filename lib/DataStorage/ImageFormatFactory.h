/**  A one line description of the class.  
 *
 * #include "ImageFormatFactory.h" <BR>
 * -llib 
 *
 * A longer description.
 *  
 * @see something
 * 
 */
/*
 * ImageFormatFactory.h
 *
 *  Created on: Aug 11, 2009
 *      Author: hellrung
 */

#ifndef IMAGEFORMATFACTORY_H_
#define IMAGEFORMATFACTORY_H_

// SYSTEM INCLUDES
//
#include <iostream>
#include <string>
#include <map>

// PROJECT INCLUDES
//
#include "ImageFormat.h"
// LOCAL INCLUDES
//

// FORWARD REFERENCES
//


class ImageFormatFactory
{
public:
	typedef ImageFormat* (*CreateImageFormatCallback)();

private:
	typedef std::map<std::string, CreateImageFormatCallback> CreateFnMap;

public:
	// LIFECYCLE

	/*Singleton
	 * */
	static ImageFormatFactory& GetInstance() {

		static ImageFormatFactory m_Instance;

		return m_Instance;
		//TODO: boost::singleton for dynamic libraries Boost.Utility/Singleton 1.0
	}

	/** Destructor.
	 */
	virtual ~ImageFormatFactory(
	    void);

	// OPERATORS

	//all operators private

	// OPERATIONS
	/*
	 * each ImageFormat has to register itself by this function
	 * @param strFormatId the unique id of the registering image format
	 * @param fnCreateFn the callback function that creates this format
	 *
	 * @return true if registration was successful, false otherwise
	 * */
	bool RegisterImageFormat(
	    const std::string& strFormatId,  CreateImageFormatCallback fnCreateFn);

	/*
	 * each ImageFormat can be unregister itself by this function
	 * @param strFormatId the unique id of the unregistering image format
	 *
	 * @return true if the format was registered before
	 * */
	bool UnregisterImageFormat(
	    const std::string& strFormatId);

	/*
	 *factory method to create ImageFormats
	 *@params strFormatId the unique id of the image format
	 *
	 *@return reference to the abstract ImageFormat
	 * */
	ImageFormat* CreateImageFormat(
	    const std::string& strFormatId);

	// ACCESS
	/*
	 *
	 * */

	// INQUIRY

protected:
private:

	/** Default constructor.
	 */

	ImageFormatFactory(
	    void);

	/** Copy constructor.
	 *
	 * @param from The value to copy to this object.
	 */
	ImageFormatFactory(
	    const ImageFormatFactory& from);

	/** Assignment operator.
	 *
	 * @param from The value to assign to this object.
	 *
	 * @return A reference to this object.
	 */
	ImageFormatFactory& operator=(
	    const ImageFormatFactory& from);

	bool LoadFromPluginPath();

	/*
	 * the map holding all the key value pairs of the registered image formats
	 * */
	CreateFnMap m_mapCreateFunctions;

	/*
	 * Singleton Instance
	 * */
	static ImageFormatFactory m_Instance;

};

// INLINE METHODS
//

// EXTERNAL REFERENCES
//

#endif /* IMAGEFORMATFACTORY_H_ */

