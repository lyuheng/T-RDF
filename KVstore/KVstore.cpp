/*
 * KVstore.cpp
 *
 *  Created on: 2014-5-20
 *      Author: liyouhuan
 */
#include"KVstore.h"
#include"../Database/Database.h"
/* public methods: */

int KVstore::getEntityDegree(int _entity_id)
{
	int _in_degree = this->getEntityInDegree(_entity_id);
	int _out_degree = this->getEntityOutDegree(_entity_id);
	return _in_degree + _out_degree;
}

int KVstore::getEntityInDegree(int _entity_id)
{
	int* _olist = NULL;
	int _list_len = 0;
	this->getobjIDlistBysubID(_entity_id, _olist, _list_len);
	return _list_len;
}
int KVstore::getEntityOutDegree(int _entity_id)
{
	int* _slist = NULL;
	int _list_len = 0;
	this->getsubIDlistByobjID(_entity_id, _slist, _list_len);
	return _list_len;
}


/* there are two situation when we need to update tuples list: s2o o2s sp2o op2s s2po o2ps
 * 1. insert triple(finished in this function)
 * 2. remove triple
 * before call this function, we were sure that this triple did not exist
 */
int KVstore::updateTupleslist_insert(int _sub_id, int _pre_id, int _obj_id)
{
    //debug
//    {
//        stringstream _ss;
//        _ss << "updateTupleslist_insert: " << _sub_id << " " << _pre_id << " " << _obj_id << endl;
//        Database::log(_ss.str());
//    }

    //debug
    int updateListLen = 0;

    /* update sp2o */
    {
        int* _sp2olist = NULL;
        int _sp2o_len = 0;
        this->getobjIDlistBysubIDpreID(_sub_id, _pre_id, _sp2olist, _sp2o_len);

        /* if no duplication, _insert will be true
         * this->setXXX function will override the previous value */
        bool _insert = this->insert_x(_sp2olist, _sp2o_len, _obj_id);
        if(_insert){
            this->setobjIDlistBysubIDpreID(_sub_id, _pre_id, _sp2olist, _sp2o_len);
        }

        updateListLen += _sp2o_len;

        delete[] _sp2olist;
        _sp2olist = NULL;
        _sp2o_len = 0;

    }

    //debug
//  Database::log("update sp2o done.");

    /* update op2s */
    {
        int* _op2slist = NULL;
        int _op2s_len = 0;
        this->getsubIDlistByobjIDpreID(_obj_id, _pre_id, _op2slist, _op2s_len);

        /* if no duplication, _insert will be true
         * this->setXXX function will override the previous value */
        bool _insert = this->insert_x(_op2slist, _op2s_len, _sub_id);
        if(_insert){
            this->setsubIDlistByobjIDpreID(_obj_id, _pre_id, _op2slist, _op2s_len);
        }

        updateListLen += _op2s_len;

        delete[] _op2slist;
        _op2slist = NULL;
        _op2s_len = 0;
    }

    //debug
//  Database::log("update op2s done.");

    /* update s2po */
    {
        int* _s2polist = NULL;
        int _s2po_len = 0;
        this->getpreIDobjIDlistBysubID(_sub_id, _s2polist, _s2po_len);

        /* if no duplication, _insert will be true
         * this->setXXX function will override the previous value */
        bool _insert = this->insert_xy(_s2polist, _s2po_len, _pre_id, _obj_id);
        if(_insert){
            this->setpreIDobjIDlistBysubID(_sub_id, _s2polist, _s2po_len);
        }

        updateListLen += _s2po_len;

        delete[] _s2polist;
        _s2polist = NULL;
        _s2po_len = 0;
    }

    //debug
//  Database::log("update s2po done.");

    /* update o2ps */
    {
        int* _o2pslist = NULL;
        int _o2ps_len = 0;
        this->getpreIDsubIDlistByobjID(_obj_id, _o2pslist, _o2ps_len);

        /* if no duplication, _insert will be true
         * this->setXXX function will override the previous value */
        bool _insert = this->insert_xy(_o2pslist, _o2ps_len, _pre_id, _sub_id);
        if(_insert){
            this->setpreIDsubIDlistByobjID(_obj_id, _o2pslist, _o2ps_len);
        }

        updateListLen += _o2ps_len;

        delete[] _o2pslist;
        _o2pslist = NULL;
        _o2ps_len = 0;
    }

    //debug
//  Database::log("update o2ps done.");

    /* update s2o */
    {
        int* _s2olist = NULL;
        int _s2o_len = 0;
        this->getobjIDlistBysubID(_sub_id, _s2olist, _s2o_len);
        /* if no duplication, _insert will be true
         * this->setXXX function will override the previous value */
        bool _insert = this->insert_x(_s2olist, _s2o_len, _obj_id);
        if(_insert){
            this->setobjIDlistBysubID(_sub_id, _s2olist, _s2o_len);
        }

        updateListLen += _s2o_len;

        delete[] _s2olist;
        _s2olist = NULL;
        _s2o_len = 0;
    }

    //debug
//    Database::log("update s2o done.");

    /* update o2s */
    {
        int* _o2slist = NULL;
        int _o2s_len = 0;
        this->getsubIDlistByobjID(_obj_id, _o2slist, _o2s_len);

        /* if no duplication, _insert will be true
         * this->setXXX function will override the previous value */
        bool _insert = this->insert_x(_o2slist, _o2s_len, _sub_id);
        if(_insert){
            this->setsubIDlistByobjID(_obj_id, _o2slist, _o2s_len);
        }

        updateListLen += _o2s_len;

        delete[] _o2slist;
        _o2slist = NULL;
        _o2s_len = 0;
    }

    //debug
    return updateListLen;

    //debug
//   Database::log("update o2s done.");
}

