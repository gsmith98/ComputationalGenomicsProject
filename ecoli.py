#!usr/bin/env python2.7
#import sys to use stdin
import sys
import random
if __name__=='__main__':
	sequence = '';
	sub = '';
	input = open('sequence.fasta','r');
	output = open('reads.fna','w');
	for line in input:
		line = line.rstrip();
		sequence = sequence + line
	#print sequence;	
	#print len(sequence);
	for i in range(0,20000):	
		num = random.randint(1, 5498349);
		sub = ">seq"+str(i+1)+"\n"+ sequence[num:num+101]
		output.write(sub)
		#print sub		
	
	