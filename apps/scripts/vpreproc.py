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
dir_steps = "vpreproc_steps_" + strftime("%Y-%m-%d_%H:%M:%S")
log_file = "vpreproc_steps_" + strftime("%Y-%m-%d_%H:%M:%S") + "/log.txt"
	
	

def usage():
	print "\nvpreproc is a little script that performs the preprocessing chain for vecm."
	print "Usage:"
	print "Needed parameters:"
	print "(-i or --in):\n\tDenotes the input file which has to be preprocesed. Can be of file type dicom, vista or nifti."
	print "\nOptional parameters:"
	print "(--mni):\n\tDenotes the MNI brain used for the registration. This only has to be set if the user wishes to take an other standard brain."
	print "(-k or --keep):\n\tKeeps the files from all the steps in a folder named vpreproc_steps_{date}. If not set this folder will be deleted after the preprocessing."
	print "(--tr):\n\tDenotes the repetition time. The repetition time only has to be set if the programm requested it."
	print "(--slicetimefile):\n\t This is the file containing the acquisition time for each slice. This only has to pe set if the programm requested it."
	print "(--scriptuse):\n\tIf you want to use vpreproc in a script you can add this parameter to avoid interruption of the preprocessing chain if you get a important warning from vpreproc."
	print "\n"
	
	
def getattributevalue(filename, attribute):
	valuetuple = []
	os.system("less " + filename + " > " + dir_steps + "/attributecheck")
	fattr = open(dir_steps + "/attributecheck")
	lines = fattr.readlines()
	for i in lines:
		if (string.find(i, attribute + ":") != -1):
			valuetuple.append(i.split(":")[1])
	os.system("rm " + dir_steps + "/attributecheck")
	return valuetuple

	
def attributecheck(filename, attribute):
	attrcount = 0
	os.system("less " + filename +" > " + dir_steps + "/attributecheck")
	fattr = open(dir_steps + "/attributecheck")
	lines = fattr.readlines()
	for i in lines:
		if(string.find(i, attribute + ":") != -1):
			attrcount+=1
	os.system("rm " + dir_steps + "/attributecheck")
	if(attrcount):
		return True
	else:
		return False


