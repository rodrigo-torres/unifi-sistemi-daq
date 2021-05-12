#!/usr/bin/python

# Analyses data from a CSV file outputted by convert.py and outputs a file ready for plotting
import sys, getopt
import numpy as np

def printusage():
  print('analyse.py -f <datafile>')
  
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
  
  print('analyse.py: Will analyse file "' + filename + '"')
  
  pos = filename.find('.csv')
  if (pos == -1):
    print('analyse.py: Not a CSV file')
    sys.exit(2)
  ofilename = filename[:pos] # Truncate .csv extension
  ofilename += "_means.csv"
  
  ifile = open(filename, "r")
  ofile = open(ofilename,"w")
  ofile.write("VSTEP,VOLT(V),V_ERR(V),CURR(A),I_ERR(A),R(V/I),R_ERR\n")
  
  ifile.readline() #Ignore the CSV header
  
  vstep_max = 0;
  vdict = {}
  
  # Parse the file two times, the first we try to identify the number of voltage steps
  while True:
    line = ifile.readline()
    if (line == ""):
      print("convert.py: First parse successful")
      break
    t = line.split(',')
    vstep = int(t[1])
    vdict[t[1]] = int(t[1])  #Assuming the nsteps start from 0 and increment by 1 with no jumps
    if (vstep > vstep_max):
      vstep_max = vstep
      
  ll_curr = [[] for i in range(vstep_max + 1)] # list of lists for the current
  ll_volt = [[] for i in range(vstep_max + 1)] # list of lists for the voltage
  
  # Go back to beginning and parse again
  ifile.seek(0)
  ifile.readline() # Ignore the CSV header
  
  while True:
    line = ifile.readline()
    if (line == ""):
      print("convert.py: First parse successful")
      break
    t = line.split(',')
    
    # Measured current values are at index 6
    # Append to a list of lists, at the index corresponding to the nstep,
    #  the value found for the measured current
    ll_curr[vdict[t[1]]].append(float(t[6]))
    # Same as above but for the voltage found at index 5
    ll_volt[vdict[t[1]]].append(float(t[5]))
    
  
  # Now we have the data arrange as a vector of vector of floats
  # Each entry in the 'outermost' vector contains all measurements (of current or voltage) for a given voltage step
  
  for key in vdict:
    v_step = vdict[key]
    i_mean = np.mean(ll_curr[v_step])
    i_stde = np.std(ll_curr[v_step]) / np.sqrt(len(ll_curr[v_step]))
    v_mean = np.mean(ll_volt[v_step])
    v_stde = np.std(ll_volt[v_step]) / np.sqrt(len(ll_volt[v_step]))
    r_ohm  = v_mean / i_mean
    r_err  = ((v_stde/v_mean) + (i_stde/i_mean)) * r_ohm
    
    line = f'{key},{v_mean:.6f},{v_stde:.6f},{i_mean:.6f},{i_stde:.6f},{r_ohm:.6f},{r_err:.6f}\n'
    ofile.write(line)
      
  ifile.close()
  ofile.close()
  
if __name__ == "__main__":
  main(sys.argv[1:])

