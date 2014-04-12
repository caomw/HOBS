#!/usr/bin/env python
## This file processes the fitts law data

from os import listdir
from os.path import isfile, join
from datetime import datetime
import re, argparse

parser = argparse.ArgumentParser(description='Fitts study analysis script')
parser.add_argument('-d', action='store', dest='dir_name', default=".", help='specify the directory location')
parser.add_argument('-a', action='store', dest='action', default="", help='actions to do: check')

arguments = parser.parse_args()
directory = arguments.dir_name
action = arguments.action


class bcolors:
  HEADER = '\033[95m'
  OKBLUE = '\033[94m'
  OKGREEN = '\033[92m'
  WARNING = '\033[93m'
  FAIL = '\033[91m'
  ENDC = '\033[0m'



def check_files(directory):
  ''' this function checks if the file logs in a specific directory have the right format'''
  filenames = map(lambda x: directory + '/' + str(x), range(1, 60))
  for f in filenames:
    with open(f) as f_handler:
      for line in f_handler:
        if line[0] != '[' and line[0] != ' ':
          print "file:", f, " formatting is wrong"
          break

def process_file(filename, tag):
  target_id, target_time, search_time, acquire_time = None, None, None, None
  with open(filename) as f:
    for line in f:
      if not target_id:
        m = re.search(r'C(\d{2})', line)
        if m:
          ## get target id
          target_id = m.group()[1:3]
          target_time = datetime.strptime(line[2:28], '%Y-%m-%d %H:%M:%S %f')

      if target_id and not search_time:
        m = re.search(target_id + '[uid]', line)
        if m:
          ## first time see the target being hovered
          tn = datetime.strptime(line[2:28], '%Y-%m-%d %H:%M:%S %f')
          search_time = (tn - target_time).total_seconds() - 1

      if target_id and search_time and not acquire_time:
        m = re.search('C' + target_id, line)
        if m:
          ## acquiring the target
          tn = datetime.strptime(line[2:28], '%Y-%m-%d %H:%M:%S %f')
          acquire_time = (tn - target_time).total_seconds() - 1

  if target_id == None:
    print bcolors.WARNING + " target not found" + bcolors.ENDC
    
  if not search_time:
    search_time = "NA"
  if not acquire_time:
    acquire_time = "NA"
    
  print filename, target_id, search_time, acquire_time, tag

if __name__ == "__main__":
  if action == "check":
    check_files(directory)
  
  if action == "simple_time":
    
    if isfile(directory + '/' + "intensity order"):
      intensity_order = range(1, 31)
      name_order = range(31, 61)
    else:
      name_order = range(1, 31)
      intensity_order = range(31, 61)

    filenames = map(lambda x: directory + '/' + str(x), intensity_order)
    for f in filenames:
      process_file(f, "intensity")

    filenames = map(lambda x: directory + '/' + str(x), name_order)
    for f in filenames:
      process_file(f, "name")
