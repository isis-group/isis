#!/usr/bin/python
import sys
import os
from getopt import getopt, GetoptError
import string

__PACKAGE__="lipsia-sandbox"


def usage():
    print "\n"
    print "Usage:"
    print "This is a script which performs registration on the MNI-brain."
    print "Parameters:"
    print "-i or --in: denotes the image which should be aligned onto the MNI-brain. "
    print "-m or --mni: denotes the MNI-brain. If you have installed the " + __PACKAGE__ + "-package you can omit this parameter."
    print "-o or --out: denotes the registered output image."
    print "-f or --fmri: saves your output file as a functional image. If you just want to check your registration result you can omit this flag."
    print "-p or --peeled: if your input image is peeled (without skull) you should set this flag."
    print "-s or --scaling: performs a scaling of the input image. This is only necessary if the physical size (not to be confused with the image size) of the brain differentiates from the size of the MNI-brain.\n"
    sys.exit()

def registration():
    print "Starting registration of " + INPUT_IMAGE + " on " + MNI_BRAIN + "..."
    if ( scaling ):
        os.system("valign3d -ref " + MNI_BRAIN + " -in " + INPUT_IMAGE + " -prealign_m  -transform 0 4 -optimizer 0 1 -iter 10 0 -trans tmp.nii > logreg.txt")
    else:
        os.system("valign3d -ref " + MNI_BRAIN + " -in " + INPUT_IMAGE + " -prealign_m -v false -iter 10 -trans tmp.nii > logreg.txt")
    print "Resampling..."
    if (fmri):
        if (len(iso)):       
            os.system("vdotrans3d -ref " + MNI_BRAIN + " -in " + INPUT_IMAGE + " -out " + OUT_IMAGE + " -trans tmp.nii -fmri -reso " + iso + " > logres.txt")
        else:
            os.system("vdotrans3d -ref " + MNI_BRAIN + " -in " + INPUT_IMAGE + " -out " + OUT_IMAGE + " -trans tmp.nii -fmri -keep_reso > logres.txt")
    else:
        if(len(iso)):
            os.system("vdotrans3d -ref " + MNI_BRAIN + " -in " + INPUT_IMAGE + " -out " + OUT_IMAGE + " -trans tmp.nii -reso " + iso + "> logres.txt")
        else:
            os.system("vdotrans3d -ref " + MNI_BRAIN + " -in " + INPUT_IMAGE + " -out " + OUT_IMAGE + " -trans tmp.nii > logres.txt")
            
            
    os.system("rm tmp.nii")
    
def main(argv):
    global MNI_BRAIN, INPUT_IMAGE, fmri, OUT_IMAGE, scaling, peeled, iso
    fmri = False
    scaling = False
    peeled = False
    MNI_BRAIN = ""
    INPUT_IMAGE = ""
    OUT_IMAGE = ""
    iso = ""
    try:     
        opts, args = getopt(argv, "po:i:m:hfs", ["peeled", "scaling", "input=", "help=", "mni=", "fmri", "out=", "iso="])
    except GetoptError:
        usage()
        sys.exit(2)
    
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            usage()                     
            sys.exit()    
        elif opt in ("-m", "--mni"):
            MNI_BRAIN = arg
        elif opt in ("-i", "--input"):
            INPUT_IMAGE = arg
        elif opt in ("-f", "--fmri"):
            fmri = True
        elif opt in ("-s", "--scaling"):
            scaling = True
        elif opt in ("-p", "--peeled"):
            peeled = True
        elif opt in ("-o", "--out"):
            OUT_IMAGE = arg
        elif opt in ("--iso"):
            iso = arg
    residuals = "".join(args)
    if( len(residuals) ):
        print "Omitting: " + residuals  
    if( peeled and not len(MNI_BRAIN) ):
        MNI_BRAIN = os.environ.get('ISIS_MNI_PEELED')
    elif( not peeled and not len(MNI_BRAIN)):
        MNI_BRAIN = os.environ.get('ISIS_MNI_SKULL')
    
    if ( not len(MNI_BRAIN) or not len(INPUT_IMAGE) or not len(OUT_IMAGE)):
        print "Either mni atlas or input image or output image not specified!"
        usage()
        
    #performing the registration
    registration()
        
            

if __name__ == "__main__":
    main(sys.argv[1:])
