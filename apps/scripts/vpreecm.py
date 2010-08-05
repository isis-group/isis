#!/usr/bin/python
import os
import sys
from getopt import getopt, GetoptError
import string
from time import strftime
import logging


global MNI_BRAIN_FSL, dir_steps, converter
if (os.path.exists( "/usr/share/lipsia/mni_fsl.v" )):
	MNI_BRAIN_FSL = "/usr/share/lipsia/mni_fsl.v"
else:
	MNI_BRAIN_FSL = "not found"
dir_steps = "vpreecm_steps_" + strftime("%Y-%m-%d_%H:%M:%S")
log_file = "vpreecm_steps_" + strftime("%Y-%m-%d_%H:%M:%S") + "/log.txt"
	
	

def usage():
	print "\nvpreecm is a little script that performs the preprocessing chain for vecm."
	print "Usage:"
	print "Needed parameters:"
	print "(-i or --in):\n\tDenotes the input file which has to be preprocesed. Can be of file type dicom, vista or nifti."
	print "\nOptional parameters:"
	print "(--mni):\n\tDenotes the MNI brain used for the registration. This only has to be set if the user wishes to take an other standard brain."
	print "(-k or --keep):\n\tKeeps the files from all the steps in a folder named vpreecm_steps_{date}. If not set this folder will be deleted after the preprocessing."
	print "(--tr):\n\tDenotes the repetition time. The repetition time only has to be set if the programm requested it."
	print "(--slicetimefile):\n\t This is the file containing the acquisition time for each slice. This only has to pe set if the programm requested it."
	print "(--scriptuse):\n\tIf you want to use vpreecm in a script you can add this parameter to avoid interruption of the preprocessing chain if you get a important warning from vpreecm."
	print "\n"
	
	
def getattributevalue(filename, attribute):
	valuetuple = []
	os.system("less " + filename + " > " + dir_steps + "/attributecheck")
	fattr = open(dir_steps + "/attributecheck")
	lines = fattr.readlines()
	for i in lines:
		if (string.find(i, attribute + ":") != -1):
			valuetuple.append(i.split(":")[1])
	return valuetuple

	
def attributecheck(filename, attribute):
	attrcount = 0
	voxelcount = 0
	orientationcount = 0
	os.system("less " + filename +" > " + dir_steps + "/attributecheck")
	fattr = open(dir_steps + "/attributecheck")
	lines = fattr.readlines()
	for i in lines:
		if(string.find(i, attribute + ":") != -1):
			attrcount+=1
		elif(string.find(i, "voxel:") !=-1):
			voxelcount+=1
		elif(string.find(i, "orientation:") != -1):
			orientationcount+=1
	os.system("rm " + dir_steps + "/attributecheck")
	if(attrcount == orientationcount == voxelcount):
		return True
	else:
		return False


def preprocess(input, tr, slicetimefile, scriptuse):
	os.system("mkdir " + dir_steps)
	tmpFile = input
	#converting if not vista file
	if ( not (string.find(input, ".v") != -1) ):
		print "converting data to vista..."
		if( tr == -1):
			os.system("isisconv -in " + input + " -out " + dir_steps +  "/s1_converted.v")
		else:
			os.system("isisconv -in " + input + " -out " + dir_steps +  "/s1_converted.v -tr " + str(tr) )
			
		tmpFile = dir_steps +  "/s1_converted.v"
	
	if(not attributecheck(tmpFile, "repetition_time")):
		print "No repetition time found. You have to specifiy the repetition time in seconds by using the parameter --tr !"
		sys.exit(2)
	
	#check slicetime
	if( int(getattributevalue(tmpFile, "repetition_time")[0]) < 4000 ):			
		if ( not len(slicetimefile)):
			if (not attributecheck(tmpFile, "slice_time" ) ):
				print "Neither a slicetime file was specified nor a valid slicetime information was found in the image. Omitting slicetimecorrection!"
			else:
				print "performing slice time correction"
				os.system("vslicetime -in " + tmpFile + " -out " + dir_steps +  "/s2_slicetimecorrection.v")
		else:
			os.system("vslicetime -in " + tmpFile + " -out " + dir_steps +  "/s2_slicetimecorrection.v -slicetime " + slicetimefile)
		tmpFile = dir_steps + "/s2_slicetimecorrection.v"
	else:
		print "The repitition time (" + getattributevalue(tmpFile, "repetition_time")[0][:-1] + " ms) is too long for reliable slice time correction. Omitting slice time correction."
		print "Keep in mind that your results may not be reliable as well!"
		if (not scriptuse):
			raw_input("Press any key to continue anyway...")
		
		
		
	#moco
	print "performing motion correction"
	os.system("vmovcorrection -in " + tmpFile + " -out " + dir_steps +  "/s3_motioncorrection.v" )
	tmpFile = dir_steps +  "/s3_motioncorrection.v"
	
	#registration
	print "performing registration into MNI space..."
	os.system("valign3d -ref " + MNI_BRAIN_FSL + " -in " + tmpFile + " -itktrans " + dir_steps + "/doecm.tra -prealign_m true -iter 50")
	os.system("vdotrans3d -ref " + MNI_BRAIN_FSL + " -in " + tmpFile + " -itktrans " + dir_steps + "/doecm.tra -out " + dir_steps +  "/s4_registration.v -fmri -reso 3 ")
	print "creating image to check registration..."
	os.system("vdotrans3d -ref " + MNI_BRAIN_FSL + " -in " + tmpFile + " -itktrans " + dir_steps + "/doecm.tra -out " + dir_steps + "/doecm_check_registration.v")
	tmpFile = dir_steps + "/s4_registration.v"
	
	#vpreprocess
	print "removing baseline drifts..."
	os.system("vpreprocess -in " + tmpFile + " -fwhm 6 -low 0 -high 90 -out " + dir_steps + "/s5_baselinedrift.v")
	tmpFile = dir_steps + "/s5_baselinedrift.v"
	os.system("cp " + tmpFile + " vecm_" + input)
	
	

def removeSteps():
	print "Removing non-usable files..."
	os.system("rm " + dir_steps + " -fr")


def main(argv):
	global MNI_BRAIN_FSL
	input = ""
	slicetimefile = ""
	tr = -1
	convert = False
	keep = False
	scriptuse = False
	try:
		opts, args = getopt(argv, "i:hks:", ["scriptuse", "help", "input=", "keep", "tr=", "mni=", "slicetimefile="])
	except GetoptError:
		usage()
		sys.exit(2)

	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()					 
			sys.exit()	
		if opt in ("-i", "--input"):
			input = arg	
		if opt in ("-k", "--keep"):
			keep = True			
		if opt in ("--tr"):
			tr = arg
		if opt in ("--mni"):
			MNI_BRAIN_FSL = arg
		if opt in ("--slicetimefile", "-s"):
			slicetimefile = arg
		if opt in ("--scriptuse"):
			scriptuse = True	
	residuals = "".join(args)
	if( len(residuals) ):
		print "Omitting: " + residuals
		
	if( MNI_BRAIN_FSL == "not found"):
		print "Could not find the MNI brain. You have to specifiy it by using the parameter --mni."
		sys.exit(2)
	if(len(input)):
		preprocess(input, tr, slicetimefile, scriptuse)
	else:
		print "You have to specify an input image file. Parameter is -i or --in."
		sys.exit(2)
	if(not keep):
		removeSteps()
	
	print "Done."
	


if __name__ == "__main__":
    main(sys.argv[1:])