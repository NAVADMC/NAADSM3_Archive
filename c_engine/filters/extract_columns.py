#!/usr/bin/env python
"""This script extracts one or more output variables from a simulation.  It
reads a table of outputs in comma-separated value format from standard input.

It will extract specific requested variables or, by default, will extract the
number of units in each state on each day.  It will extract results from
specific requested simulations or, by default, will extract the variables from
all simulations."""

__author__ = "Neil Harvey <neilharvey@gmail.com>"
__date__ = "September 2006"

import getopt
import os
import re
import string
import sys
from math import floor, sqrt
from sets import Set

DEBUG = False
STATE_NAMES = ["Susc", "Lat", "Subc", "Clin", "NImm", "VImm", "Dest", "Dead"]



def usage ():
	"""Prints a usage message."""
	print """\
Usage: python extract_columns.py [OPTIONS] "variablename1,variablename2,..." < TABLE-FILE

Options are:
-r, --run INT               only get results from given run (default = all)
-e, --exclude TEXT          exclude matching variable names
-n, --case-insensitive      case insensitive"""



def main ():
	# Set defaults for the command-line options.
	desired_run = None
	excluded_varnames = []
	case_insensitive = False

	# Get any command-line arguments.
	try:
		opts, args = getopt.getopt (sys.argv[1:], "r:e:nh", ["run=", "exclude=",
		  "case-insensitive", "help"])
	except getopt.GetoptError, details:
		print details
		sys.exit()
		
	for o, a in opts:
		if o in ("-h", "--help"):
			usage()
			sys.exit()
		elif o in ("-r", "--run"):
			desired_run = int(a)
		elif o in ("-e", "--exclude"):
			excluded_varnames = a.split(",")
		elif o in ("-n", "--case-insensitive"):
			case_insensitive = True

	if len (args) >= 1:
		desired_varnames = args[0].split(",")
	else:
		desired_varnames = ["tsdU" + statename for statename in STATE_NAMES]

	# Treat the desired and excluded varnames as regular expressions.
	if case_insensitive:
		regex_flags = re.IGNORECASE
	else:
		regex_flags = 0
	desired_varnames = [re.compile (desired_varname, regex_flags) for desired_varname in desired_varnames]
	excluded_varnames = [re.compile (excluded_varname, regex_flags) for excluded_varname in excluded_varnames]

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
	if DEBUG:
		print "indices =", indices

	if indices == []:
		print "Run,Day"
	else:
		print "Run,Day," + string.join ([varnames[i] for i in indices], ',')

	# Go through the results table.
	for line in sys.stdin:
		if DEBUG:
			print "line =", line,
		# Pick off the run # and day.  The remainder of the line is output
		# variable values.
		fields = line.strip().split(",", 2)
		if len (fields) > 2:
			run = int (fields[0])
			day = int (fields[1])
			fields = fields[2]

		if desired_run != None and run != desired_run:
			continue

		output = "%i,%i" % (run, day)
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
					varvalue = fields[:end + 1]
					fields = fields[end + 2:]
				elif fields.find (",") >= 0:
					varvalue, fields = fields.split(",", 1)
				else:
					varvalue = fields
					fields = ""
				j += 1
			output += "," + varvalue
		print output



if __name__ == "__main__":
	main()
