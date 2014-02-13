#!/usr/bin/env python
"""This script pre-processes a SHARCSpread parameter file, expanding any
production types that are given as patterns.

For example, to specify direct contact parameters *from* any unit whose
production type contains the term "beef" *to* any unit whose production type
contains the term "dairy",

<contact-spread-model from-production-type="$beef" to-production-type="$dairy" contact-type="direct">

(The $ tells the pre-processor that the production type is a pattern that must
be expanded.)

The search terms are not case-sensitive, so "dairy" as a search term would
match both "Dairy - milking" and "Mixed dairy and goats" production types.

You can also use multiple search terms, separated by spaces.  For example, to
specify direct contact parameters *from* any unit whose production type
contains the term "beef" or "dairy" *to* any unit whose production type
contains the term "dairy",

<contact-spread-model from-production-type="$beef dairy" to-production-type="$dairy" contact-type="direct">

You can also add terms that *must* be in the production type.  For example,
to specify direct contact *from* any unit whose production type contains the
terms "dairy" and "milking" *to* any unit whose production type contains the
term "dairy",

<contact-spread-model from-production-type="$+dairy +milking" to-production-type="$dairy" contact-type="direct">

You can also add terms that *must not* be in the production type.  For example,
to specify direct contact *from* any unit whose production type contains the
term "dairy" but not "milking" *to* any unit whose production type contains the
term "dairy",

<contact-spread-model from-production-type="$dairy -milking" to-production-type="$dairy" contact-type="direct">

Enclose multiple-word terms in quotes.  To adhere to the XML standard, you must
then enclose the entire attribute in single quotes.  For example, to specify
direct contact *from* any unit whose production type contains the terms "dairy"
and "not dealer" *to* any unit whose production type contains the term "dairy",

<contact-spread-model from-production-type='$dairy "not dealer"' to-production-type="$dairy" contact-type="direct">

Finally, you can use an empty attribute string to indicate that the parameters
apply to *all* production types.  For example, to specify direct contact *from*
any unit whose production type contains the term "cattle" *to* all other units,

<contact-spread-model from-production-type="$cattle" to-production-type="" contact-type="direct">


The resulting altered parameter file is written to standard output.
"""

__author__ = "Neil Harvey <neilharvey@canada.com>"
__date__ = "July 2004"

import getopt
import re
import sys
import xml.dom.pulldom
import xml.sax
from sets import Set
from string import join
from search_term_parser import SearchTermParser

DEBUG = False



def usage ():
	"""Prints a usage message."""
	print """\
Usage: python expand_parameters.py -h HERD-FILE PARAMETER-FILE > NEW-PARAMETER-FILE

Options are:
-h FILENAME                 the herd file
--help                      print a usage message"""



class productionTypeLocator (xml.sax.handler.ContentHandler):
	"""A SAX handler for production type elements in an XML herd file."""
	def __init__ (self):
		"""Constructor."""
		self.text = ""
		self.inProductionType = False
		self.productionTypes = Set()



	def startElement (self, name, attrs):
		"""Sets a flag when the parser enters a production type element."""
		if name == "production-type":
			self.inProductionType = True



	def endElement (self, name):
		"""Unsets a flag and adds the element text to the set of production
		type names when the parser leaves a production type element."""
		if name == "production-type":
			self.inProductionType = False
			self.productionTypes.add (self.text.strip())
			self.text = ""



	def characters (self, content):
		"""Accumulates the text inside a production type element (which may be
		returned to the parser in more than one chunk)."""
		if self.inProductionType:
			self.text += content



	def getProductionTypes (self):
		"""Returns the accumulated set of production types."""
		return self.productionTypes



def none (items):
	"""Returns True if every item in the list is a "non-value", False
	otherwise."""
	for item in items:
		if item:
			return False
	return True



def getText (node):
	"""Returns the text, if any, contained in an XML element."""
	text = ""
	for child in node.childNodes:
		if child.nodeType == xml.dom.Node.TEXT_NODE:
			text += child.data
	return text



def getProductionTypes (herd_file):
	"""Returns a list of production types found in the herd file.  The argument
	may be either a string (a filename) or a stream."""
	parser = xml.sax.make_parser()
	handler = productionTypeLocator()
	parser.setContentHandler (handler)
	parser.parse (herd_file)

	result = list (handler.getProductionTypes())
	result.sort()
	return result



def expandSingleProductionType (node, production_types):
	"""Expands the parameters for a model with a single production type
	attribute."""
	# We're only interested if the production type is blank, indicating that
	# the parameters apply to all units, or if the production type starts with
	# a $, indicating that it is a pattern.
	prodtype = node.getAttribute ("production-type")
	if prodtype == "":
		# Empty string means use *all* production types.
		prodtypes = production_types
	elif prodtype.startswith ("$"):
		prodtypes = SearchTermParser (prodtype[1:]).matches (production_types)
	else:
		if DEBUG:
			print "production type =", prodtype
		print node.toxml()
		return

	if DEBUG:
		print "production type(s) =", join (["\n  %s" %x for x in prodtypes])

	# Create copies of the model parameters, one per production type.
	modelName = node.localName
	if modelName in ("detection-model", "basic-destruction-model", "vaccine-model"):
		if len (prodtypes) < len (production_types):
			node.setAttribute ("production-type", join (prodtypes, ","))
		else:
			# An empty string is allowed as shorthand for "all production
			# types."
			node.setAttribute ("production-type", "")
		print node.toxml()
	else:
		for prodtype in prodtypes:
			newnode = node.cloneNode (deep=True)
			newnode.setAttribute ("production-type", prodtype)
			print newnode.toxml()
			newnode.unlink()


	
