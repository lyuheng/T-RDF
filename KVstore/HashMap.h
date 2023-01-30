#ifndef HASHMAP_H_
#define HASHMAP_H_

#include "ioser.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <ext/hash_map>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <assert.h>

using namespace std;


//@@: Checked on Nov 16
class SIMap
{
public:
	SIMap(const string &_store_path, const string &_file_name)
	{
		storePath = _store_path;
		fileName = _file_name;
	}
	// ~SIMap();
	void insert(const string &_key, const int &_val)
	{
		string2id[_key] = _val;
	}
	bool search(string &_key, int &_val) const
	{
		auto it = string2id.find(_key);
		if(it != string2id.end())
		{
			_val = it->second;
			return true;
		}
		else return false;
	}
	void serialize()
	{
		string file = getMapFilePath();
		ifbinstream out(file.c_str());

		out << string2id.size();

		for(auto it = string2id.begin(); it != string2id.end(); ++it)
		{
			out << it->first;
			out << it->second;
		}
	}
	void deserialize()
	{
		string file = getMapFilePath();
		ofbinstream in(file.c_str());
		
		size_t size;
		in >> size;
		for(int i = 0; i < size; ++i)
		{
			string key;
			in >> key;
			in >> string2id[key];
		}
	}
	string getMapFilePath()
	{
		return storePath+"/"+fileName;
	}

    void clear()
    {
        string2id.clear();
    }

	unordered_map<string, int> string2id;
	string storePath;
	string fileName;
};


