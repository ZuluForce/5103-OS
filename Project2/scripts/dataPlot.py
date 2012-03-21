from dataParse import *
import numpy as np
import matplotlib.pyplot as plt

try:
	import axisLabels
except:
	pass

## The config an entry from the .conf file which is parsed by the
## graphOutputs class in dataParse.py
##
## Only pass a single entry from the .conf to this function in order
## to create the desired graph
##
## The tests parameter is a list of testRun classes. Each one defines a
## specific run of the VMM and its corresponding parameters and results


##Just to make the output cleaner
prettyConfig = {'sps':"Page Size",\
				'sfc':"Total Frame Count",\
				'sprt':"Page Replacement Module",\
				'gcs':"Global context switches",\
				'gpf':"Global page faults",\
				'get':"Global exec time",\
				'lcs':"Avg process context switches",\
				'lpf':"Avg process page faults",\
				'let':"Avg process exec time",\
				'lcsp':"Per-Process context switches",\
				'lpfp':"Per-Process page faults",\
				'letp':"Per-Process exec time"}

## Simple merge-sort with the addition of keeping track
## of how the items are reordered which is necessary if
## you are going to make accurate graphs
def sort(lst, newOrder, cmpfn, start = 0, end = -1):
	if ( end == -1 ):
		end = len(lst) - 1
	
	if ( end - start ) <= 0:
		return
		
	if (end - start) == 1:
		if cmpfn(int(lst[start]),int(lst[end])):
			##Swap values
			lst[start],lst[end] = lst[end],lst[start]
			newOrder[start],newOrder[end] = newOrder[end],newOrder[start]
		
		return
	
	mid = (start + end) / 2
	sort(lst, newOrder, cmpfn, start, mid)
	sort(lst, newOrder, cmpfn, mid+1, end)
	
	left = lst[start:mid+1]
	right = lst[mid+1:end + 1]
	
	lindex = rindex = 0
	_newOrder = list(newOrder) ##Copies the list
	
	while lindex < len(left) and rindex < len(right):
		print(_newOrder)
	
		##Take from the right
		if cmpfn(int(left[lindex]),int(right[rindex])):
			lst[start + lindex + rindex] = right[rindex]
			_newOrder[start + lindex + rindex] = newOrder[mid + 1 + rindex]
			rindex += 1
			
		else:
			##Take from the left side
			lst[start + lindex + rindex] = left[lindex]
			_newOrder[start + lindex + rindex] = newOrder[start + lindex]
			lindex += 1
	
	print(_newOrder)
	##Fill in any remaining
	while lindex < len(left):
		lst[start + lindex + rindex] = left[lindex]
		print("start + lindex = " + str(start + lindex))
		print("newOrder[start + lindex] = " + str(newOrder[start + lindex]))
		_newOrder[start + lindex + rindex] = newOrder[start + lindex]
		lindex += 1

	while rindex < len(right):
		lst[start + lindex + rindex] = right[rindex]
		_newOrder[start + lindex + rindex] = newOrder[mid + 1 + rindex]
		rindex += 1
		
	newOrder = _newOrder
	return
		
def createGraph(config, tests):
	x_axis	= config[configMap['x-axis']]
	order	= config[configMap['order']]
	y_axis	= config[configMap['y-axis']]
	output	= config[configMap['output']]
	gtype	= config[configMap['type']]

	print("Creating Graph:")
	print("\tx-axis: " + prettyConfig[x_axis])
	if ( order == 'i' ):
		print("\t\t****Increasing Order****")
	else:
		print("\t\t****Decreasing Order****")

	print("\ty-axis: " + prettyConfig[y_axis])
	print("\tOutput Graph: " + output)
	
	if ( gtype ):
		print("\tFile Type: " + gtype)
	else:
		print("\tFile Type: *default .png*")
		gtype = "png"
		
	fig = plt.figure()
	
	x_list = []
	x_names = []
	y_list = []
	
	for test in tests:
		x_list.append(test.getParam(x_axis))
		x_names.append(test.getParam('stitle'))
		y_list.append(test[y_axis])
		
	print x_list
	
	## This keeps track of where items are moved to
	## so the corresponding y-values will still match
	x_reorder = range(len(x_list))
	
	if ( order == 'i' ):
		sort(x_list,x_reorder, lambda a,b: a > b)
	elif ( order == 'd' ):
		sort(x_list,x_reorder, lambda a,b: a < b)
		
	print x_list
	print y_list
	print x_reorder
	
	y_reorder = [None] * len(y_list)
	for new, old in enumerate(x_reorder):
		y_reorder[new] = y_list[old]
		
	y_list = y_reorder

	## This simply reorders the y_list values so that they can
	## be passed to matplotlib to create the bars for the graph.
	## This only really matters if you have graphs with more than
	## one bar per x-axis node. For example, if you wanted to graph
	## page-size vs. the page faults for each individual process then
	## for each x node (ex 4096 bytes) you will have 1 or more
	## process bars
	mod_ylist = []
	max_y = 0 ##Used later in scaling graph
	x_names = []
	for proc in range(len(y_list[0])):
		mod_ylist.append([])
		for test in range(len(x_list)):
			val = y_list[test][proc]
			mod_ylist[proc].append(val)
			
			if ( val > max_y ):
				max_y = val
			
	print mod_ylist
	
	## Now we can actually get around to building
	## the graph.
	
	axis_labels = []
	try:
		axis_labels = axisLabels.getLabels(output)
		print("axisLabels = " + str(axis_labels))
	except:
		##Get the defaults
		axis_labels = (prettyConfig[x_axis],prettyConfig[y_axis])
		
	graph_title = None
	try:
		graph_title = axisLabels.getTitle(output)
		print("Graph_Title = " + str(graph_title))
	except:
		graph_title = "\"%s\" vs \"%s\"" % (prettyConfig[x_axis],prettyConfig[y_axis])
		
	print("X-Axis label: " + axis_labels[0])
	print("Y-Axis label: " + axis_labels[1])
	print("Graph Title: " + graph_title)

	barWidth = 1
	padWidth = 0.1 ##Padding between bars/bar sections
	N = len(x_list)
	NY = float(barWidth) / len(mod_ylist)
	ind = np.arange(0,N,barWidth + padWidth)

	ax = fig.add_subplot(111)
	ax.set_title(graph_title)
	ax.set_ylabel(axis_labels[1])
	ax.set_xlabel(axis_labels[0])
	ax.set_xticks(ind+ (barWidth / 2.))
	ax.set_xticklabels( tuple(x_list) )
	
	if ( max_y == 0 ):
		ax.set_ylim(0,1)
	else:
		ax.set_ylim(0, max_y * 1.1 )
	
	rects = []
	colors = ['r','y','b','g']
	for i, plotVal in enumerate(mod_ylist):
		print(tuple(plotVal))
		rects.append(ax.bar(ind+ (i * NY), tuple(plotVal), NY, color=colors[i % 4]))
	
	def autolabel(rects):
		for rect in rects:
		    height = rect.get_height()
		    ax.text(rect.get_x() + rect.get_width()/2.,1.03*height, '%.2f' % float(height),
		            ha='center', va='bottom')

	if ( len(rects) > 1 ):
		legend_rects = []
		legend_names = []
		for i, rectSet in enumerate(rects):
			legend_rects.append(rectSet[0])
			legend_names.append("P " + str(i))
		
		ax.legend(tuple(legend_rects), tuple(legend_names))

	for rectSet in rects:
		autolabel(rectSet)

	plt.show()
	print("Output File: " + output)
	print("File Type: ." + gtype) 
	plt.savefig(output, format=gtype)
		
	return True
