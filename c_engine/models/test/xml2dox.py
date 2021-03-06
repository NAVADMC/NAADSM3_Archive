#!/usr/bin/env python
"""This script converts an XML description of test cases into a C comment
formatted for Doxygen.

Copyright (C) University of Guelph, 2003-2008

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version."""

__author__ = "Neil Harvey <neilharvey@gmail.com>"
__date__ = "December 2003"

import re
import sys
import time
import xml.dom.minidom
from sets import Set
from string import upper, join
from warnings import warn

OUTPUT_ENCODING = "utf-8" # Doxygen's preferred encoding



def getText (element):
	"""Returns the contents of all text children of the given element,
	concatenated, as a string."""
	text = ""
	for node in element.childNodes:
		if node.nodeType == node.TEXT_NODE:
			text += node.data

	return text.strip()



def getIntensity (color):
	"""Returns the intensity (a.k.a. value in HSV color space) of a color.  The
	color parameter must be an (r,g,b) tuple where each number is between 0 and
	255 inclusive.  The intensity is a value from 0 to 1 inclusive."""
	return 1.0 * max(color) / 255



def getColor (state):
	"""Returns the color corresponding to a state.  The state parameter may be a
	single-letter state code, or a sequence of state code / count pairs, such as
	S50L25.  The return value is an (r,g,b) tuple where each number is between 0
	and 255 inclusive.  If the state cannot be read, returns white as a default."""

	color = (0xff,0xff,0xff) # default

	state_color = {
	  'S': (0xff, 0xff, 0xff), # white
	  'L': (0xe5, 0xe5, 0x00), # yellow
	  'B': (0xe5, 0x99, 0x00), # orange
	  'C': (0xcc, 0x00, 0x00), # red
	  'N': (0x00, 0xa8, 0x00), # green
	  'V': (0x00, 0x00, 0xa8), # blue
	  'D': (0x00, 0x00, 0x00), # black
	  'X': (0x7f, 0x7f, 0x7f)  # grey
	}

	# One unit-level state.
	if state_color.has_key(state):
		color = state_color[state]

	return color



def getUnitNames (filename):
	"""Returns an array of strings, giving the names of units in the population
	file in the order they appear."""
	try:
		doc = xml.dom.minidom.parse (filename)
		names = [getText(node) for node in doc.getElementsByTagName ("id")]
	except IOError:
		names = None
	return names



