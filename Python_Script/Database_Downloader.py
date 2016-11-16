#!/usr/bin/env python2.7

from __future__ import division
from math import floor, ceil
from contextlib import closing
from multiprocessing.dummy import Pool
from os import path, makedirs, rmdir, remove
import re
from shutil import copyfileobj
from subprocess import call
from sys import stdout
from urllib2 import urlopen

Count = 0
#Set the max amount of files to fetch at a time. Max is 10000 files.
retmax = 10000
#Select the output format and file format of the files. 
#For table visit: http://www.ncbi.nlm.nih.gov/books/NBK25499/table/chapter4.chapter4_table1
format = 'fasta'
type = 'text'
#Create the options string for efetch
options = '&rettype='+format+'&retmode='+type

def update_progress(progress):
	barLength = 20 # Modify this to change the length of the progress bar
	status = ""
	if progress >= 1:
		progress = 1
		status = "Done...\r\n"
	progress = round(progress, 4)
	block = int(round(barLength*progress))
	text = "\rPercent: [{0}] {1}% {2}".format( "#"*block + "-"*(barLength-block), progress*100, status)
	stdout.write(text)
	stdout.flush()
	
def generate_Urls():
	global total_num_urls
	total_num_urls = 0
	#Range of all sequences to download
	for i in xrange(0,Count,retmax):
		total_num_urls += 1
		#Create the position string
		position = '&retstart='+str(i)+'&retmax='+str(retmax)
		#Create the efetch URL
		url = base + generate_url_prams() + position + options
		#create the output filename
		filename = directory + '/query'+str(i)+'.fasta'
		#counter checks the number of times the program tried to restart the download
		counter = 0
		start = i
		#returns generator
		update_progress((i+retmax)/Count)
		yield url, filename, start, counter
	print str(total_num_urls) + ' URLs generated. Starting downloads: '

def generate_url_prams():
	#Create the search string from the information above
	esearch = 'esearch.fcgi?db='+Database+'&term='+SearchQuery+'&usehistory=y&retmax=0'
	#Create your esearch url
	url = base + esearch
	#Fetch your esearch using urllib2
	response = urlopen(url).read()
	#Grab the amount of hits in the search
	WebEnv = re.search(r'<WebEnv>(.+)</WebEnv>', response).group(1)
	#Grab the QueryKey
	QueryKey = re.search(r'<QueryKey>(.+)</QueryKey>', response).group(1)
	#Create the fetch string
	efetch = 'efetch.fcgi?db='+Database+'&WebEnv='+WebEnv+'&query_key='+QueryKey
	#returns the substring of the url
	return efetch

def generate_Urls2(Error_list):
	global total_num_urls
	total_num_urls = 0
	#Range of all sequences to download
	counter = 0
	for h in Error_list:
		counter +=1
		for i in xrange(0, retmax, 50):
			total_num_urls += 1
			url = base + generate_url_prams2(h+i,50)
			filename = directory + '/query_'+str(h+i)+'.fasta'
			counter = 0
			start = h + i
			#returns generator
			yield url, filename, start, counter

def generate_url_prams2(start, max):
	#Create the search string from the information above
	esearch = 'esearch.fcgi?db='+Database+'&term='+SearchQuery+'&retstart='+str(start)+'&retmax='+str(max)
	#Create your esearch url
	url = base + esearch
	#Fetch your esearch using urllib2
	response = urlopen(url).read()
	#Grab the amount of hits in the search
	ids = re.findall(r'<Id>([0-9]+)</Id>', response)
	efetch = 'efetch.fcgi?db='+Database+options+'&id='
	for i in ids:
		efetch += i+','
	#returns the substring of the url
	return efetch	
	
def download((url, filename, start, counter)):
	#amount of times you want to reload the download
	if counter < 3:
		#Try to download
		try:
			#Gets a response and creates a file
			with closing(urlopen(url)) as response, open(filename, 'wb') as file:
				#copies the response to the file
				copyfileobj(response, file)
		#Restart download when there is an error
		except Exception as e:
			return download((url, filename, start, counter + 1))
		else: # success
			return (url, filename, start), None
	else:
		#After trying to reload it X times it returns with an error statement
		try:
			with closing(urlopen(url)) as response, open(filename, 'wb') as file:
				copyfileobj(response, file)
		except Exception as e:
			return (url, filename, start), repr(e)
		else: # success
			return (url, filename, start), None
			
