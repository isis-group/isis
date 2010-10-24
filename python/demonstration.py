#! /usr/bin/python

#this is a little script which demonstrates the currently implemented python isis-functions

#first we have to import the isis core module
import isis.core
#the same for the isis data module
import isis.data
#this module is needed to get the commandline parameters
import sys

#here we create an object of type IOApplication with the name "IOApp" and needed input and output 
ioapp = isis.data.IOApplication("IOApp", True, True)
#prints the svn revision and the version of the currently used isis package
print "Version " + ioapp.getCoreVersion()
#here we add an additional parameter "hallo" of type fvector4 to ioapp and initialize it with (3,2,1,4)
ioapp.addParameter("ivector4Param", isis.core.ivector4(2,4,5,2), "ivector4")
ioapp.addParameter("SelectionParam", isis.core.Selection("gna,ugga,asdasd"), "selEction")
#this should work as well

#set it hidden 
ioapp.setHidden("ivector4Param", True)
#and not required
ioapp.setNeeded("ivector4Param", False)
#standard initialization of our ioapp. First parameter is the amount of parameters, the second parameter is a tuple of all parameters and the last one indicates if the programm should exit if an error occurs
ioapp.init(len(sys.argv), sys.argv, True)
#ioapp.printHelp(True)
iList = isis.data.ImageList(ioapp.images())

print "creating image"
image = isis.data.Image(iList[0])

print "creating deep copy"
deepImageCopy = isis.data.Image( image.deepCopy() )
print "creating cheap copy"
cheapImageCopy = isis.data.Image( image.cheapCopy() )
#print "manipulating deepImage"
#deepImageCopy.setVoxel(1,2,3,0,100)

print "diff size cheap<->deep: " + str(deepImageCopy.cmp(cheapImageCopy))
deepImageCopy.setVoxel(2,2,2,0, -deepImageCopy.getVoxel(2,2,2,0) )
print "diff size orig<->deep: " + str(deepImageCopy.cmp(image))

print "manipulating cheapImage"
image.setVoxel(1,2,3,0, -image.getVoxel(1,2,3,0))


image.setProperty( "indexOrigin", isis.core.fvector4(1,2,2,2), "fvector4" )
print image.getPropMap().propertyValue("indexOrigin").toString(True)

#ioapp.autowrite(outList, True)
