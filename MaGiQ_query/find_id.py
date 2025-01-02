import sys

file_name = sys.argv[1]
str_to_find = sys.argv[2]

file = open(file_name, 'r')
lines = file.readlines()

pre_set = {}
sub_obj_set = {}

pre_cnt = 0
sub_obj_cnt = 0

num_edge = 0

for line in lines:
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

    if str_to_find == sub:
        print(sub_obj_set[str_to_find])
        break
    if str_to_find == obj:
        print(sub_obj_set[str_to_find])
        break