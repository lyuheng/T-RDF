import sys

file_name = sys.argv[1]

file = open(file_name, 'r')
lines = file.readlines()

pre_set = {}
sub_obj_set = {}

pre_cnt = 0
sub_obj_cnt = 0

num_edge = 0

for line in lines:
    
    if line.startswith("@") or line.startswith("#"):
        continue
    lst = line.strip().split()

    if len(lst) < 3:
        continue

    sub = lst[0]
    pre = lst[1]
    obj = lst[2]
    if pre not in pre_set:
        pre_set[pre] = pre_cnt
        pre_cnt += 1
    if sub not in sub_obj_set:
        sub_obj_set[sub] = sub_obj_cnt
        sub_obj_cnt += 1
    if obj not in sub_obj_set:
        sub_obj_set[obj] = sub_obj_cnt
        sub_obj_cnt += 1
    num_edge += 1

out_name = sys.argv[2]
ofile = open(out_name, 'w')

ofile.write('{}\n'.format(str(num_edge)))
ofile.write('{}\n'.format(str(sub_obj_cnt)))

for line in lines:
    
    if line.startswith("@") or line.startswith("#"):
        continue
    lst = line.strip().split()
    if len(lst) < 3:
        continue
    sub = lst[0]
    pre = lst[1]
    obj = lst[2]
    ofile.write('<{}> <{}> <{}> .\n'.format(sub_obj_set[sub], pre_set[pre], sub_obj_set[obj]))

print('#nodes = ', sub_obj_cnt)
print('#edges = ', num_edge)

pred_dict = out_name + '.dict'
dfile = open(pred_dict, 'w')
for k in pre_set.keys():
    dfile.write('{} \t {}\n'.format(k, pre_set[k]))