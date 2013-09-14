from glob import glob
## schema would be 
## participant, mode, target, correct_time, correct_type, first_miss_time, miss_count, switch_count, switch_target
class log:
  def __init__(self, participant, mode, target):
    self.participant = participant
    self.mode = mode
    self.target = target
    self.correct_type = ""
    self.correct_time = ""
    self.first_miss_time = ""
    self.miss_count = 0
    self.switch_count = 0
    self.switch_target = []
    self.wrong_count = 0
    self.wrong_target = []
    self.fate = []
  def __repr__(self):
    return self.participant + ',\t' +\
           self.mode + ',\t' +\
           self.target + ',\t' +\
           self.correct_time + ',\t' +\
           self.correct_type + ',\t' +\
           self.first_miss_time + ',\t' +\
           str(self.miss_count) + ',\t' +\
           str(self.switch_count) + ',\t' +\
           '[' + ';'.join(map(str, self.switch_target)) + '],\t' +\
           str(self.wrong_count) + ',\t' +\
           '[' + ';'.join(map(str, self.wrong_target)) + '],\t' +\
           '[' + ';'.join(map(str, self.fate)) + ']'

filelist = glob("../*.txt")
f1=open('./data.csv', 'w+')
print >>f1, "participant, mode, target, correct_time, correct_type, first_miss_time, miss_count, switch_count, switch_target, wrong_count, wrong_target, fate"
        
for f in filelist:
  logs = []
  new_log = None
  lines = [line.strip().split() for line in open(f)]
  participant = f.split('_')[1]
  mode = f.split('_')[2][:-4]
  for l in lines:
    if not l or l[1] == "msg":
      continue
    if l[1] == "start":
      if new_log is not None:
        print >>f1, str(new_log).expandtabs(4)
      new_log = log(participant, mode, l[2])
    if any(item in l[1] for item in ["reset", "disconnect", "next", "jump"]) and new_log is not None:
      new_log.fate.append(l[1])      
    if new_log is not None:
      ## then it's good to start monitoring keywords here
      if l[1].split('_')[0] == "correct":
        new_log.correct_type = l[1].split('_')[1]
        new_log.correct_time = l[0]
      if l[1].split('_')[0] == "miss":
        new_log.miss_count += 1
        if new_log.miss_count == 1:
          new_log.first_miss_time = l[0]
      if l[1].split('_')[0] == "switch":
        new_log.switch_count += 1
        new_log.switch_target.append(l[2])
      if l[1].split('_')[0] == "wrong":
        new_log.wrong_count += 1
        new_log.wrong_target.append(l[2])
      
        
