#!/usr/bin/env python
"""This class processes Google-style search terms."""

__author__ = "Neil Harvey <neilharvey@canada.com>"
__date__ = "July 2004"

import re

DEBUG = False



def listToEnglish (items, connective="or"):
	"""Returns the list as a string, with commas and the appropriate connective
    at the end.  E.g.,
	[] -> ""
	[A] -> "A"
	[A,B] -> "A or B"
	[A,B,C] -> "A, B or C"
	[A,B,C,D] -> "A, B, C or D"
	"""
	text = ""
	n = len (items)
	for i in range (n):
		if i > 0:
			if i == n - 1:
				text += " " + connective + " "
			else:
				text += ", "
		text += items[i]
	return text



class SearchTermParser:
	# Class variables
	termpat = re.compile (r'[+-]?(?:"([^"]+)"|([^ ]+))', re.I)

	def __init__ (self, pattern):
		"""Constructor.  Parses the pattern into three lists, of terms to
		search for, terms that must be included, and terms that must not be
		included.  (Or, "or", "and", and "not" lists.)"""
		if DEBUG:
			print "pattern = '%s'" % pattern

		self.or_terms = []
		self.and_terms = []
		self.not_terms = []
		while 1:
			match = self.termpat.match (pattern)
			if not match:
				break

			directive = match.group(0) # whole thing, including the '-'
			term = match.group(1) or match.group(2) # just the keyword
			term = term.lower()
			if directive.startswith ("-"):
				self.not_terms.append (term)
			elif directive.startswith ("+"):
				self.and_terms.append (term)
			else:
				self.or_terms.append (term)
			pattern = pattern[match.end(0):]
			pattern = pattern.lstrip()
			if DEBUG:
				print "or terms =", self.or_terms
				print "and terms =", self.and_terms
				print "not terms =", self.not_terms



	def matches (self, items):
		"""Returns the subset of items that
		a) contain at least one string from the "or" list, and
		b) contain all strings from the "and" list, and
		b) contain no strings from the "not" list."""
		result = []
		# Create all-lowercase copies of the lists, to make the substring searches
		# case-insensitive.
		items_lowercase = [item.lower() for item in items]
		for i in range (len (items)):
			item = items_lowercase[i]
			# Check that the item contains no "not terms".  If it contains one,
			# skip to the next item.
			keep = True
			for term in self.not_terms:
				if term in item:
					keep = False
					break
			if not keep:
				continue

			# Check that the item contains all "and terms".  If it is missing one,
			# skip to the next item.
			for term in self.and_terms:
				if term not in item:
					keep = False
					break
			if not keep:
				continue

			# Check whether the item contains at least one "or term".
			if len (self.or_terms) > 0:
				keep = False
			for term in self.or_terms:
				if term in item:
					keep = True
					break

			# Note that if the "not terms" and "and terms" are satisfied and there
			# are no "or terms", we will accept the item.
			# If the "not terms" are satisfied and there are no "and terms" or "or
			# terms", we will accept the item.
			# If there are no "not terms", "and terms", or "or terms", we will
			# accept the item (i.e., a blank search returns everything.)
			if keep:
				result.append (items[i])
		return result



	def toEnglish (self):
		"""Returns an english description of what the search terms will
		match."""
		text = ""
		if self.and_terms != []:
			text += "containing " + listToEnglish (['"' + x + '"' for x in self.and_terms], "and")

		if self.or_terms != []:
			if self.and_terms == []:
				text += "containing one or more of "
			else:
				text += ", plus one or more of "
			text += listToEnglish (['"' + x + '"' for x in self.or_terms], "or")

		if self.not_terms != []:
			if self.and_terms == [] and self.or_terms == []:
				text += "not containing "
			else:
				text += ", but not containing "
			text += listToEnglish (['"' + x + '"' for x in self.not_terms], "or")

		if self.and_terms == [] and self.or_terms == [] and self.not_terms == []:
			text += "containing anything"

		return text