/* insert <_x_id, _y_id> into _xylist(keep _xylist(<x,y>) in ascending order) */
bool KVstore::insert_xy(int*& _xylist, int& _list_len,int _x_id, int _y_id)
{

	/* check duplication */
	for(int i = 0; i < _list_len; i += 2)
	{
		if(_xylist[i] == _x_id &&
		   _xylist[i+1] == _y_id )
		{
			return false;
		}
	}

	int _new_list_len = _list_len + 2;
	int* _new_xylist = new int[_new_list_len];

	/* if _xylist does not exist */
	if(_xylist == NULL){
		_new_xylist[0] = _x_id;
		_new_xylist[1] = _y_id;
		_xylist = _new_xylist;
		_list_len = _new_list_len;
		return true;
	}

	bool _insert_head = (_xylist[0] > _x_id) || (_xylist[0] == _x_id && _xylist[1] > _y_id);
	if(_insert_head)
	{
		_new_xylist[0] = _x_id;
		_new_xylist[1] = _y_id;
		memcpy(_new_xylist + 2, _xylist, _list_len*(sizeof(int)));
		delete[] _xylist;
		_xylist = _new_xylist;
		_list_len = _new_list_len;
	}
	else
	{
		_new_xylist[0] = _xylist[0];
		_new_xylist[1] = _xylist[1];
		int j = 2;
		int i = 2;

		bool _insert_xyid = false;
		while(i < _list_len)
		{
			/* greater than previous pair */
			bool _gt_previous = (_xylist[i-2] < _x_id) || (_xylist[i-2]==_x_id && _xylist[i-1] < _y_id);
			/* less than current pair */
			bool _lt_current = (_x_id < _xylist[i]) || (_x_id==_xylist[i] && _y_id < _xylist[i+1]);

			_insert_xyid = _gt_previous && _lt_current;
			if(_insert_xyid)
			{
			    //insert the new pair.
				_new_xylist[j] = _x_id;
				_new_xylist[j+1] = _y_id;
				j += 2;
			}

			//copy the ith old pair to the new list.
			_new_xylist[j] = _xylist[i];
			_new_xylist[j+1] = _xylist[i+1];
			j += 2;
			i += 2;
		}

		bool _insert_tail = (j == _list_len);
		if(_insert_tail)
		{
			_new_xylist[j] = _x_id;
			_new_xylist[j+1] = _y_id;
			j += 2;
		}

		delete[] _xylist;
		_xylist = _new_xylist;
		_list_len = _new_list_len;
	}

	return true;
}

