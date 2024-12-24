import sys

f = open(sys.argv[1], 'r')

lines = f.readlines()

preds = set()

for line in lines:
    lst = line.split()
    preds.add(lst[1])
    

for p in preds:
    print(p)
