#!/usr/bin/env python
"""This script removes testsuite-related items from the list of "related pages"
in Doxygen's output, leaving just the top-level testsuite link.
"""

__author__ = "Neil Harvey <neilharvey@gmail.com>"
__date__ = "November 2005"

import re
import sys



def main ():
	linepat = re.compile (r'^<li>.*href="(testsuite|test-xml)-')
	for line in sys.stdin:
		if not linepat.match (line):
			print line,



if __name__ == "__main__":
	main()
