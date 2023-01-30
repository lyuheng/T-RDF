#ifndef KVSTORE_H_
#define KVSTORE_H_

#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stack>
#include "HashMap.h"
using namespace std;
class KVstore{
public:
	static const bool debug_mode = false;
	static const bool test = false;
	static const int READ_WRITE_MODE = 1;
	static const int CREATE_MODE = 2;

	const int array_init_value = 1000;

	/* include IN-neighbor & OUT-neighbor */
	int getEntityDegree(int _entity_id);
	int getEntityInDegree(int _entity_id);
	int getEntityOutDegree(int _entity_id);

	/* there are two situation when we need to update tuples list: s2o o2s sp2o op2s s2po o2ps
	 * 1. insert triple
	 * 2. remove triple
	 */
	int updateTupleslist_insert(int _sub_id, int _pre_id, int _obj_id);
	void updateTupleslist_remove(int _sub_id, int _pre_id, int _obj_id);
private:

	/* insert <_x_id, _y_id> into _xylist(keep _xylist(<x,y>) in ascending order) */
	bool insert_xy(int*& _xylist, int& _list_len,int _x_id, int _y_id);

	/* insert _x_id into _xlist(keep _xlist in ascending order) */
	bool insert_x(int*& _xlist, int& _list_len, int _x_id);

	/* remove _x_id from _xlist */
	bool remove_x(int*& _xlist, int& _list_len, int _x_id);

	/* remove <_x_id, _y_id> from _xylist */
	bool remove_xy(int*& _xylist, int& _list_len,int _x_id, int _y_id);

public:

	/* for entity2id */
	bool open_entity2id();
	int getIDByEntity(const string _entity);
	bool setIDByEntity(const string _entity, int _id);


	/* for id2entity */
	bool open_id2entity();
	string getEntityByID(int _id);
	bool setEntityByID(int _id, string _entity);

	/* for predicate2id */
	bool open_predicate2id();
	int getIDByPredicate(string _predicate);
	bool setIDByPredicate(string _predicate, int _id);

	/* for id2predicate */
	bool open_id2predicate();
	string getPredicateByID(int _id);
	bool setPredicateByID(int _id, string _predicate);

	/* for id2literal */
	bool open_id2literal();
	string getLiteralByID(int _id);
	bool setLiteralByID(int _id, string _literal);

	/* for literal2id */
	bool open_literal2id();
	int getIDByLiteral(string _literal);
	bool setIDByLiteral(string _literal, int _id);



	/* for subID 2 objIDlist */
	bool open_subid2objidlist();
	bool getobjIDlistBysubID(int _subid, int*& _objidlist, int& _list_len);
	bool setobjIDlistBysubID(int _subid, int* _objidlist, int _list_len);

	/* for objID 2 subIDlist */
	bool open_objid2subidlist();
	bool getsubIDlistByobjID(int _objid, int*& _subidlist, int& _list_len);
	bool setsubIDlistByobjID(int _objid, int* _subidlist, int _list_len);

	/* TODO: for preID 2 subIDlist */
	bool open_preid2subidlist();
	bool getsubIDlistBypreID(int _preid, int*& _subidlist, int& _list_len);
	bool setsubIDlistBypreID(int _preid, int* _subidlist, int _list_len);

	
	/* TODO: for preID 2 objIDlist */
	bool open_preid2objidlist();
	bool getobjIDlistBypreID(int _preid, int*& _objidlist, int& _list_len);
	bool setobjIDlistBypreID(int _preid, int* _objidlist, int _list_len);


	/* for subID&preID 2 objIDlist */
	bool open_subIDpreID2objIDlist();
	bool getobjIDlistBysubIDpreID(int _subid, int _preid, int*& _objidlist, int& _list_len);
	bool setobjIDlistBysubIDpreID(int _subid, int _preid, int* _objidlist, int _list_len);

	/* for objID&preID 2 subIDlist */
	bool open_objIDpreID2subIDlist();
	bool getsubIDlistByobjIDpreID(int _objid, int _preid, int*& _subidlist, int& _list_len);
	bool setsubIDlistByobjIDpreID(int _objid, int _preid, int* _subidlist, int _list_len);

