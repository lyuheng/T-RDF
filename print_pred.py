import sys

f = open(sys.argv[1], 'r')

lines = f.readline()

preds = set()

for line in lines:
    lst = line.strip().split()

    print(lst[1])
    preds.add(lst[1])

    break
    
print(preds)