/* insert _x_id into _xlist(keep _xlist in ascending order) */
bool KVstore::insert_x(int*& _xlist, int& _list_len, int _x_id)
{


	/* check duplication */
	for(int i = 0; i < _list_len; i ++)
	{
		if(_xlist[i] == _x_id){
			return false;
		}
	}

	int _new_list_len = _list_len + 1;
	int* _new_xlist = new int[_new_list_len];

	/* if _xlist does not exist before */
	if(_xlist == NULL){
		_new_xlist[0] = _x_id;
		_xlist = _new_xlist;
		_list_len = _new_list_len;
		return true;
	}

	bool _insert_head = _x_id < _xlist[0];
	if(_insert_head)
	{
		_new_xlist[0] = _x_id;
		memcpy(_new_xlist+1, _xlist, _list_len*(sizeof(int)));
		delete[] _xlist;
		_xlist = _new_xlist;
		_list_len = _new_list_len;
	}
	else
	{
		_new_xlist[0] = _xlist[0];
		int j = 1;
		int i = 1;
		bool _insert_xid = false;
		while(i < _list_len)
		{
			_insert_xid = (_xlist[i-1] < _x_id) && (_x_id < _xlist[i]);

			//insert the new element.
			if(_insert_xid)
			{
				_new_xlist[j] = _x_id;
				j ++;
			}

			//copy the ith old element to the new list.
			_new_xlist[j] = _xlist[i];
			j ++;
			i ++;

		}
		bool _insert_tail = (j == _list_len);
		if(_insert_tail)
		{
			_new_xlist[j] = _x_id;
			j++;
		}

		delete[] _xlist;
		_xlist = _new_xlist;
		_list_len = _new_list_len;
	}
	return true;
}
/* there are two situation when we need to update tuples list: s2o o2s sp2o op2s s2po o2ps
 * 1. insert triple
 * 2. remove triple(finished in this function)
 * before call this function, we were sure that this triple did not exist
 */
void KVstore::updateTupleslist_remove(int _sub_id, int _pre_id, int _obj_id)
{

	cout << "!!!!!!!!!!!! @updateTupleslist_remove NOT GOOD ..................." << endl;
	/* update sp2o */
	{
		int* _sp2olist = NULL;
		int _sp2o_len = 0;
		this->getobjIDlistBysubIDpreID(_sub_id, _pre_id, _sp2olist, _sp2o_len);
		bool _remove = this->remove_x(_sp2olist, _sp2o_len, _obj_id);
		if(_sp2o_len == 0)
		{
			int* _sp = new int[2];
			_sp[0] = _sub_id;
			_sp[1] = _pre_id;
			// (this->removeKey(this->subID2preIDobjIDlist, (char*)_sp, sizeof(int)*2 ));
			delete[] _sp;
		}
		else
		if(_remove)
		{
			this->setobjIDlistBysubIDpreID(_sub_id, _pre_id, _sp2olist, _sp2o_len);
		}

		delete[] _sp2olist;
	}

	/* update op2s */
	{
		int* _op2slist = NULL;
		int _op2s_len = 0;
		this->getsubIDlistByobjIDpreID(_obj_id, _pre_id, _op2slist, _op2s_len);
		bool _remove = this->remove_x(_op2slist, _op2s_len, _sub_id);
		if(_op2s_len == 0)
		{
			int* _sp = new int[2];
			_sp[0] = _obj_id;
			_sp[1] = _pre_id;
			// this->removeKey(this->objIDpreID2subIDlist, (char*)_sp, sizeof(int)*2 );
			delete[] _sp;
		}
		else
		if(_remove)
		{
			this->setsubIDlistByobjIDpreID(_obj_id, _pre_id, _op2slist, _op2s_len);
		}

		delete[] _op2slist;
	}

	/* update s2po */
	{
		int* _s2polist = NULL;
		int _s2po_len = 0;
		this->getpreIDobjIDlistBysubID(_sub_id, _s2polist, _s2po_len);
		bool _remove = this->remove_xy(_s2polist, _s2po_len, _pre_id, _obj_id);
		if(_s2po_len == 0)
		{
			// this->removeKey(this->subID2preIDobjIDlist, (char*)&_sub_id, sizeof(int));
		}
		else
		if(_remove)
		{
			this->setpreIDobjIDlistBysubID(_sub_id, _s2polist, _s2po_len);
		}

		delete[] _s2polist;
	}

	/* update o2ps */
	{
		int* _o2pslist = NULL;
		int _o2ps_len = 0;
		this->getpreIDsubIDlistByobjID(_obj_id, _o2pslist, _o2ps_len);
		bool _remove = this->remove_xy(_o2pslist, _o2ps_len, _pre_id, _sub_id);
		if(_o2ps_len == 0)
		{
			// this->removeKey(this->objID2preIDsubIDlist, (char*)&_obj_id, sizeof(int));
		}
		else
		if(_remove)
		{
			this->setpreIDsubIDlistByobjID(_obj_id, _o2pslist, _o2ps_len);
		}

		delete[] _o2pslist;
	}

	/* update s2o */
	{
		int* _s2olist = NULL;
		int _s2o_len = 0;
		this->getobjIDlistBysubID(_sub_id, _s2olist, _s2o_len);
		bool _remove = this->remove_x(_s2olist, _s2o_len, _obj_id);
		if(_s2o_len == 0)
		{
			// this->removeKey(this->subID2objIDlist, (char*)&_sub_id, sizeof(int));
		}
		else
		if(_remove)
		{
			this->setobjIDlistBysubID(_sub_id, _s2olist, _s2o_len);
		}

		delete[] _s2olist;
	}

	/* update o2s */
	{
		int* _o2slist = NULL;
		int _o2s_len = 0;
		this->getsubIDlistByobjID(_obj_id, _o2slist, _o2s_len);
		bool _remove = this->remove_x(_o2slist, _o2s_len, _sub_id);
		if(_o2s_len == 0)
		{
			// this->removeKey(this->objID2subIDlist, (char*)&_obj_id, sizeof(int));
		}
		else
		if(_remove)
		{
			this->setsubIDlistByobjID(_obj_id, _o2slist, _o2s_len);
		}

		delete[] _o2slist;
	}

}