def Combine_files(Error_list):
	missing_sequences = 0
	for h in Error_list:
		with open(directory + '/query'+str(h)+'.fasta', 'wb') as output:
			counter = 0
			for i in xrange(0, retmax, 50):
				with open(directory + '/query_'+str(h+i)+'.fasta') as input:
					for line in input:
						if '>' in line:
							counter+=1
						output.write(line)
				remove(directory + '/query_'+str(h+i)+'.fasta')
		print 'Checking redownloaded files.'
		print 'query_'+str(h)+'.fasta":' +str(counter)+' out of '+str(retmax)+' sequences'
		missing_sequences += counter-retmax
	print 'Total missing sequences: '+str(missing_sequences)

def check_Directory():
	global directory
	directory = path.dirname(path.realpath(__file__))+'/database_files'
	if path.exists(directory) == False:
		makedirs(directory)			
	else:
		try:
			rmdir(directory)
		except OSError as e2:
			print 'The directory database_files is not empty. Please move or Delete the files'
			exit()
		else:
			makedirs(directory)
		
def get_User_input():
	global SearchQuery, Database
	SearchQuery = raw_input('What is your search query: ')
	while ' ' in SearchQuery:
		SearchQuery = raw_input('Rememeber that spaces are replaced with +\'s.\nPlease re-eter your search query: ')
	Database = raw_input('Please enter your database: ')
	
def check_esearch():
	global base, Count, retmax
	#This is the base url for NCBI eutils.
	base = 'http://eutils.ncbi.nlm.nih.gov/entrez/eutils/'
	#Create the search string from the information above
	esearch = 'esearch.fcgi?db='+Database+'&term='+SearchQuery+'&usehistory=y&retmax=0'
	#Create your esearch url
	url = base + esearch
	content = ''
	#Fetch your esearch using urllib2
	response = urlopen(url).read()
	try:
		#Grab the amount of hits in the search
		Count = int(re.search(r'<Count>(.+)</Count><RetMax>', response).group(1))
	except Exception as e:
		Count = 0
	else:
		if (floor(Count/retmax)<20):
			if (floor(Count/5000)>20):
				retmax = 5000
			elif (floor(Count/1000)>20):
				retmax = 1000
			else:
				retmax = 500
			
def parse_filter_write_output():
	store = False
	list = []
	with open(directory+'/all_query.fasta', 'a') as output:
		with open(directory+'/all_discarded.fasta', 'a') as discard:
			for i in xrange(0,Count,retmax):
				with open(directory+'/query'+str(i)+'.fasta') as input: 
					for line in input.readlines():
						if '>' in line:
							if store == True:
								try:
									key = species.split('_')[0]+'_'+species.split('_')[1]+'_'+protein
								except Exception as e:
									key = species+'_'+protein
								check = False
								for position, element in enumerate(list):
									if element[0] == key:
										if	len(sequence) > len(element[4]):
											list[position] = [key, protein, genbank, species, sequence]
										check = True
										break
								if check == False:
									list.insert(0,[key, protein, genbank, species, sequence])
									if len(list) > 100:
										if list[100][4].strip() != '':
											output.write('>'+list[100][3]+list[100][1]+'{'+list[100][2]+'}\n')
											output.write(list[100][4]+'\n')
										list.pop()
							elif store == False:
								try:
									discard.write(delete_line+sequence+'\n')
								except NameError:
									sequence = ''
							sequence = ''
							findspaces=r'(\W+)'
							try:
								searchstr=r'.+[a-z]+\|(.*)\.\d\|(.*)\[(.+)\]'
								result=re.search(searchstr,line)
								genbank=result.group(1)
								protein=result.group(2)
								species=result.group(3)
								protein=re.sub(r"(\W+)", "_", protein)
								species=re.sub(r"(\W+)", "_", species)	
							except Exception as e:
								try:
									if 'AltName' in line:
										searchstr=r'.+sp\|(.*)\.\d\|(.+)\sRecName:\sFull=(.+); AltName:'
									else:
										searchstr=r'.+sp\|(.*)\.\d\|(.+)\sRecName:\sFull=(.+)'
									result=re.search(searchstr,line)
									genbank=result.group(1)
									protein=result.group(2)
									species=result.group(3)
									protein=re.sub(r"(\W+)", "_", protein)
									species=re.sub(r"(\W+)", "_", species)	
								except Exception as e:
									store = False
									delete_line = line
								else:
									store = True
							else:			  
								store = True					
						else:
							if '.' and '+' not in line:
								sequence += line.strip()
				update_progress((i+retmax)/Count)
			key = species.split('_')[0]+'_'+species.split('_')[1]+'_'+protein
			for position, element in enumerate(list):
				if element[0] == key:
					if	len(sequence) > len(element[4]):
						list[position] = [key, protein[:-1], genbank, species, sequence]
						check = True
						break
			if check == False:
				list.insert(0,[key, protein[:-1], genbank, species, sequence])
			for element in list:
				if element[4].strip() != '':
					output.write('>'+element[3]+element[1]+'{'+element[2]+'}\n')
					output.write(element[4]+'\n')
	
