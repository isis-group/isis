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

image = isis.data.Image(iList[0])

print "MainOrientation: ", image.getMainOrientation()

print "bytes per voxel: ", image.bytesPerVoxel()
print "typeID: ", image.typeID()
print "min: ", image.getMin()
print "max: ", image.getMax()
chunkAt = isis.data.Chunk(image.getChunkAt(0, True))
chunkAt = image.getChunk(0,0,0,0,True)
chunkAt = image.getChunk(isis.core.ivector4(0,0,0,0), True)

myChunkList = isis.data.ChunkList(image.getChunkList() )
myChunk = isis.data.Chunk( myChunkList[0] )

#image.insertChunk(chunkAt)
print "image contains ", len(myChunkList), " chunks"

mat = [[0,-1,0],[1,0,0],[0,0,-1]]

image.transformCoords( mat )
newfvector4 = isis.core.fvector4(4,3,2,21)

image2 = isis.data.Image(image.deepCopy())

size = isis.core.ivector4(image.sizeToVector())
for i in range(size[0]):
	for j in range(size[1]):
		coords = isis.core.ivector4(i,j,0,0)
		image.setVoxel(coords, ((i) / (j+1)) * image.voxel(coords) )
#print "diff (should be 0): ", image.cmp(image2) #should be 0 because of cheap copy
print "diff: " + str(image.cmp(image2))
image.makeOfTypeName("float")
image.spliceDownTo("sliceDim")
print "Now the image has " + str(len(image.getChunkList())) + " chunks"

iList.append(image)
outList = isis.data.ImageList()
outList.append(image)
ioapp.autowrite(outList, True)