bool KVstore::remove_x(int*& _xlist, int& _list_len, int _x_id)
{
	for(int i = 0; i < _list_len; i ++)
	{
		if(_xlist[i] == _x_id)
		{
			/* move the latter ones backward */
			for(int j = i+1; j < _list_len; j ++)
			{
				_xlist[j-1] = _xlist[j];
			}
			_xlist[_list_len-1] = 0;
			_list_len --;
			return true;
		}
	}

	return false;
}
bool KVstore::remove_xy(int*& _xylist, int& _list_len,int _x_id, int _y_id)
{
	for(int i = 0; i < _list_len; i += 2)
	{
		bool _remove = (_xylist[i] == _x_id) && (_xylist[i+1] == _y_id);
		if(_remove)
		{
			/* move the latter pairs backward */
			for(int j = i+2; j < _list_len; j += 2)
			{
				_xylist[j-2] = _xylist[j];
				_xylist[j-1] = _xylist[j+1];
			}
			_xylist[_list_len-2] = 0;
			_xylist[_list_len-1] = 0;
			_list_len -= 2;
			return true;
		}
	}
	return false;
}

/*** for entity2id
 * _mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
 * ***/
bool KVstore::open_entity2id()
{
	return this->open(this->entity2id, KVstore::s_entity2id);
}
int KVstore::getIDByEntity(string _entity)
{
	int ret = -1;
	this->getValueByKey(this->entity2id, _entity, ret);
	return ret;
}
bool KVstore::setIDByEntity(string _entity, int _id)
{
	this->setValueByKey(this->entity2id, _entity, _id);
	return true;
}

/*** for id2entity
 * _mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
 * ***/
bool KVstore::open_id2entity()
{
	return this->open(this->id2entity, KVstore::s_id2entity);
}
string KVstore::getEntityByID(int _id)
{
	string ret;
	this->getValueByKey(this->id2entity, _id, ret);
	return ret;
}
bool KVstore::setEntityByID(int _id, string _entity)
{
	this->setValueByKey(this->id2entity, _id, _entity);
	return true;
}


/*** for predicate2id
 * _mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
 * ***/
bool KVstore::open_predicate2id()
{
	return this->open(this->predicate2id, KVstore::s_predicate2id);
}
int KVstore::getIDByPredicate(string _predicate)
{
	int ret = -1;
	this->getValueByKey(this->predicate2id, _predicate, ret);
	return ret;
}
bool KVstore::setIDByPredicate(string _predicate, int _id)
{
	this->setValueByKey(this->predicate2id, _predicate, _id);
	return true;
}

/*** for id2predicate
 * _mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
 * ***/
bool KVstore::open_id2predicate()
{
	return this->open(this->id2predicate, KVstore::s_id2predicate);
}
string KVstore::getPredicateByID(int _id)
{
	string ret;
	this->getValueByKey(this->id2predicate, _id, ret);
	return ret;
}
bool KVstore::setPredicateByID(int _id, string _predicate)
{
	this->setValueByKey(this->id2predicate, _id, _predicate);
	return true;
}


/*** for literal2id
 * _mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
 * ***/