inline void hash_combine(size_t &seed, int v)
{
    seed ^= v + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct IntVec
{
	int *vec;
	int len;

	IntVec(): vec(NULL), len(0)
	{}

	IntVec & operator = (const IntVec & iv)
	{
		vec = iv.vec;
		len = iv.len;
	}

	void copy(int *_vec, int _len)
	{
		if (_vec == NULL)
		{
			vec = NULL;
			len = 0;
			assert(false);
			return;
		}
		if (vec != NULL)
		{
			delete[] vec;
			len = 0;
			vec = NULL;
		}

		len = _len;
		vec = new int[len];
		memcpy(vec, _vec, sizeof(int)*len);
	}

	void copy(IntVec & val)
	{
		if (vec != NULL)
		{
			delete[] vec;
			vec = NULL;
		}

		len = val.len;
		vec = new int[len];
		memcpy(vec, val.vec, sizeof(int)*len);
	}
};



class IIVHashCode 
{
public:
    size_t operator()(pair<int, int> code) const
    {   
        size_t seed = 0;
        hash_combine(seed, code.first);
        hash_combine(seed, code.second);
        return seed;
    }
};


//@@: Checked on Nov 16
class IIVMap
{
public:
	IIVMap(const string &_store_path, const string &_file_name)
	{
		storePath = _store_path;
		fileName = _file_name;
	}
	~IIVMap()
	{
		for(auto it = pair2vec.begin(); it != pair2vec.end(); ++it)
		{
			if(it->second.vec != NULL) delete[] it->second.vec;
		}
	}
	void insert(const pair<int, int> &_key, int *&_val, const int &_val_len)
	{
		pair2vec[_key].copy(_val, _val_len);
	}
	bool search(pair<int, int> &_key, int * &_val, int &_val_len) const
	{
		auto it = pair2vec.find(_key);
		if(it != pair2vec.end())
		{
			_val_len = it->second.len;
			// _val = new int[_val_len];
			// memcpy(_val, it->second.vec, sizeof(int)*_val_len);
			_val = it->second.vec;
			return true;
		}
		else return false;
	}
	void serialize()
	{
		string file = getMapFilePath();
		ifbinstream out(file.c_str());

		out << pair2vec.size();
		for(auto it = pair2vec.begin(); it != pair2vec.end(); ++it)
		{
			out << it->first.first;
			out << it->first.second;
			int len = it->second.len; 
			out << len;
			for(int i=0; i<len; ++i)
			{
				out << it->second.vec[i];
			}
		}
	}
	void deserialize()
	{
		string file = getMapFilePath();
		ofbinstream in(file.c_str());
		
		size_t size;
		in >> size;
		for(int i = 0; i < size; ++i)
		{
			pair<int, int> key;
			in >> key.first;
			in >> key.second;
		
			IntVec val;
			in >> val.len;
			val.vec = new int[val.len];
			for(int j=0; j<val.len; ++j)
			{
				in >> val.vec[j];
			}
			pair2vec[key] = val;
		}
	}
	string getMapFilePath()
	{
		return storePath+"/"+fileName;
	}

    void clear()
    {
		for(auto it = pair2vec.begin(); it!=pair2vec.end(); ++it)
		{
			if(it->second.vec != NULL) delete[] it->second.vec;
		}
		pair2vec.clear();
    }

	unordered_map<pair<int,int>, IntVec, IIVHashCode> pair2vec;
	string storePath;
	string fileName;
};


// TODO: test
class IVMap
{
public:
	IVMap(const string &_store_path, const string &_file_name)
	{
		storePath = _store_path;
		fileName = _file_name;
	}
	~IVMap()
	{
		for(auto it = int2vec.begin(); it != int2vec.end(); ++it)
		{
			if(it->second.vec != NULL) delete[] it->second.vec;
		}
	}
	void insert(const int &_key, int *&_val, const int &_val_len)
	{
		int2vec[_key].copy(_val, _val_len);
	}
	bool search(int &_key, int * &_val, int &_val_len) const
	{
		auto it = int2vec.find(_key);
		if(it != int2vec.end())
		{
			_val_len = it->second.len;
			_val = new int[_val_len];
			memcpy(_val, it->second.vec, sizeof(int)*_val_len);
			return true;
		}
		else return false;
	}
	void serialize()
	{
		string file = getMapFilePath();
		ifbinstream out(file.c_str());

		out << int2vec.size();
		for(auto it = int2vec.begin(); it != int2vec.end(); ++it)
		{
			out << it->first;
			int len = it->second.len; 
			out << len;
			for(int i=0; i<len; ++i)
			{
				out << it->second.vec[i];
			}
		}
	}
	void deserialize()
	{
		string file = getMapFilePath();
		ofbinstream in(file.c_str());
		
		size_t size;
		in >> size;
		for(int i = 0; i < size; ++i)
		{
			int key;
			in >> key;
		
			IntVec val;
			in >> val.len;
			val.vec = new int[val.len];
			for(int j=0; j<val.len; ++j)
			{
				in >> val.vec[j];
			}
			int2vec[key] = val;
		}
	}
	string getMapFilePath()
	{
		return storePath+"/"+fileName;
	}

    void clear()
    {
		for(auto it = int2vec.begin(); it!=int2vec.end(); ++it)
		{
			if(it->second.vec != NULL) delete[] it->second.vec;
		}
		int2vec.clear();
    }

	unordered_map<int, IntVec> int2vec;
	string storePath;
	string fileName;
};

//@@: Checked on Nov 16
class SArray
{
public:
	SArray(const string& _store_path, const string& _file_name)
	{
		storePath = _store_path;
		fileName = _file_name;
	}
	// ~SArray();
	void insert(const int &_key, const string &_val)
	{
		if (_key == _array.size())
			_array.push_back(_val);
		else
			_array[_key] = _val;
	}
	bool search(int &_key, string &_val) const
	{
		if(_key >= 0 && _key < _array.size())
		{
			_val = _array[_key];
			return true;
		}
		else return false;
	}
	void serialize()
	{
		ifbinstream m(getBtreeFilePath().c_str());
		m << _array;
	}
	void deserialize()
	{	
		ofbinstream m(getBtreeFilePath().c_str());
		m >> _array;
	}

	string getBtreeFilePath()
	{
		return this->storePath+"/"+this->fileName;
	}

    void clear()
    {
        _array.clear();
    }

	vector<string> _array;
	string storePath;
	string fileName;
};

//@@: Check on Nov 16, ID has to be consecutive. Memory leak FIXED...
class Array2D
{
public:
	Array2D(const string& _store_path, const string& _file_name)
	{
		storePath = _store_path;
		fileName = _file_name;
		capacity = 1000;
		count = 0;

		_array = NULL;

		open();
	}
	void insert(const int &_key, int * &_val, const int &_val_len)
	{
		if (count >= capacity)
		{
			IntVec *_new_array = new IntVec[capacity * 2];
			for(int i=0; i<capacity; ++i)
			{
				_new_array[i].copy(_array[i]);
			}
			release();
			this->_array = _new_array;
			capacity *= 2;
		}

		// cout << "Key = " << _key << endl;
		_array[_key].copy(_val, _val_len);
		count += 1;
	}
	bool search(int &_key, int * &_val, int &_val_len) const
	{
		if(_key >= 0 && _key < count)
		{
			_val_len = _array[_key].len;
			_val = new int[_val_len];
			memcpy(_val, _array[_key].vec, sizeof(int)*_val_len);
			return true;
		}
		else return false;
	}
	void serialize()
	{
		ifbinstream m(getBtreeFilePath().c_str());

		m << count;
		for(int i=0; i<count; ++i)
		{
			int len = _array[i].len;
			m << len;
			for(int j=0; j<len; ++j)
			{
				m << _array[i].vec[j];
			}
		}
	}
	void deserialize()
	{	
		ofbinstream m(getBtreeFilePath().c_str());

		m >> count;
		this->_array = new IntVec[count];
		for(int i=0; i<count; ++i)
		{
			IntVec val;
			m >> val.len;
			val.vec = new int[val.len];
			for(int j=0; j<val.len; ++j)
			{
				m >> val.vec[j];
			}
			this->_array[i] = val;
		}
	}

	void open()
	{
		this->_array = new IntVec[this->capacity];
	}

	string getBtreeFilePath()
	{
		return this->storePath+"/"+this->fileName;
	}

	~Array2D()
	{
		for(int i = 0; i < count; ++i)
		{
			if(_array[i].vec != NULL) delete[] _array[i].vec;
		}
		delete[] _array;
	}

	void release()
	{
		for(int i = 0; i < count; ++i)
		{
			if (_array[i].vec != NULL) delete[] _array[i].vec;
		}
		delete[] _array;
	}

	IntVec* _array;
	int count;
	int capacity;
	string storePath;
	string fileName;
};


#endif /* HASHMAP_H_ */
