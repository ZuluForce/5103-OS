##############################################
## Written by Andrew Helgeson
## Purpose:
##	Creates output graphs for tests run on the
##	5103 Virtual Memory Manager.
##
## Dependencies:
##	matplotlib
##	numpy
##	python v2.6 - 2.7
##############################################

import sys, os, re
import numpy as np
import matplotlib.pyplot as plt

from dataParse import *
from dataPlot import *


def getFiles(fileArray, directory):
	for dirname, dirnames, filenames in os.walk(directory):
		for filename in filenames:
			print("Found File: " + filename)
			fileArray.append( os.path.join(dirname, filename) )
			
		for subdirname in dirnames:
			getFiles(fileArray, os.path.join(dirname, subdirname))
			
	return

def fillFileInfo(filename, cs, pf, et):
	f = open(filename,"w")
	return

if __name__ == '__main__':
	if ( len(sys.argv) < 2 ):
		print("Usage: %s <result_dir>" % (sys.argv[0]))
		exit(-1)
	
	match = None ##For storing regex matches
		
	## ---- Check Desired Output Graphs ---- ##
	output_results = graphOutputs()
	output_results.loadOutputs(os.path.join(sys.argv[1],"out_graphs.conf"))
	
	
	files = []
	tests = []
	getFiles(files, sys.argv[1])
	print("Got filenames from dir: " + sys.argv[1])
	
	##Ignore file if it matches these
	ext_re = re.compile(".+[.]conf$")
	trace_re = re.compile("^.+\.trace")
	img_re = re.compile("^.+\.(png|jpg|jpeg|bmp)$")
	save_re = re.compile("^.+\.save$")
	
	for filename in files:
		if ( ext_re.match(filename) or trace_re.match(filename)):
			continue
			
		if ( img_re.match(filename) ):
			continue
			
		if ( save_re.match(filename) ):
			continue
		
		newTest = testRun()
		if ( not newTest.loadTest(filename) ):
			print("Error in loading test file: " + filename)
			exit(-1)
		
		newTest.setupMaps()
		tests.append(newTest)
		
	if ( len(sys.argv) > 2 ):
		print("Switching to directory: " + sys.argv[2])
		os.chdir(sys.argv[2])
	else:
		print("Switching to directory: " + sys.argv[1])
		os.chdir(sys.argv[1])

	##Create Results
	for config in output_results:
		createGraph(config,tests)
		
	print("Finished creating %d Graphs" % (len(output_results)))
	print("If any graphs did not show up, check for typos in out_graphs.conf")
			
	
