/*
 * CandidateList.h
 *
 *  Created on: 2014-7-2
 *      Author: liyouhuan
 */
#include<iostream>
#include<vector>

#ifndef IDLIST_H_
#define IDLIST_H_

class IDList{
public:
	IDList();
	int getID(int _i)const;
	bool addID(int _id);

	/* check whether _id exists in this IDList. */
	bool isExistID(int _id)const;
	int size()const;
	const std::vector<int>* getList()const;
	int& operator[] (const int & _i);
	std::string to_str();
	int sort();
	void clear();

	/* intersect/union _id_list to this IDList, note that the two list must be ordered before using these two functions. */
	int intersectList(const int* _id_list, int _list_len);
	int intersectList(const IDList&);
	int unionList(const int* _id_list, int _list_len);
	int unionList(const IDList&);
	int bsearch_uporder(int _key);
// private:
	std::vector<int> id_list;
	int erase(int i);
};


#endif /* IDLIST_H_ */