def preprocess(input, tr, slicetimefile, scriptuse, output, fwhm, high):
	os.system("mkdir " + dir_steps)
	tmpFile = input
	#converting if not vista file
	if ( not (string.find(input, ".v") != -1) ):
		print "converting data to vista..."
		if( tr == -1):
			os.system("vvinidi -in " + input + " -out " + dir_steps +  "/s1_converted.v  > vpreproc.tmp 2> vpreproc.tmp" )
		else:
			os.system("vvinidi -in " + input + " -out " + dir_steps +  "/s1_converted.v  -rdialect functional -tr " + str(tr) + "> vpreproc.tmp 2> vpreproc.tmp" )
			
		tmpFile = dir_steps +  "/s1_converted.v"
	if(tr != -1):
		trstr = str(int(tr)*1000);
		os.system("vattredit -name repetition_time -value '" + trstr + "' -in " + tmpFile + " -out " + dir_steps + "/s1_converted.v 2> vpreproc.tmp")
		tmpFile = dir_steps +  "/s1_converted.v"
		
		
	if(not attributecheck(tmpFile, "repetition_time") and tr == -1):
		print "No repetition time found. You have to specifiy the repetition time in seconds by using the parameter --tr !"
		sys.exit(2)
	
	#check slicetime
	if( int(getattributevalue(tmpFile, "repetition_time")[0]) < 4000 and int(tr) < 4):			
		if ( not len(slicetimefile)):
			if (not attributecheck(tmpFile, "slice_time" ) ):
				print "!WARNING! Neither a slicetime file was specified nor a valid slicetime information was found in the image. Omitting slicetimecorrection!"
			else:
				print "performing slice time correction..."
				os.system("vslicetime -in " + tmpFile + " -out " + dir_steps +  "/s2_slicetimecorrection.v 2> vpreproc.tmp")
				tmpFile = dir_steps + "/s2_slicetimecorrection.v"
		else:
			print "performing slice time correction with slicetime file " + slicetimefile
			os.system("vslicetime -in " + tmpFile + " -out " + dir_steps +  "/s2_slicetimecorrection.v -slicetime " + slicetimefile + " 2> vpreproc.tmp")
			tmpFile = dir_steps + "/s2_slicetimecorrection.v"
		
	else:
		print "The repitition time (" + getattributevalue(tmpFile, "repetition_time")[0][:-1] + " ms) is too long for reliable slice time correction. Omitting slice time correction."
		print "Keep in mind that your results may not be reliable as well!"
		if (not scriptuse):
			raw_input("Press any key to continue anyway...")
		
	
	
	#moco
	print "performing motion correction..."
	os.system("vmovcorrection -in " + tmpFile + " -out " + dir_steps +  "/s3_motioncorrection.v 2> vpreproc.tmp" )
	tmpFile = dir_steps +  "/s3_motioncorrection.v"
	
	#registration
	print "performing registration into MNI space..."
	os.system("valign3d -ref " + MNI_BRAIN_FSL + " -in " + tmpFile + " -itktrans " + dir_steps + "/vpreproc.tra -prealign_m true -iter 50 > vpreproc.tmp 2> vpreproc.tmp")
	os.system("vdotrans3d -ref " + MNI_BRAIN_FSL + " -in " + tmpFile + " -itktrans " + dir_steps + "/vpreproc.tra -out " + dir_steps +  "/s4_registration.v -fmri -reso 3 > vpreproc.tmp 2> vpreproc.tmp")
	print "creating image to check registration..."
	os.system("vdotrans3d -ref " + MNI_BRAIN_FSL + " -in " + tmpFile + " -itktrans " + dir_steps + "/vpreproc.tra -out " + dir_steps + "/vpreproc_check_registration.v > vpreproc.tmp 2> vpreproc.tmp")
	tmpFile = dir_steps + "/s4_registration.v"
	
	#vpreprocess
	print "removing baseline drifts below 1/" + str(high) + " Hz. Applying spatial Gaussian filter of width " + str(fwhm) + " mm."
	os.system("vpreprocess -in " + tmpFile + " -fwhm " + str(fwhm) + " -low 0 -high " + str(high) + " -out " + dir_steps + "/s5_baselinedrift.v 2> vpreproc.tmp")
	tmpFile = dir_steps + "/s5_baselinedrift.v"
	os.system("cp " + tmpFile + " " + output)
	
	

def removeSteps():
	print "Removing non-usable files..."
	os.system("rm " + dir_steps + " -fr")
	


def main(argv):
	global MNI_BRAIN_FSL
	input = ""
	output = ""
	slicetimefile = ""
	fwhm = -1
	high = -1
	tr = -1
	convert = False
	keep = False
	scriptuse = False
	try:
		opts, args = getopt(argv, "o:i:hks:", ["scriptuse", "help", "in=", "keep", "tr=", "mni=", "slicetimefile=", "fwhm=", "high=", "out="])
	except GetoptError:
		usage()
		sys.exit(2)

	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()					 
			sys.exit()	
		if opt in ("-i", "--in"):
			input = arg
		if opt in ("-o", "--out"):
			output = arg	
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
		if opt in ("--fwhm"):
			fwhm = arg
		if opt in ("--high"):
			high = arg
	residuals = "".join(args)
	if( len(residuals) ):
		print "Omitting: " + residuals
		
	if( fwhm == -1 ):
		fwhm = 6
	if( high == -1 ):
		high = 90
	if (not len(output)):
		print "You have to specify an output image filename. Please use the parameter -o or --out."
		sys.exit(2)
	if( MNI_BRAIN_FSL == "not found"):
		print "Could not find the MNI brain. You have to specifiy it by using the parameter --mni."
		sys.exit(2)
	if(len(input)):
		preprocess(input, tr, slicetimefile, scriptuse, output, fwhm, high)
	else:
		print "You have to specify an input image file. Please use the parameter -i or --in."
		sys.exit(2)
	if(not keep):
		removeSteps()
	os.system("rm vpreproc.tmp")
	print "Done."
	


if __name__ == "__main__":
    main(sys.argv[1:])