	/* for subID 2  preID&objIDlist */
	bool open_subID2preIDobjIDlist();
	bool getpreIDobjIDlistBysubID(int _subid, int*& _preid_objidlist, int& _list_len);
	bool setpreIDobjIDlistBysubID(int _subid, int* _preid_objidlist, int _list_len);

	/* for objID 2 preID&subIDlist */
	bool open_objID2preIDsubIDlist();
	bool getpreIDsubIDlistByobjID(int _objid, int*& _preid_subidlist, int& _list_len);
	bool setpreIDsubIDlistByobjID(int _objid, int* _preid_subidlist, int _list_len);

	/*
	 * _store_path denotes where to store the data
	 */
	KVstore(string _store_path = ".");
	~KVstore();
	void flush();
	void release();
	void open();
	void open_query_mode();

public:

	string store_path;
	/*
	 *
	 * map entity to its id, and id to the entity
	 * s_entity2id is relative store file name
	 */
	SIMap* entity2id;
	SArray* id2entity;
	static string s_entity2id;
	static string s_id2entity;

	SIMap* predicate2id;
	SArray* id2predicate;
	static string s_predicate2id;
	static string s_id2predicate;

	SIMap* literal2id;
	SArray* id2literal;
	static string s_literal2id;
	static string s_id2literal;

	IVMap* subID2objIDlist;
	IVMap* objID2subIDlist;
	static string s_sID2oIDlist;
	static string s_oID2sIDlist;

	IVMap* preID2subIDlist;
	IVMap* preID2objIDlist;
	static string s_pID2sIDlist;
	static string s_pID2oIDlist;

	/* lack exist in update tuple */
	IIVMap* subIDpreID2objIDlist;
	IIVMap* objIDpreID2subIDlist;
	static string s_sIDpID2oIDlist;
	static string s_oIDpID2sIDlist;

	IVMap* subID2preIDobjIDlist;
	IVMap* objID2preIDsubIDlist;
	static string s_sID2pIDoIDlist;
	static string s_oID2pIDsIDlist;


	void flush(SIMap* _p_map);
	void flush(SArray* _p_array);
	void flush(Array2D* _p_array);
	void flush(IIVMap* _p_map);
	void flush(IVMap* _p_map);



	void release(SIMap *_p_map) {}
	void release(SArray* _p_array) {}
	void release(Array2D* _p_array) {}
	void release(IIVMap* _p_map) {}
	void release(IVMap* _p_map) {}

	bool setValueByKey(SIMap* _p_map, string &_key, int &_val);
	bool setValueByKey(SArray* _p_array, int &_key, string &_val);
	bool setValueByKey(Array2D* _p_array, int &_key, int* &_val, int &_vlen);
	bool setValueByKey(IIVMap* _p_map, pair<int,int> &_key, int* &_val, int &_vlen);
	bool setValueByKey(IVMap* _p_map, int &_key, int* &_val, int &_vlen);

	bool getValueByKey(SIMap* _p_map, string &_key, int &_val);
	bool getValueByKey(SArray* _p_array, int &_key, string &_val);
	bool getValueByKey(Array2D* _p_array, int &_key, int* &_val, int &_vlen);
	bool getValueByKey(IIVMap* _p_map, pair<int,int> &_key, int* &_val, int &_vlen);
	bool getValueByKey(IVMap* _p_map, int &_key, int* &_val, int &_vlen);



	// int getIDByStr(HashMap* _p_HashMap, const char* _key, int _klen);
	// bool removeKey(HashMap* _p_HashMap, const char* _key, int _klen);

	/* Open a HashMap according the mode */
	/* CREATE_MODE: 		build a new HashMap and delete if exist 	*/
	/* READ_WRITE_MODE: 	open a HashMap, HashMap must exist  		*/
	bool open(SIMap* & _p_map, const string _map_name);
	bool open(SArray * & _p_array, const string _array_name);
	bool open(Array2D * & _p_array, const string _array_name);
	bool open(IIVMap * & _p_map, const string _map_name);
	bool open(IVMap * & _p_map, const string _map_name);
	
};


#endif /* KVSTORE_H_ */
