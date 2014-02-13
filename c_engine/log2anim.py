#!/usr/bin/env python2
"""This script takes a herd file and a simulation output log and outputs
numbered animation frames (named frameNNNN.png) and an MPEG-2 move file (named
anim.mpg).

Requires the program "convert" from the free ImageMagick package, which in turn
requires the MPEG-2 Video Codec v1.2.

http://www.imagemagick.org/www/archives.html
http://www.mpeg.org/MPEG/MSSG/#source
"""

__author__ = "Neil Harvey <neilharvey@canada.com>"
__date__ = "August 2003"

import getopt
import GIS
import Image
import ImageDraw
import ImageFont
import os
import re
import sys
from categorizer import Categorizer

DEFAULT_FONT = "helvB10.pil"
DEFAULT_BGCOLOR = (212,208,200)
PADDING = 1.25
TEXT_PAD = 2



class Draw (ImageDraw.Draw):
	"""An extended version of the PIL Draw object.
	
	Adds a 1-pixel outline option to text drawing."""
	def text (self, position, string, font=None, fill=None, outline=None):
		if outline != None:
			text_x, text_y = position
			for offset_x, offset_y in [
			  (-1,-1), ( 0,-1), ( 1,-1),
			  (-1, 0),          ( 1, 0),
			  (-1, 1), ( 0, 1), ( 1, 1)]:
				ImageDraw.Draw.text (self, (text_x + offset_x,
				  text_y + offset_y), string, font=font, fill=outline)
		ImageDraw.Draw.text (self, position, string, font=font, fill=fill)



def usage ():
	"""Prints a usage message."""
	print """\
Usage: python log2anim.py [OPTIONS] HERD-FILE < LOG-FILE

Options are:
-h, --help          print this usage message and exit
-n, --node INT      show results from this node (default=0)
-r, --run INT       show results from this Monte Carlo run (default=0)
-s, --size INTxINT  output frames of this resolution (default=250x250)

-m, --map SERVER;PROJECTION;LAYERS
                    use as background a map from this Web Map Service, e.g.,
                    -m http://atlas.gc.ca/cgi-bin/atlaswms_en;EPSG:4326;nat_bounds,can_7.5m,cap_symb_e,cap_labels
                    (default=plain gray background)"""



def main ():
	# Process the command-line options.
	node = 0
	run = 0
	width = 250
	height = 250
	server = None
	srs = None
	layers = None

	try:
		opts, args = getopt.getopt (sys.argv[1:], "n:r:s:m:h", ["node=",
		  "run=", "size=", "map=", "help"])
	except getopt.GetoptError, details:
		print details
		usage()
		sys.exit()

	for o, a in opts:
		if o in ("-n", "--node"):
			node = int (a)
		elif o in ("-r", "--run"):
			run = int (a)
		elif o in ("-s", "--size"):
			width, height = [int(val) for val in a.split('x')]
		elif o in ("-m", "--map"):
			server, srs, layers = a.split(';')
		elif o in ("-h", "--help"):
			usage()
			sys.exit()

	if len (args) < 1: # must have at least a herd file on the command-line.
		usage()
		sys.exit()

	# Read the herds file.
	herds = []
	fp = open (args[0])
	fp.readline() # discard the header line
	for line in fp:
		size, lat, lon = line.split (',')[1:4]
		herds.append ((int(size), float(lat), float(lon)))
	fp.close()
	nherds = len (herds)

	# Get a bounding box around the herds.
	if srs != None:
		projection = GIS.projection[srs]
	else:
		projection = GIS.LinearProjection()
	coords = [projection.getCoords (lat, lon) for size, lat, lon in herds]
	xs = [x for x, y in coords]
	ys = [y for x, y in coords]
	minx = min(xs)
	maxx = max(xs)
	miny = min(ys)
	maxy = max(ys)
	pad = (PADDING - 1.0)/2 * max(maxx - minx, maxy - miny)
	minx -= pad
	maxx += pad
	miny -= pad
	maxy += pad
	bbox = GIS.adjustBox ((minx, miny, maxx, maxy), float(width) / height)

	# Make the background, either a blank image or a map, if requested.
	if server == None:
		background = Image.new ('RGB', (width, height), DEFAULT_BGCOLOR)
	else:
		wms = GIS.WebMapService (server, cachedir=os.getcwd())
		try:
			background = wms.getMap (layers, '', srs, bbox, width, height)
		except IOError, details:
			raise IOError, "Attempt to contact " + wms.shortname + " caused " + str(details)
		background = background.convert ('RGB')

	# In the animation herd size is represented by dot size, and latitude and
	# longitude become x,y positions in the frame.  Do those conversions now.
	dotsizer = Categorizer ((0, 101, 501, 1001))
	dotsizes = (1,2,3,4)
	for i in range (nherds):
		size, lat, lon = herds[i]
		x, y = projection.getCoords (lat, lon)
		x, y = GIS.imageCoords (bbox, (width, height), x, y)
		dotsize = dotsizes [dotsizer.category_for(size)]
		herds[i] = (dotsize, x, y)

	# Set up some things for drawing.
	font = ImageFont.load_path (DEFAULT_FONT)
	status_color = (
	  (  0,  0,  0),
	  (230,230,  0),
	  (230,153,  0),
	  (204,  0,  0),
	  (  0,168,  0),
	  (  0,  0,168),
	  (255,255,255),
	  (204,204,204)
	)

	# Read the simulation log file.
	day = 1
	noderunpat = re.compile (r"node (\d+) run (\d+)")
	while 1:
		line = sys.stdin.readline()
		if not line:
			break

		match = noderunpat.match (line)
		if (not match) or int (match.group(1)) != node or int (match.group(2)) != run:
			continue

		line = sys.stdin.readline()
		statuses = [int(status) for status in line.split()]

		frame = background.copy()
		draw = Draw (frame)

		# Draw the herds.
		i = 0
		for status in statuses:
			dotsize, x, y = herds[i]
			draw.rectangle ((x-dotsize, y-dotsize, x+dotsize, y+dotsize),
			  fill=status_color[status])
			i += 1

		# Include the day, to make it easy to look up interesting events in the
		# log file.
		text = "Day %i" % day
		textsize = draw.textsize (text, font=font)
		draw.text ((width - textsize[0] - TEXT_PAD, height - textsize[1] - TEXT_PAD),
		  text, font=font, fill=(0,0,0), outline=DEFAULT_BGCOLOR)
		
		frame.save ('frame%04i.png' % day)
		day += 1

	# Add a closing screen
	frame = Image.new ('RGB', (width, height), (0,0,0))
	draw = Draw (frame)
	text = "Only virtual animals died during"
	textsize = font.getsize (text)
	draw.text (((width - textsize[0]) / 2, height / 2 - textsize[1]),
	  text, font=font, fill=(255,255,255))
	text = "the making of this animation"
	textsize = font.getsize (text)
	draw.text (((width - textsize[0]) / 2, height / 2),
	  text, font=font, fill=(255,255,255))
	frame.save ('frame%04i.png' % day)

	os.system ("convert frame*.png anim.m2v")
	os.rename ("anim.m2v", "anim.mpg")
	os.system ("rm -f frame*.png")



if __name__ == "__main__":
	main()
