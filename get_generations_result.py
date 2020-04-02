#!/usr/bin/python3

import re
import csv

results = []
with open( 'log.txt' ) as fin:
	for line in fin:
		if re.match( '(.*)avg:(.*)', line ):
			tmp = line.split()
			results.append( tmp[1] )

i=1

results = results[1:]
with open( 'ecf_results.csv', 'w' ) as fout:
	for result in results:
		
		writer = csv.writer(fout, delimiter = ',')
		row = [ i, result ]
		writer.writerow( row )
		i = i+1

 