def parse_end():
	with open(directory+'/final_database.fasta', 'a') as output:
		with open(directory+'/all_uniques.fasta') as input: 
			for line in input.readlines():
				if '>' in line:
					output.write(line.split(';')[0]+'\n')
				else:
					output.write(line)
	
def main():
	print 'Database Download by Jack Zhan'
	check_Directory()
	while Count == 0:
		get_User_input()
		check_esearch()
		if Count == 0:
			print '0 queries found. Restarting ....'
	print ''
	print str(Count)+' sequences found. Preparing to download files.\n'
	print str(int(ceil(Count/retmax)))+' URL\'s will be created.'
	
	#Create multiple threads. Limit to 20 concurrent downloads
	threads = 0
	while threads == 0:
		try:
			threads = int(raw_input('Number of threads to use. Recommended number is 40: '))
		except Exception as e:
			threads = 0
	pool = Pool(threads)
	#creates the urls
	print 'Generating and mapping URLs'
	urls = generate_Urls()
	#downloads each of the mapped urls
	counter = 0
	Error_message = False
	Error_list = []
	for (url, filename, start), error in pool.imap_unordered(download, urls):
		if str(int(retmax * floor(Count/retmax))) not in filename:
			counter2 = 0
			with open(filename) as check:
				for line in check:
					if '>' in line:
						counter2 +=1
			if counter2 != retmax:
				Error_list.append(start)
				Error_message = True
		counter += 1
		update_progress(counter/total_num_urls)

	if Error_message == True:
		print 'Error was encountered when downloading files.'
		print 'Restarting downloads of '+str(len(Error_list))+' fragmented files.'
		urls  = generate_Urls2(Error_list)
		counter = 0
		for (url, filename, start), error in pool.imap_unordered(download, urls):
			counter += 1
			update_progress(counter/(len(Error_list)*retmax/50))
		print 'Checking new files'
		Combine_files(Error_list)
			
	print 'Parsing, filtering and writing result to file:' 
	parse_filter_write_output()
	print 'Filtering data using USearch made by Robert Edgar'
	minseqlength = 0
	while minseqlength < 1:
		try:
			minseqlength = int(raw_input('Minimun sequence length cutoff: '))
		except Exception as e:
			minseqlength = 0
	pool = Pool(threads)
	program = directory[:-14]+'usearch'
	input_file = directory+'/all_query.fasta'
	output_file = directory+'/all_uniques.fasta'
	call(['chmod', '+x', program])
	call([program, '-derep_prefix', input_file, '-output', output_file , '-sizeout', '-minseqlength', str(minseqlength)])
	print 'Parsing the end result: '
	parse_end()
	print 'Database download has been completed.'
	print 'Final output is called Final Database'
	exit()
	
if __name__ == "__main__":
	main()