bool KVstore::open_literal2id()
{
	return this->open(this->literal2id, KVstore::s_literal2id);
}
int KVstore::getIDByLiteral(string _literal)
{
	int ret = -1;
	this->getValueByKey(this->literal2id, _literal, ret);
	return ret;
}
bool KVstore::setIDByLiteral(string _literal, int _id)
{
	this->setValueByKey(this->literal2id, _literal, _id);
	return true;
}

/*** for id2literal
 * _mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
 * ***/
bool KVstore::open_id2literal()
{
	return this->open(this->id2literal, KVstore::s_id2literal);
}
string KVstore::getLiteralByID(int _id)
{
	int _new_id = _id - Database::LITERAL_FIRST_ID;
	string ret;
	this->getValueByKey(this->id2literal, _new_id, ret);
	return ret;
}
bool KVstore::setLiteralByID(int _id, string _literal)
{
	int _new_id = _id - Database::LITERAL_FIRST_ID;
	this->setValueByKey(this->id2literal, _new_id, _literal);
	return true;
}

/* for subID2objIDlist
 * _mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
 * ***/
bool KVstore::open_subid2objidlist()
{
	return this->open(this->subID2objIDlist, KVstore::s_sID2oIDlist);
}
bool KVstore::getobjIDlistBysubID(int _subid, int*& _objidlist, int& _list_len)
{
	bool ret = this->getValueByKey(this->subID2objIDlist, _subid, _objidlist, _list_len);
	if (ret) return true;
	else
	{
		_objidlist = NULL;
		_list_len = 0;
		return false;
	}
}
bool KVstore::setobjIDlistBysubID(int _subid, int* _objidlist, int _list_len)
{
	return this->setValueByKey(this->subID2objIDlist, _subid, _objidlist, _list_len);
}

/* for objID2subIDlist
 * _mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
 * ***/
bool KVstore::open_objid2subidlist()
{
	return this->open(this->objID2subIDlist, KVstore::s_oID2sIDlist);
}
bool KVstore::getsubIDlistByobjID(int _objid, int*& _subidlist, int& _list_len)
{
	bool ret = this->getValueByKey(this->objID2subIDlist, _objid, _subidlist, _list_len);
	if (ret) return true;
	else
	{
		_subidlist = NULL;
		_list_len = 0;
		return false;
	}
}
bool KVstore::setsubIDlistByobjID(int _objid, int* _subidlist, int _list_len)
{
	return this->setValueByKey(this->objID2subIDlist, _objid, _subidlist, _list_len);
}

/* for preID2subIDlist
 * _mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
 * ***/
bool KVstore::open_preid2subidlist()
{
	return this->open(this->preID2subIDlist, KVstore::s_pID2sIDlist);
}
bool KVstore::getsubIDlistBypreID(int _preid, int*& _subidlist, int& _list_len)
{
	bool ret = this->getValueByKey(this->preID2subIDlist, _preid, _subidlist, _list_len);
	if (ret) return true;
	else
	{
		_subidlist = NULL;
		_list_len = 0;
		return false;
	}
}
bool KVstore::setsubIDlistBypreID(int _preid, int* _subidlist, int _list_len)
{
	return this->setValueByKey(this->preID2subIDlist, _preid, _subidlist, _list_len);
}

/* for preID2subIDlist
 * _mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
 * ***/
bool KVstore::open_preid2objidlist()
{
	return this->open(this->preID2objIDlist, KVstore::s_pID2oIDlist);
}
bool KVstore::getobjIDlistBypreID(int _preid, int*& _objidlist, int& _list_len)
{
	bool ret = this->getValueByKey(this->preID2objIDlist, _preid, _objidlist, _list_len);
	if (ret) return true;
	else
	{
		_objidlist = NULL;
		_list_len = 0;
		return false;
	}
}
bool KVstore::setobjIDlistBypreID(int _preid, int* _objidlist, int _list_len)
{
	return this->setValueByKey(this->preID2objIDlist, _preid, _objidlist, _list_len);
}



/* for subID&preID2objIDlist
 * _mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
 * ***/
bool KVstore::open_subIDpreID2objIDlist()
{
	return this->open(this->subIDpreID2objIDlist, KVstore::s_sIDpID2oIDlist);
}

bool KVstore::getobjIDlistBysubIDpreID(int _subid, int _preid, int*& _objidlist, int& _list_len)
{
	pair<int, int> _key{_subid, _preid};
	bool ret = this->getValueByKey(this->subIDpreID2objIDlist, _key, _objidlist, _list_len);
	if (ret) return true;
	else
	{
		_objidlist = NULL;
		_list_len = 0;
		return false;
	}
}

