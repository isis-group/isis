#!/usr/bin/python
import os
import sys
from getopt import getopt, GetoptError
import string
from time import strftime
import logging


global MNI_BRAIN_FSL, dir_steps
if (os.path.exists( "/usr/share/lipsia/mni_fsl.v" )):
	MNI_BRAIN_FSL = "/usr/share/lipsia/mni_fsl.v"
else:
	MNI_BRAIN_FSL = "not found"
dir_steps = "vdoecm_steps_" + strftime("%Y-%m-%d_%H:%M:%S")
log_file = "vdoecm_steps_" + strftime("%Y-%m-%d_%H:%M:%S") + "/log.txt"

	
	

def usage():
	print "Usage"


def preprocess(input, tr):
	os.system("mkdir " + dir_steps)
	tmpFile = input
	#converting if not vista file
	if ( not (string.find(input, ".v") != -1) ):
		if ( tr == -1 ):
			print "You have to specify a repetition time by using the parameter --tr !"
			sys.exit(2)
		print "converting data to vista..."
		if( tr == -1):
			os.system("vvinidi -in " + input + " -out " + dir_steps +  "/s1_converted.v")
		else:
			os.system("vvinidi -in " + input + " -out " + dir_steps +  "/s1_converted.v -tr " + str(tr) )
			
		tmpFile = dir_steps +  "/s1_converted.v"
	#slicetime
	print "performing slice time correction"
	os.system("vslicetime -in " + tmpFile + " -out " + dir_steps +  "/s2_slicetimecorrection.v")
	tmpFile = dir_steps + "/s2_slicetimecorrection.v"
	
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
	tr = -1
	convert = False
	keep = False
	try:
		opts, args = getopt(argv, "i:hk", ["help", "input=", "keep", "tr=", "mni="])
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
	residuals = "".join(args)
	if( len(residuals) ):
		print "Omitting: " + residuals
		
	if( MNI_BRAIN_FSL == "not found"):
		print "Could not find the MNI brain. You have to specifiy it by using the parameter --mni."
		sys.exit(2)
	if(len(input)):
		preprocess(input, tr)
	else:
		print "You have to specify an input image file. Parameter is -i or --in."
		sys.exit(2)
	if(not keep):
		removeSteps()
	
	print "Done."
	


if __name__ == "__main__":
    main(sys.argv[1:])