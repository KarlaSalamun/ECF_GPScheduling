#!/usr/bin/python3

import xml.etree.ElementTree as ET
import sys, getopt
import argparse
from graphviz import Digraph

dot = Digraph(comment='Test tree')

parser = argparse.ArgumentParser()
parser.add_argument( '-b', '--best', required = True, help = 'input .xml file ( best )' )
parser.add_argument( '-p', '--params', required = True, help = 'parameters file' )
parser.add_argument( '-o', '--output', required = True, help = 'output .dot file' )

args = vars(parser.parse_args())

filein = args['best']
params = args['params']
fileout = args['output']

tree = ET.parse( params )
root = tree.getroot()
functions = root[0][0][2].text.split();
terminals = root[0][0][3].text.split();

tree = ET.parse( filein )
root = tree.getroot()
prefix_sol = root[1].text.split()

tmp_stack = []

for node_id, iter in enumerate(reversed( prefix_sol )):
	node_id = str( node_id )
	print( 'iter',iter )
	dot.node( node_id, iter )	
	print(tmp_stack)
	if iter in functions:
		dot.edge( node_id, tmp_stack.pop() )
		if iter != 'positive':
			dot.edge( node_id, tmp_stack.pop())
		if int(node_id) != len(prefix_sol):
			tmp_stack.append( node_id )
	elif iter in terminals:
		tmp_stack.append( node_id )

dot.render( filename = 'tree_graphs/' + str(fileout), cleanup = True )
