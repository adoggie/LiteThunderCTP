
from collections import OrderedDict

lines = open('instruments.txt').readlines()[1:]

instruments =OrderedDict()
for line in lines:
    line = line.strip()
    fs = line.split()
    instruments[fs[0].strip()] = (fs[1].strip(), fs[-1].strip())
comms = set()
for k,v in instruments.items():
    print(k, v[0],v[1])
    comms.add(v[1])
print(len(instruments))

print( list(comms) )
for comm in list(comms):
    
    insts = []
    for ins in instruments.keys():
        if instruments[ins][1] == comm:
            insts.append(ins)
    print(comm,'=', ','.join(insts))