bool KVstore::setobjIDlistBysubIDpreID(int _subid, int _preid, int* _objidlist, int _list_len)
{
	pair<int, int> _key{_subid, _preid};
	return this->setValueByKey(this->subIDpreID2objIDlist, _key, _objidlist, _list_len);
}


/* for objID&preID2subIDlist
 * _mode is either KVstore::CREATE_MODE or KVstore::READ_WRITE_MODE
 * ***/
bool KVstore::open_objIDpreID2subIDlist()
{
	return this->open(this->objIDpreID2subIDlist, KVstore::s_oIDpID2sIDlist);
}

bool KVstore::getsubIDlistByobjIDpreID(int _objid, int _preid, int*& _subidlist, int& _list_len)
{
	pair<int, int> _key{_objid, _preid};
	bool ret = this->getValueByKey(this->objIDpreID2subIDlist, _key, _subidlist, _list_len);
	if (ret) return true;
	else
	{
		_subidlist = NULL;
		_list_len = 0;
		return false;
	}
}

bool KVstore::setsubIDlistByobjIDpreID(int _objid, int _preid, int* _subidlist, int _list_len)
{
	pair<int, int> _key{_objid, _preid};
	return this->setValueByKey(this->objIDpreID2subIDlist, _key, _subidlist, _list_len);
}


/* for subID 2 preID&objIDlist */
bool KVstore::open_subID2preIDobjIDlist()
{
	return this->open(this->subID2preIDobjIDlist, KVstore::s_sID2pIDoIDlist);
}
bool KVstore::getpreIDobjIDlistBysubID(int _subid, int*& _preid_objidlist, int& _list_len)
{
	bool ret = this->getValueByKey(this->subID2preIDobjIDlist, _subid, _preid_objidlist, _list_len);
	if (ret) return true;
	else
	{
		_preid_objidlist = NULL;
		_list_len = 0;
		return false;
	}
}
bool KVstore::setpreIDobjIDlistBysubID(int _subid, int* _preid_objidlist, int _list_len)
{
	return this->setValueByKey(this->subID2preIDobjIDlist, _subid, _preid_objidlist, _list_len);
}

/* for objID 2 preID&subIDlist */
bool KVstore::open_objID2preIDsubIDlist()
{
	return this->open(this->objID2preIDsubIDlist, KVstore::s_oID2pIDsIDlist);
}
bool KVstore::getpreIDsubIDlistByobjID(int _objid, int*& _preid_subidlist, int& _list_len)
{
	bool ret = this->getValueByKey(this->objID2preIDsubIDlist, _objid, _preid_subidlist, _list_len);
	if (ret) return true;
	else
	{
		_preid_subidlist = NULL;
		_list_len = 0;
		return false;
	}
}
bool KVstore::setpreIDsubIDlistByobjID(int _objid, int* _preid_subidlist, int _list_len)
{
	return this->setValueByKey(this->objID2preIDsubIDlist, _objid, _preid_subidlist, _list_len);
}

/* set the store_path as the root dir of this KVstore
 * initial all Btree pointer as NULL
 *  */
KVstore::KVstore(const string _store_path){
	this->store_path = _store_path;


	this->entity2id = NULL;
	this->id2entity = NULL;

	this->predicate2id = NULL;
	this->id2predicate = NULL;

	this->literal2id = NULL;
	this->id2literal = NULL;

	this->objID2subIDlist = NULL;
	this->subID2objIDlist = NULL;

	this->objIDpreID2subIDlist = NULL;
	this->subIDpreID2objIDlist = NULL;

	this->subID2preIDobjIDlist = NULL;
	this->objID2preIDsubIDlist = NULL;
}

/* release all the memory used in this KVstore
 * before destruction
 *  */
KVstore::~KVstore(){
	this->release();

	delete this->entity2id;
	delete this->id2entity;

	delete this->literal2id;
	delete this->id2literal;

	delete this->predicate2id;
	delete this->id2predicate;

	delete this->objID2subIDlist;
	delete this->subID2objIDlist;

	delete this->objIDpreID2subIDlist;
	delete this->subIDpreID2objIDlist;

	delete this->subID2preIDobjIDlist;
	delete this->objID2preIDsubIDlist;
}

/*
 * just flush all modified part into disk
 * will not release any memory at all
 * any Btree pointer that is null or
 * has not been modified will do nothing
 *  */
