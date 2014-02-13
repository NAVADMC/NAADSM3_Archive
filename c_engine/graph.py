#!/usr/bin/env python
"""This script plots one or more output variables from a simulation.  It reads
a table of outputs in comma-separated value format from standard input.

It will plot specific requested variables or, by default, will try to plot the
number of units in each state on each day.  It will plot results from specific
requested simulations on specific requested nodes or, by default, will plot the
variables from all simulations."""

__author__ = "Neil Harvey <neilharvey@canada.com>"
__date__ = "August 2004"

import getopt
import os
import re
import string
import sys
from math import floor
from sets import Set

STATE_NAMES = ["Susc", "Lat", "Subc", "Clin", "NImm", "VImm", "Dest"]



def usage ():
	"""Prints a usage message."""
	print """\
Usage: python graph.py2 [OPTIONS] "variablename1,variablename2,..." < TABLE-FILE

Options are:
-n, --node INT              only show results from given node (default = all)
-r, --run INT               only show results from given run (default = all)
-x, --xlabel TEXT           set a label for the x-axis
-y, --ylabel TEXT           set a label for the y-axis
-e, --exclude TEXT          exclude matching variable names
--lw N                      set the gnuplot linewidth
--lt N                      set the gnuplot linetype
--hline N                   draw a horizontal line at value N
--right-y                   put 0-100 marks on the right y-axis
--eps, --epsfile FILENAME   output to an encapsulated postscript file
--png, --pngfile FILENAME   output to a portable network graphics file"""



def odd (n):
	"""Returns 1 if the given number is odd, 0 otherwise."""
	return n%2



def median (X, sorted=0):
	"""Returns the median for a variable."""
	if not sorted:
		X.sort()
	n = len(X)
	# The usual formula says take element (n+1)/2, but since Python uses zero-
	# based indexing, we substitute (n-1)/2.
	midpoint = (n-1)/2 # integer arithmetic; no fractional part
	if odd(n):
		return X[midpoint]
	else:
		return 0.5*(X[midpoint]+X[midpoint+1])



def percentile (X, p, sorted=0):
	"""Returns a quantile value for a variable.  For example, to get the
	75th percentile, use p=0.75."""
	if not sorted:
		X.sort()
	n = len(X)
	i = int (floor (p * (n-1)))
	delta = p * (n-1) - i

	try:
		v = (1 - delta)*X[i] + delta*X[i+1]
	except IndexError, details:
		print "X =", X
		print "p =", p
		print "i =", i
		raise IndexError (details)
	
	return v



class Line (list):
	"""A line for a plot or graph; contains a title and a set of points."""
	def __init__ (self, title=None, points=[]):
		"""Constructor."""
		self.title = title
		self += points



	def medianx (self, quantiles=False):
		"""Combines multiple points with the same x-coordinate into a median
		value.
		
		Parameters:
		quantiles = True  adds error bars showing 5% and 95% quantiles
		"""
		xcoords = Set ([x for x,y in self])
		yvalue = {}
		lowerbar = {}
		upperbar = {}
		for x in xcoords:
			yvalue[x] = []
			
		for x,y in self:
			yvalue[x].append (y)
		
		for x in xcoords:
			yvalue[x].sort()
			med = median (yvalue[x], sorted=True)
			if quantiles:
				if len (yvalue[x]) >= 2:
					lowerbar[x] = percentile (yvalue[x], 0.05, sorted=True)
					upperbar[x] = percentile (yvalue[x], 0.95, sorted=True)
				else:
					lowerbar[x] = yvalue[x][0]
					upperbar[x] = yvalue[x][0]
			yvalue[x] = med

		i = 0
		while (i < len(self)):
			x = self[i][0]
			# This is the first point with x-coordinate 'x'.  Set its y-
			# coordinate to the median value for 'x',
			if quantiles:
				self[i] = (x, yvalue[x], lowerbar[x], upperbar[x])
			else:
				self[i] = (x, yvalue[x])
			# then erase all other points with that x-coordinate.
			for j in range (len(self)-1, i, -1):
				if self[j][0] == x:
					del self[j]
			i += 1



	def __repr__ (self):
		"""Returns a Python expression that could be used to re-create this
		object."""
		return "Line (" + self.title.__repr__() + ", " + list.__repr__ (self) \
		  + ")"



	def __str__ (self):
		"""Returns the informal string representation of this object."""
		s = ""
		if self.title:
			s += self.title + " "
		s += list.__repr__ (self)
		return s



