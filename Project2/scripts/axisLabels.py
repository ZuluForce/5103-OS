def getLabels(out):
	if (out == "ps_vs_cs"):
		return ("Page Size (Bytes)", "Process CS per test")
	elif out == "ps_vs_csp":
		return ("Page Size (Bytes)", "Per-Process CS")
	else:
		raise NameError
		
		
def getTitle(out):
	if (out == "ps_vs_cs"):
		return "Page Size vs. Local CS Average"
	elif (out == "ps_vs_csp"):
		return "Page Size vs Per-Process Context Switches"
	else:
		raise NameError