def expandToFromProductionType (node, production_types):
	"""Expands the parameters for a model with both "to" and "from" production
	type attributes."""
	# We're only interested if the production types are blank, indicating that
	# the parameters apply to all units, of if the production type starts with
	# a $, indicating that it is a pattern.
	from_prodtype = node.getAttribute ("from-production-type")
	to_prodtype = node.getAttribute ("to-production-type")
	if not (from_prodtype == "" or from_prodtype.startswith ("$")
	   or to_prodtype == "" or to_prodtype.startswith ("$")):
		if DEBUG:
			print "from production type =", from_prodtype
			print "to production type =", to_prodtype
		print node.toxml()
		return

	if from_prodtype == "":
		from_prodtypes = production_types
	elif from_prodtype.startswith ("$"):
		from_prodtypes = SearchTermParser (from_prodtype[1:]).matches (production_types)
	else:
		from_prodtypes = [from_prodtype]
	if DEBUG:
		print "from production type(s) =", join (["\n  %s" % x for x in from_prodtypes])

	if to_prodtype == "":
		to_prodtypes = production_types
	elif to_prodtype.startswith ("$"):
		to_prodtypes = SearchTermParser (to_prodtype[1:]).matches (production_types)
	else:
		to_prodtypes = [to_prodtype]
	if DEBUG:
		print "to production type(s) =", join (["\n  %s" %x for x in to_prodtypes])

	# Create copies of the model parameters, one per pairing of production
	# types.
	modelName = node.localName
	if modelName in ("airborne-spread-model", "trace-back-destruction-model", "ring-destruction-model", "ring-vaccination-model"):
		if len (from_prodtypes) < len (production_types):
			node.setAttribute ("from-production-type", join (from_prodtypes, ","))
		else:
			# An empty string is allowed as shorthand for "all production
			# types."
			node.setAttribute ("from-production-type", "")
		if len (to_prodtypes) < len (production_types):
			node.setAttribute ("to-production-type", join (to_prodtypes, ","))
		else:
			# An empty string is allowed as shorthand for "all production
			# types."
			node.setAttribute ("to-production-type", "")
		print node.toxml()
	else:
		for from_prodtype in from_prodtypes:
			for to_prodtype in to_prodtypes:
				newnode = node.cloneNode (deep=True)
				newnode.setAttribute ("from-production-type", from_prodtype)
				newnode.setAttribute ("to-production-type", to_prodtype)
				print newnode.toxml()
				newnode.unlink()


	
def expandProductionTypes (events, production_types):
	"""Expands a SHARCSpread parameter file as described in the program header
	comment.  The argument is a pull DOM object.  The function prints the XML
	tree, expanding any model element with a pattern for a production type."""
	tagclose = re.compile (r"/>$")
	for (event, node) in events:
		if event == xml.dom.pulldom.START_ELEMENT:
			tag = node.localName

			if tag.endswith ("-model") or tag.endswith ("-monitor"):
				events.expandNode (node)

				prodtype = node.getAttribute ("production-type")
				to_prodtype = node.getAttribute ("to-production-type")
				from_prodtype = node.getAttribute ("from-production-type")

				if none ([prodtype, to_prodtype, from_prodtype]):
					# This node does not have production types attributes.
					print node.toxml()
				else:
					# This node needs to be checked for expansion.
					if prodtype:
						expandSingleProductionType (node, production_types)
					else:
						expandToFromProductionType (node, production_types)
				#node.parentNode.removeChild (node)
				node.unlink()
				
			elif tag in ("description", "num-days", "num-runs", "output"):
				events.expandNode (node)
				print node.toxml()

			else:
				print tagclose.sub (">", node.toxml())

		elif event == xml.dom.pulldom.END_ELEMENT:
			# Print close tags for any element we didn't expand.
			print "</%s>" % node.tagName



def main ():
	# Get any command-line arguments.
	try:
		opts, args = getopt.getopt (sys.argv[1:], "h:", ["help"])
	except getopt.GetoptError, details:
		print details
		usage()
		sys.exit()

	# Need at least one command-line parameter, the name of the parameter file.
	if len (args) < 1:
		usage()
		sys.exit()
	parameter_file_name = args[0];
		
	herd_file_name = None
	for o, a in opts:
		if o == "-h":
			herd_file_name = a
		elif o == "--help":
			usage()
			sys.exit()

	# Need a herd file.
	if herd_file_name == None:
		usage()
		sys.exit()

	# Get a list of production types.
	production_types = getProductionTypes (herd_file_name)

	# Get the simulation parameters, as an XML DOM object.
	parameter_file_events = xml.dom.pulldom.parse (parameter_file_name)

	# Expand any parameters in which the production type is given as a pattern.
	print '<?xml version="1.0" encoding="UTF-8"?>'
	expandProductionTypes (parameter_file_events, production_types)



if __name__ == "__main__":
	main ()
