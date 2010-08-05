#!/usr/bin/python
import os
import sys
import glob
from getopt import getopt, GetoptError
import string

def usage():
    print "\nvcreatemask is a little script that creates a mask from a list of functional vista files stored in a folder. "
    print "Usage:"
    print "Needed parameters:"
    print "(-i or --in):\n\tDenotes the input folder which containes the functional vista files."
    print "\nOptional parameters:"
    print "(-o or --output):\n\tDenotes the name of the mask which will be created."
    print "(--maxvoxel:) \n\tDenotes the maximal amount of voxel the mask can contain. The script will automatically shrink the mask until this amount of voxels is reached. Default value is 50000"
    
    
def end(inputfolder):
    os.system("rm " + inputfolder + "vcreatemask_tmp -rf" )
    os.system("rm volumeinfo.tmp vcreatemask.tmp")


def docreatemask(inputfolder, maxvoxel, outfile):
    vistafiles = []
    vistafilesstring = ""
    if (not inputfolder[len(inputfolder)-1] == "/"):
        inputfolder+="/"
    for infiles in glob.glob(os.path.join(inputfolder, "*.v") ):
        vistafiles.append(infiles)
    if (not len(vistafiles)):
        print "No vista files were found in " + inputfolder + "!"
        sys.exit(2)
    else:
        print "Collected " + str(len(vistafiles)) + " vista files."
    os.system("mkdir " + inputfolder + "vcreatemask_tmp")
    counter = 0
    print "Processing files. This may take some time..."
    for i in vistafiles:
        os.system("vtimestep -in " + i + " -out " + inputfolder + "vcreatemask_tmp/" + str(counter) + ".v 2> vcreatemask.tmp")
        #check for functional data
        f = open("vcreatemask.tmp")
        lines = f.readlines()
        for line in lines:
            if (string.find(line, "vtimestep: Fatal:") != -1):
                os.system("rm " + inputfolder + "vcreatemask_tmp/" + str(counter) + ".v")
        counter+=1
    counter = 0
    for infiles in glob.glob(os.path.join(inputfolder + "vcreatemask_tmp/", "*.v")):
        vistafilesstring+=infiles + " "
        counter += 1
    print str(counter) + " images declared as valid. Performing averaging..."
    os.system("vave -in " + vistafilesstring +  " -out " + inputfolder + "vcreatemask_tmp/average.v 2> vcreatemask.tmp")
    print "Creating mask with a maximal volume of " + str(maxvoxel) + " voxel"
    voxelcount = maxvoxel+1
    threshold = 80
    while( voxelcount > maxvoxel):    
        os.system("vbinarize -in " + inputfolder + "vcreatemask_tmp/average.v -out " + inputfolder + "vcreatemask_tmp/binarized.v -min " + str(threshold) + "  2> vcreatemask.tmp" )
        os.system("vsmooth3d -in " + inputfolder + "vcreatemask_tmp/binarized.v -out "  + inputfolder + "vcreatemask_tmp/smoothed.v -iter 1000  2> vcreatemask.tmp");
        os.system("vconvert -in " + inputfolder + "vcreatemask_tmp/smoothed.v -out " + inputfolder + "vcreatemask_tmp/smooth_float.v -repn float 2> vcreatemask.tmp")
        os.system("volumeinfo -in " + inputfolder + "vcreatemask_tmp/smoothed.v 2> volumeinfo.tmp")
        volumefile = open("volumeinfo.tmp")
        lines = volumefile.readlines()
        for line in lines:
            if(string.find(line, "0:") != -1):
                voxelcount =  float((line.split(":")[1].split(",")[1]).rstrip("\n")) 
        threshold+=1
        if(threshold == 255):
            print "Error: Creating of mask failed!"
            end(inputfolder)
            sys.exit(2)
    print "A window will open. There you can check the mask. To view the basic image data you have to use the left vertical slider."
    os.system("vlv -in " + inputfolder + "vcreatemask_tmp/average.v -z " + inputfolder + "vcreatemask_tmp/smooth_float.v" )
    os.system("cp " + inputfolder + "vcreatemask_tmp/smoothed.v " + outfile)
    print "A mask named " + outfile + " was created. To edit this mask you can use vledit."
    raw_input("Press any key to exit.")
    end(inputfolder)
    print "Done."
    


def main(argv):
    inputfolder = ""
    outfile = "vcreated_mask.v"
    maxvoxel = 50000
    try:
        opts, args = getopt(argv, "hi:o:", ["help", "in=", "maxvoxel=", "out="])
    except GetoptError:
        usage()
        sys.exit(2)

    for opt, arg in opts:
        if opt in ("-h", "--help"):
            usage()                     
            sys.exit()
        if opt in ("-i", "--in"):
            inputfolder = arg
        if opt in ("--maxvoxel"):
            maxvoxel = int(arg)
        if opt in ("o", "--out"):
            outfile = arg
    
    if(not len(inputfolder)):
        print "You have to specify an input folder containing all your preprocessed vista files!"
        sys.exit(2)
    docreatemask(inputfolder, maxvoxel, outfile)




if __name__ == "__main__":
    main(sys.argv[1:])