def main ():
	doc = xml.dom.minidom.parse (sys.stdin)
	print """\
/** @file
 *
 * A dummy C file containing only comments.  This file causes the
 * <a href="testsuite.html">model test suite pages</a> to be built.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date %s
 *
 * Copyright &copy; University of Guelph, 2003-%s
 */
""" % (time.strftime ("%B %Y"), time.strftime ("%Y"))

	print """/** @page testsuite Model test suite
 *
 * Some thoughts about testing:	
 *
 * &ldquo;The first principle is that you must not fool yourself &ndash;
 * and you are the easiest person to fool.
 * I'm talking about a specific, extra type of integrity ...
 * bending over backwards to show how you're maybe wrong.&rdquo;<br>
 * &ndash; Richard Feynman, in a Caltech commencement
 * address given in 1974, also in <i>Surely You're Joking, Mr. Feynman!</i>
 *
 * &ldquo;The competent programmer is fully aware of the strictly limited size of his
 * own skull; therefore he approaches the programming task in full humility&rdquo;<br>
 * &ndash; Edsger W. Dijkstra, in <i>The Humble Programmer</i>, 1972.
 *"""

	# Find the categories.  We will sort the output by these.
	categories = Set ([getText (element) for element in doc.getElementsByTagName ("category")])
	categories = list (categories)
	categories.sort()

	# Create "display" names for each category by changing dashes to spaces and
	# capitalizing the first letter.
	for i in range(len(categories)):
		displayName = categories[i].replace("-", " ")
		displayName = re.sub ("^[a-z]", lambda m: upper(m.group(0)), displayName)
		categories[i] = (categories[i], displayName)

	tests = filter (
	  lambda node:node.nodeType == node.ELEMENT_NODE
	    and node.localName in ("deterministic-test", "stochastic-test", "variable-test", "stochastic-variable-test"),
	  doc.getElementsByTagName ("tests")[0].childNodes
	)

	# Count how many tests there are in each category.
	testcount = {}
	for category, displayName in categories:
		testcount[category] = 0
	for test in tests:
		category = getText (test.getElementsByTagName("category")[0])
		testcount[category] += 1

	# Print an index at the top.
	print ' * <ul>'
	for category, displayName in categories:
		print ' *   <li><a href="testsuite-%s.html">%s</a> (%i tests)' % \
		  (category, displayName, testcount[category])
		num = 0
		print ' *     <ul>'
		for test in tests:	
			if getText (test.getElementsByTagName ("category")[0]) != category:
				continue
			num += 1
			print ' *       <li><a href="testsuite-%s.html#%s%i">%s</a></li>' \
			  % (category, category, num, getText (test.getElementsByTagName ("short-name")[0]))
		print ' *     </ul>'
	print ' * </ul>'
	print ' *'
	print ' * Total %i tests' % sum ([testcount[category] for category in testcount.keys()])
	print ' */'

	state_code = {
	  'Susceptible': 'S', # map long names to short ones
	  'Latent': 'L',
	  'InfectiousSubclinical': 'B',
	  'InfectiousClinical': 'C',
	  'NaturallyImmune': 'N',
	  'VaccineImmune': 'V',
	  'Destroyed': 'D'
	}

	# This will cache the names of units in the test population.  The keys are
	# population file names and the values are arrays of strings, giving the names
	# of units in the order in which they appear.    
	unitNames = {}

	# Print a page of detailed entries for each category.
	for category, displayName in categories:
		print
		print '/** @page testsuite-%s %s tests\n *' % (category, displayName)

		# Print an index at the top.
		print ' * <ul>'
		num = 0
		for test in tests:	
			if getText (test.getElementsByTagName ("category")[0]) != category:
				continue
			num += 1
			print ' *   <li><a href="#%s%i">%s</a></li>' \
			  % (category, num, getText (test.getElementsByTagName ("short-name")[0]))
		print ' * </ul>'
		print ' *'
		print ' * Total %i tests' % testcount[category]
		print ' *'
		print ' * <a href="testsuite.html"><b>Back</b></a> to master list of tests.\n *'
		print ' *'

		num = 0
		for test in tests:
			if getText (test.getElementsByTagName ("category")[0]) != category:
				continue
			num += 1
			shortName = getText (test.getElementsByTagName ("short-name")[0])
			print ' * @section %s%i %s\n *' % (category, num, shortName)

			testtype = test.tagName

			description = getText (test.getElementsByTagName ("description")[0])
			for line in description.split('\n'):
				print ' * %s' % line.strip().encode(OUTPUT_ENCODING, "replace")
			print ' *'

			try:
				authors = test.getElementsByTagName ("author")
				if len(authors) == 1:
					print ' * Author: %s' % getText(authors[0]).encode(OUTPUT_ENCODING, "replace")
				else:
					print ' * Authors: %s' % join([getText(author).encode(OUTPUT_ENCODING, "replace") for author in authors], ', ')
				print ' *'
			except IndexError:
				warn ("Missing author for test %s/%s" % (category, shortName))

			try:
				creation_date = getText (test.getElementsByTagName ("creation-date")[0])
				print ' * Created: %s' % creation_date
				print ' *'
			except IndexError:
				warn ("Missing creation date for test %s/%s" % (category, shortName))

			try:
				last_modified_date = getText (test.getElementsByTagName ("last-modified-date")[0])
				print ' * Last modified: %s' % last_modified_date
				print ' *'
			except IndexError:
				pass

			try:
				model_version = getText (test.getElementsByTagName ("model-version")[0])
				print ' * Applies to model specification %s' % model_version
				print ' *'
			except IndexError:
				warn ("Missing model spec version for test %s/%s" % (category, shortName))

			parameters = getText (test.getElementsByTagName ("parameter-description")[0])
			for line in parameters.split('\n'):
				print ' * %s' % line.strip().encode(OUTPUT_ENCODING, "replace")
			print ' *'

			paramFileName = getText (test.getElementsByTagName ("parameter-file")[0])
			print ' * See the <a href="test-xml-%s-%s.html">parameters as XML</a>.' \
			  % (category, paramFileName.replace('.','_8'))
			# The replace above deals with an odd behavior of Doxygen: if you
			# use the @page directive to create a documentation page, but there
			# is a period in the page name (e.g., the test suite contains pages
			# for parameter files with "radius_2.5" in their name), then
			# Doxygen will use "_8" in place of the period.
			print ' *'

			herdFileName = getText (test.getElementsByTagName ("herd-file")[0])
			if not unitNames.has_key (herdFileName):
				unitNames[herdFileName] = getUnitNames (herdFileName + ".xml")
			diagrams = test.getElementsByTagName ("diagram")
			if len (diagrams) >= 1:
				for element in diagrams:
					diagram = getText (element)
					print ' * @image html %s.png Using %s.xml' % (diagram, herdFileName)
			else:
				print ' * @image html %s.png Using %s.xml' % (herdFileName, herdFileName)
			print ' *'

			noutcomes = 0
			tables = test.getElementsByTagName ("output")
			for table in tables:
				probability = table.getAttribute ("probability")
				if probability != "":
					noutcomes += 1
					print ' * Possible outcome #%i (expected frequency %s):' % (noutcomes, probability)
					print ' *'
				print ' * <table>'
				print ' *   <tr>'
				print ' *     <th>Day</th>',
				rows = table.getElementsByTagName ("tr")
				if testtype == "deterministic-test" or testtype == "stochastic-test":
					# Print the units' names.
					names = unitNames[herdFileName]
					if not names:
						# Find out how many units there are.
						nherds = len (rows[0].getElementsByTagName ("td"))
						names = [str(n) for n in range(nherds)]
					print '<th>Unit %s</th>' % names[0]
					for name in names[1:]:
						print ('<th>%s</th>' % name),
				elif testtype == "variable-test" or testtype == "stochastic-variable-test":
					# Find out how many output variables there are.
					for cell in rows[0].getElementsByTagName ("td"):
						print ('<th>%s</th>' % getText(cell).encode(OUTPUT_ENCODING, "replace")),
					del rows[0]
				print '\n *   </tr>'

				day = 0
				for row in rows:
					print ' *   <tr>'
					day += 1
					print (' *     <td>%i</td>' % day),
					for cell in row.getElementsByTagName ("td"):
						if testtype == "deterministic-test" or testtype == "stochastic-test":
							state = getText (cell)
							if state_code.has_key(state):
								state = state_code[state] # convert long name to short
							background_color = getColor (state)
							if getIntensity (background_color) > 0.5:
								text_color = 'black'
							else:
								text_color = 'white'
							print ('<td style="background-color:#%02x%02x%02x;color:%s">%s</td>' % (background_color+(text_color,state))),
						elif testtype == "variable-test" or testtype == "stochastic-variable-test":
							value = getText (cell) or "&nbsp;"
							print ('<td>%s</td>' % value),
					print '\n *   </tr>'
				print ' * </table>\n *'

			print ' * <a href="#testsuite-%s"><b>Back</b></a> to list of %s tests.\n *' \
			  % (category, displayName)
			print ' * <a href="testsuite.html"><b>Back</b></a> to master list of tests.\n *'
		print ' */'

	# Create pages showing the actual XML parameter files.
	for category, displayName in categories:
		for test in tests:
			if getText (test.getElementsByTagName ("category")[0]) != category:
				continue

			paramFileName = getText (test.getElementsByTagName ("parameter-file")[0])

			print
			print "/** @page test-xml-%s-%s %s/%s.xml File Reference" \
			  % (category, paramFileName, category, paramFileName)
			print " * @verbinclude model.%s/%s.xml" % (category, paramFileName)
			print " */"

	print
	print "/* end of file */"


if __name__ == "__main__":
	main()
