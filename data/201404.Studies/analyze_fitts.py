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
  if not all(map(isfile, filenames)) or not isfile(directory + '/glass.txt'):
    print bcolors.FAIL + " file missing in " + directory + bcolors.ENDC
    exit(1)
    
  for f in filenames:
    with open(f) as f_handler:
      for line in f_handler:
        if line[0] != '[' and line[0] != ' ':
          print "file:", f, " formatting is wrong"
          exit(1)
  
  with open(directory + '/glass.txt') as f:
    for line in f:
      if line[0:10] != "E/GlasSees" and line[0:5] != "-----":
        print "file:", f, " formatting is wrong"
        break


### problem now with time synchronization
## consider doing it later
def merge(arduino_filename, glass_filename):
  logs = []
  start, finish = None, None
  with open(arduino_filename) as f:
    for line in f:
      m = re.search(r"\[ 2014-04", line)
      if m:
        time = datetime.strptime(line[2:28], '%Y-%m-%d %H:%M:%S %f')
        if not start:
          start = time
        finish = time
        event = line[40:]
        logs.append((time, event))

  print start, finish
  with open(glass_filename) as f:
    for line in f:
      m = re.search(r'time: (\d{13})', line)
      if m:
        time = datetime.fromtimestamp(int(m.group()[6:])/1000.0)
        if time > start and time < finish:
          event = line[39:]
          logs.append((time, event))

  logs.sort(key = lambda x: x[0])
  return logs

## this works and helps!!
def convert_glass_logs(glass_filename):
  with open(glass_filename) as f:
    for line in f:
      m = re.search(r'time: (\d{13})', line)
      if m:
        time = datetime.fromtimestamp(int(m.group()[6:])/1000.0)
        event = line[39:]
        print time, event,

def process_file(directory, filename, tag):
  target_id, target_time, search_time, acquire_time = None, None, [], None
  first_time = None
  tap_time, first_target = None, None
  miss_selection = 0
  with open(directory + '/' + filename) as f:
    for line in f:
      if not target_id:
        m = re.search(r'C(\d{2})', line)
        if m:
          ## get target id
          target_id = m.group()[1:3]
          target_time = datetime.strptime(line[2:28], '%Y-%m-%d %H:%M:%S %f')
          last_miss_selection_time = target_time
          
      if target_id and not tap_time:
        m = re.search(target_id + '[uid]', line)
        if m:
          search_time.append( (datetime.strptime(line[2:28], '%Y-%m-%d %H:%M:%S %f') - target_time).total_seconds() - 1 )

          
      if target_id and search_time and not tap_time and not acquire_time:
        m = re.search('H\d\dH\d\d', line)
        if m:
          tap_time = (datetime.strptime(line[2:28], '%Y-%m-%d %H:%M:%S %f') - target_time).total_seconds() - 1
          first_target = int(m.group()[1:3])

      if target_id and search_time and not acquire_time:
        m = re.search('C\d\d', line)
        if m:
          if m.group()[1:3] == target_id:
            ## acquiring the target
            acquire_time = (datetime.strptime(line[2:28], '%Y-%m-%d %H:%M:%S %f') - target_time).total_seconds() - 1
          else:
            miss_selection_time = datetime.strptime(line[2:28], '%Y-%m-%d %H:%M:%S %f')
            if ( (miss_selection_time - last_miss_selection_time).total_seconds() > 0.3):
              miss_selection += 1
            last_miss_selection_time = miss_selection_time

  # if target_time:
  #   ## checks glass logs
  #   with open(glass_filename) as f:
  #     for line in f:
  #       m = re.search(r'time: (\d{13})', line)
  #       if m:
  #         time_string = m.group()[6:]
  #         ts = datetime.fromtimestamp(int(time_string)/1000.0)
  #         ## cheap way of filtering, use ts > target_time
  #         if ts > target_time and ts < acquire_time and not tap_time and "Tapped to connect" in line:
  #           tap_time = ts

  #         if ts > target_time and ts < acquire_time and not disambiguation_list and "Room level" in line:
  #           m = re.search(r'\[.*\]', line)
  #           t = m.group()
  #           disambiguation_list = map(int, (t[1:-1].split(",")))
  #           disambiguation_number = len(disambiguation_list)
  #           first_guess = disambiguation_list[0]        

  if target_id == None:
    print bcolors.WARNING + " target not found" + bcolors.ENDC

  if not search_time:
    search_time = "NA"
  else:
    if len(search_time) > 1:
      new_search_time = None
      for i in range(len(search_time) - 1):
        if search_time[i+1] - search_time[i] < 0.35:
          new_search_time = search_time[i]
          break
      if new_search_time:
        search_time = new_search_time
      else:
        search_time = search_time[-1]          
    elif len(search_time) == 1:
      search_time = search_time[0]

    ## find the first time when there will be continous coverage
    # if len(search_time) > 1:
    #   for i in range(len(search_time) - 2):
    #     if search_time[i+1] - search_time[i] < 0.3 and \
    #        search_time[i+2] - search_time[i+1] < 0.3:
    #       search_time = search_time[i]
    # elif len(search_time) == 1:
    #   search_time = search_time[1]
    # elif len(search_time) == 0:
    #   search_time = search_time[0]

  if not acquire_time:
    acquire_time = "NA"
  if not tap_time:
    tap_time = "NA"
    first_guess = "NA"
  if not first_target:
    first_target = "NA"

    ##  print filename, target_id, search_time, tap_time, acquire_time, disambiguation_list, tag
  print directory, filename, target_id, first_target, search_time, tap_time, acquire_time, tag, miss_selection

if __name__ == "__main__":
  if action == "check":
    check_files(directory)

  ## this is no longer working probably
  if action == "simple_time":
    
    if isfile(directory + '/' + "intensity order"):
      intensity_order = range(1, 31)
      name_order = range(31, 61)
    else:
      name_order = range(1, 31)
      intensity_order = range(31, 61)

    filenames = map(str, intensity_order)
    for f in filenames:
      process_file(directory, f, "intensity")

    filenames = map(str, name_order)
    for f in filenames:
      process_file(directory, f, "name")

  if action == "convert":
    filename = directory + '/' + "glass.txt"
    if isfile(filename):
      convert_glass_logs(filename)