void KVstore::flush(){
	this->flush(this->entity2id);
	this->flush(this->id2entity);

	this->flush(this->literal2id);
	this->flush(this->id2literal);

	this->flush(this->predicate2id);
	this->flush(this->id2predicate);

	this->flush(this->objID2subIDlist);
	this->flush(this->subID2objIDlist);

	this->flush(this->objIDpreID2subIDlist);
	this->flush(this->subIDpreID2objIDlist);

	this->flush(this->subID2preIDobjIDlist);
	this->flush(this->objID2preIDsubIDlist);
}

/* Release all the memory used in this KVstore,
 * following an flush() for each Btree pointer
 * any Btree pointer that is null or
 * has not been modified will do nothing
 *  */
void KVstore::release(){
	cout << "release of KVstore..." << endl;
	this->release(this->entity2id);
	this->release(this->id2entity);

	this->release(this->literal2id);
	this->release(this->id2literal);

	this->release(this->predicate2id);
	this->release(this->id2predicate);

	this->release(this->objID2subIDlist);
	this->release(this->subID2objIDlist);

	this->release(this->objIDpreID2subIDlist);
	this->release(this->subIDpreID2objIDlist);

	this->release(this->subID2preIDobjIDlist);
	this->release(this->objID2preIDsubIDlist);
}

void KVstore::open()
{
	cout << "open KVstore" << endl;

	this->open(this->entity2id, KVstore::s_entity2id);
	this->open(this->id2entity, KVstore::s_id2entity);

	this->open(this->literal2id, KVstore::s_literal2id);
	this->open(this->id2literal, KVstore::s_id2literal);

	this->open(this->predicate2id, KVstore::s_predicate2id);
	this->open(this->id2predicate, KVstore::s_id2predicate);

	this->open(this->objID2subIDlist, KVstore::s_oID2sIDlist);
	this->open(this->subID2objIDlist, KVstore::s_sID2oIDlist);

	this->open(this->objIDpreID2subIDlist, KVstore::s_oIDpID2sIDlist);
	this->open(this->subIDpreID2objIDlist, KVstore::s_sIDpID2oIDlist);

	this->open(this->subID2preIDobjIDlist, KVstore::s_sID2pIDoIDlist);
	this->open(this->objID2preIDsubIDlist, KVstore::s_oID2pIDsIDlist);
}

void KVstore::open_query_mode()
{

	cout << "open KVstore" << endl;

	this->open(this->entity2id, KVstore::s_entity2id);
	this->open(this->id2entity, KVstore::s_id2entity);

	this->open(this->literal2id, KVstore::s_literal2id);
	this->open(this->id2literal, KVstore::s_id2literal);

	this->open(this->predicate2id, KVstore::s_predicate2id);
	this->open(this->id2predicate, KVstore::s_id2predicate);

	this->open(this->objID2subIDlist, KVstore::s_oID2sIDlist);
	this->open(this->subID2objIDlist, KVstore::s_sID2oIDlist);

	this->open(this->objIDpreID2subIDlist, KVstore::s_oIDpID2sIDlist);
	this->open(this->subIDpreID2objIDlist, KVstore::s_sIDpID2oIDlist);

	this->open(this->subID2preIDobjIDlist, KVstore::s_sID2pIDoIDlist);
	this->open(this->objID2preIDsubIDlist, KVstore::s_oID2pIDsIDlist);

	cout << "================= Load Data on Disk =======================" << endl;

	this->entity2id->deserialize();
	this->id2entity->deserialize();
	this->literal2id->deserialize();
	this->id2literal->deserialize();
	this->predicate2id->deserialize();
	this->id2predicate->deserialize();
	this->objID2subIDlist->deserialize();
	this->subID2objIDlist->deserialize();
	this->objIDpreID2subIDlist->deserialize();
	this->subIDpreID2objIDlist->deserialize();
	this->subID2preIDobjIDlist->deserialize();
	this->objID2preIDsubIDlist->deserialize();

	cout << "================= Load Data on Disk DONE =======================" << endl;
}
/*
 * private methods:
 */
// void KVstore::flush(Btree* _p_btree){
// 	if(_p_btree != NULL)
// 	{
// 		_p_btree->flush();
// 	}
// }

void KVstore::flush(SIMap* _p_map)
{
	if (_p_map != NULL)
	{
		_p_map->serialize();
	}
}
void KVstore::flush(SArray* _p_array)
{
	if (_p_array != NULL)
	{
		_p_array->serialize();
	}
}
void KVstore::flush(Array2D* _p_array)
{
	if (_p_array != NULL)
	{
		_p_array->serialize();
	}
}
void KVstore::flush(IIVMap* _p_map)
{
	if (_p_map != NULL)
	{
		_p_map->serialize();
	}
}
void KVstore::flush(IVMap* _p_map)
{
	if (_p_map != NULL)
	{
		_p_map->serialize();
	}
}