def main ():
	# Set defaults for the command-line options.
	desired_node = None
	desired_run = None
	eps_filename = None
	png_filename = None
	xlabel = "Day"
	xrange = ('', '')
	ylabel = ""
	yrange = ('', '')
	linewidth = 3
	linetype = 1
	hline = None
	righty = False
	excluded_varnames = []

	# Get any command-line arguments.
	try:
		opts, args = getopt.getopt (sys.argv[1:], "o:y:x:n:r:e:h", ["eps=",
		  "epsfile=", "outfile=", "png=", "pngfile=", "ylabel=", "xlabel=",
		  "yrange=", "xrange=", "node=", "run=", "lw=", "lt=", "exclude=",
		  "hline=", "right-y", "help"])
	except getopt.GetoptError, details:
		print details
		sys.exit()
		
	for o, a in opts:
		if o in ("-h", "--help"):
			usage()
			sys.exit()
		elif o in ("--eps", "--epsfile", "-o", "--outfile"):
			eps_filename = a
		elif o in ("--png", "--pngfile"):
			png_filename = a
		elif o in ("-y", "--ylabel"):
			ylabel = a
		elif o in ("-x", "--xlabel"):
			xlabel = a
		elif o == "--yrange":
			yrange = tuple (a.split(':'))
		elif o == "--xrange":
			xrange = tuple (a.split(':'))
		elif o in ("-n", "--node"):
			desired_node = int(a)
		elif o in ("-r", "--run"):
			desired_run = int(a)
		elif o == "--lw":
			linewidth = int(a)
		elif o == "--lt":
			linetype = int(a)
		elif o == "--hline":
			hline = float(a)
		elif o == "--right-y":
			righty= True
		elif o in ("-e", "--exclude"):
			excluded_varnames = a.split(",")

	if len (args) >= 1:
		desired_varnames = args[0].split(",")
	else:
		desired_varnames = ["tsdU" + statename for statename in STATE_NAMES]

	# Treat the desired and excluded varnames as regular expressions.
	desired_varnames = [re.compile (desired_varname) for desired_varname in desired_varnames]
	excluded_varnames = [re.compile (excluded_varname) for excluded_varname in excluded_varnames]

	# Read the header line.
	header = sys.stdin.readline().strip()
	varnames = header.split(",")[2:]

	# Find the indices of the desired variables.
	indices = []
	for i in range (len (varnames)):
		keep = False
		for desired_varname in desired_varnames:
			match = desired_varname.search (varnames[i])
			if match:
				keep = True
		for excluded_varname in excluded_varnames:
			match = excluded_varname.search (varnames[i])
			if match:
				keep = False
		if keep:
			indices.append (i)

	# We're going to build a set of Line objects.  They will be stored in a
	# dictionary, keyed by output variable name.
	lines = {}
	for i in indices:
		lines[varnames[i]] = Line (varnames[i])

	# Go through the results table.
	for line in sys.stdin:
		# Pick off the run # and day.  The remainder of the line is output
		# variable values.
		fields = line.strip().split(",", 2)
		if len (fields) > 2:
			day = int (fields[1])
			fields = fields[2]

		j = 0
		for i in indices:
			# Advance to the ith output variable value.  NB: don't try to use a
			# "split" here to split the entire input line at once, you'll get a
			# "maximum recursion limit exceeded" error for big files.
			while j <= i:
				# If the line begins with a quoted string, remove the quoted
				# string (because it might have commas inside it that will mess
				# us up).  NB: don't try to use a regular expression match
				# here, you'll get a "maximum recursion limit exceeded" error
				# for big files.
				if fields.startswith ('"'):
					end = fields.find ('"', 1)
					fields = fields[end + 1:]
				if fields.find (",") >= 0:
					varvalue, fields = fields.split(",", 1)
				else:
					varvalue = fields
					fields = ""
				j += 1
			varname = varnames[i]
			if varvalue != "":
				try:
					lines[varname].append ((day, float (varvalue)))
				except ValueError:
					lines[varname].append ((day, 0))

	# Check whether the graph actually contains any points.
	no_data = (len(lines) == 0)
	if no_data:
		xrange = ("0","1")
		yrange = ("0","1")

	#plot = sys.stdout
	plot = os.popen ("gnuplot -persist", "w")
	plot.write ("set xlabel '%s'\n" % xlabel)
	plot.write ("set xrange [%s:%s]\n" % xrange)
	plot.write ("set ylabel '%s'\n" % ylabel)
	plot.write ("set yrange [%s:%s]\n" % yrange)
	if righty:
		plot.write ("set ytics nomirror\n")
		plot.write ("set y2range [0:100]\n")
		plot.write ('set y2tics 0,10,100\n')
	if no_data:
		plot.write ("set noxtics\n")
		plot.write ("set noytics\n")
		plot.write ("set noy2tics\n")
	if eps_filename != None:
		plot.write ("set terminal postscript eps color 16\n")
		plot.write ("set output '%s'\n" % eps_filename)
	if png_filename != None:
		plot.write ("set terminal png large\n")
		plot.write ("set output '%s'\n" % png_filename)

	cmd = []
	if hline != None:
		cmd.append ("%g notitle w lines lt 0 lw %i" % (hline, linewidth))
	# Gnuplot requires that you plot each line twice: once for the line and
	# once for the errorbars.
	for i in range (len (indices)):
		index = indices[i]
		varname = varnames[index]
		if varname == "tsdUSusc":
			custom_linetype = 9
		elif varname == "tsdULat":
			custom_linetype = 6
		elif varname == "tsdUSubc":
			custom_linetype = 8
		elif varname == "tsdUClin":
			custom_linetype = 1
		elif varname == "tsdUNImm":
			custom_linetype = 2
		elif varname == "tsdUVImm":
			custom_linetype = 3
		elif varname == "tsdUDest":
			custom_linetype = 7
		else:
			custom_linetype = i+linetype
		cmd.append ("'-' title '%s' w lines lt %i lw %i" % (lines[varname].title, custom_linetype, linewidth))
		cmd.append ("'-' notitle w errorbars lt %i" % custom_linetype)
	if len(cmd) > 0:
		plot.write ("plot " + string.join (cmd, ",") + "\n")

	for i in indices:
		line = lines[varnames[i]]
		if len (line) > 0:
			line.medianx (quantiles=True)
			# Sort the points in the line by x-coordinate.
			line.sort()
			for i in range (2):
				for point in line:
					plot.write ("%g %g %g %g\n" % point)
				plot.write ("e\n")
		else:
			for i in range (2):
				plot.write ("0 0\n")
				plot.write ("e\n")		

	if no_data:
		plot.write ("set label 'No output variable found to create this plot' at 0.5,0.5 center\n")
		plot.write ("plot 1 notitle lt -1\n")

	plot.close()



if __name__ == "__main__":
	main()
