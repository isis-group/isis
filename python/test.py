#! /usr/bin/python

import isis.core
import isis.data
import sys


ioapp = isis.data.IOApplication("IOApp", True, True)
print "Version " + ioapp.getCoreVersion()
ioapp.addParameter("hallo", isis.core.fvector4(3,2,1,4), "fvector4")
ioapp.init(len(sys.argv), sys.argv, True)
iList = isis.data.ImageList()
iList = ioapp.images()
image = isis.data.Image()
image = iList[0]

newfvector4 = isis.core.fvector4(4,3,2,21)

size = isis.core.ivector4(image.sizeToVector())
print size[0]
print size[1]
print size[2]
print size[3]

#print image.voxel(1,22,32,0)



#print image.voxel(1,22,32,0)

#newList = isis.core.ImageList()
#newList.append(image)

#ioapp.autowrite(newList, True)
