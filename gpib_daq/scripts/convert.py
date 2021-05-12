#!/usr/bin/python

# Converts an output log file (from an older version of the gpib_daq program) to a CSV file

import sys, getopt
import numpy

def printusage():
  print('log_to_csv.py -f <file_to_convert>')
  
# Converts the line from from "log output" to CSV
def process_line(line):
  t = line.split()
  
  sep  = ','
  line = ''
  line = t[1] + sep + t[3] + sep + t[5]
  line = line[:-1] # Remove the last character which is a colon
  line += sep + t[6] + sep + t[8] + sep + t[10] + sep + t[12] + '\n'
 
  return line  

def main(argv):
  filename = ""
  try:
    opts, args = getopt.getopt(argv,"hf:")
  except getopt.GetoptError:
    printusage()
    sys.exit(2)
  
  for opt, arg in opts:
    if opt == '-f':
      filename = arg
  
  print('convert.py: Will convert file "' + filename + '" to CSV')
  
  ifile = open(filename, "r")
  ofile = open(filename + ".csv","w")
  ofile.write("NMEAS,NSTEP,NSAMPLE,TIMESTAMP(s),TIME_ERR(ms),VOLTAGE(V),CURRENT(A)\n")
  
  while True:
    line = ifile.readline()
    if (line == ""):
      print("Reached EOF")
      break
    # Ignore all lines containing warnings or debug messages
    if (line == '\n'):
      break
    if ((line.find('GPIB') == -1) and (line.find('Warning')) == -1) and (line.find('[') == -1):
      line = process_line(line)
      ofile.write(line)
      
  ifile.close()
  ofile.close()
  
if __name__ == "__main__":
  main(sys.argv[1:])