/* Open a btree according the mode */
/* CREATE_MODE: 		build a new btree and delete if exist 	*/
/* READ_WRITE_MODE: 	open a btree, btree must exist  		*/
bool KVstore::open(SIMap* & _p_map, const string _map_name)
{
	_p_map = new SIMap(this->store_path, _map_name);
	return true;
}
bool KVstore::open(SArray * & _p_array, const string _array_name)
{
	_p_array = new SArray(this->store_path, _array_name);
	return true;
}
bool KVstore::open(Array2D * & _p_array, const string _array_name)
{
	_p_array = new Array2D(this->store_path, _array_name);
	return true;
}
bool KVstore::open(IIVMap * & _p_map, const string _map_name)
{
	_p_map = new IIVMap(this->store_path, _map_name);
	return true;
}
bool KVstore::open(IVMap * & _p_map, const string _map_name)
{
	_p_map = new IVMap(this->store_path, _map_name);
	return true;
}


bool KVstore::setValueByKey(SIMap* _p_map, string &_key, int &_val)
{
	_p_map->insert(_key, _val);
	return true;
}
bool KVstore::setValueByKey(SArray* _p_array, int &_key, string &_val)
{
	_p_array->insert(_key, _val);
	return true;
}
bool KVstore::setValueByKey(Array2D* _p_array, int &_key, int* &_val, int &_vlen)
{
	_p_array->insert(_key, _val, _vlen);
	return true;
}
bool KVstore::setValueByKey(IIVMap* _p_map, pair<int,int> &_key, int* &_val, int &_vlen)
{
	_p_map->insert(_key, _val, _vlen);
	return true;
}
bool KVstore::setValueByKey(IVMap* _p_map, int &_key, int* &_val, int &_vlen)
{
	_p_map->insert(_key, _val, _vlen);
	return true;
}


bool KVstore::getValueByKey(SIMap* _p_map, string &_key, int &_val)
{
	return _p_map->search(_key, _val);
}
bool KVstore::getValueByKey(SArray* _p_array, int &_key, string &_val)
{
	return _p_array->search(_key, _val);
}
bool KVstore::getValueByKey(Array2D* _p_array, int &_key, int* &_val, int &_vlen)
{
	return _p_array->search(_key, _val, _vlen);
}
bool KVstore::getValueByKey(IIVMap* _p_map, pair<int,int> &_key, int* &_val, int &_vlen)
{
	return _p_map->search(_key, _val, _vlen);
}
bool KVstore::getValueByKey(IVMap* _p_map, int &_key, int* &_val, int &_vlen)
{
	return _p_map->search(_key, _val, _vlen);
}






// int KVstore::getIDByStr(Btree* _p_btree, const char* _key, int _klen)
// {
// 	bool _ret = _p_btree->search(_key, _klen);
// 	if(!_ret){
// 		return -1;
// 	}
// 	/* int is stored in str
// 	 * forcely change str into int* and, get the int value with '*' */
// 	return *( (int*)( (_p_btree->getValueTransfer())->str ) );
// }

// bool KVstore::removeKey(Btree* _p_btree, const char* _key, int _klen)
// {
// 	return _p_btree->remove(_key, _klen);
// }

string KVstore::s_entity2id="s_entity2id";
string KVstore::s_id2entity="s_id2entity";

string KVstore::s_predicate2id="s_predicate2id";
string KVstore::s_id2predicate="s_id2predicate";

string KVstore::s_literal2id="s_literal2id";
string KVstore::s_id2literal="s_id2literal";

string KVstore::s_sID2oIDlist="s_sID2oIDlist";
string KVstore::s_oID2sIDlist="s_oID2sIDlist";

string KVstore::s_pID2sIDlist="s_pID2sIDlist";
string KVstore::s_pID2oIDlist="s_pID2oIDlist";

string KVstore::s_sIDpID2oIDlist="s_sIDpID2oIDlist";
string KVstore::s_oIDpID2sIDlist="s_oIDpID2sIDlist";

string KVstore::s_sID2pIDoIDlist="s_sID2pIDoIDlist";
string KVstore::s_oID2pIDsIDlist="s_oID2pIDsIDlist";
