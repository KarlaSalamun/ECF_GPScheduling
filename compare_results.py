#!/usr/bin/python3

import matplotlib.pyplot as plt
import numpy as np
import csv

x = []
ecf_tard = []
my_tard = []
dominance = 0;

with open('ecf_results.csv','r') as ecf_file:
	plots = csv.reader(ecf_file, delimiter=',')
	for row in plots:
		x.append(int(row[0]))
		ecf_tard.append(float(row[1]))	

with open('test_results.csv','r') as my_file:
	plots = csv.reader(my_file, delimiter=',')
	for row in plots:
		my_tard.append(float(row[1]))	

for i in range( len( my_tard ) ):
	if my_tard[i]<=ecf_tard[i]:
		dominance = dominance + 1;

dominance = (float)( dominance / len( my_tard ) )

plt.plot(x,ecf_tard, label='ECF results')
plt.plot(x, my_tard, label='my results')
plt.xlabel('generations')
plt.ylabel('tardiness')
plt.title('results')
plt.legend()
#plt.show()
plt.savefig('result.png')

stdev = np.std(ecf_tard)
print( stdev )
print( "ECF:\tocekivanje: {}\tvarijanca: {}".format( np.average(ecf_tard), np.std(ecf_tard) ) )
print( "ja:\tocekivanje: {}\tvarijanca: {}".format( np.average(my_tard), np.std(my_tard) ) )
print( "dominance: {}".format( dominance ) )
