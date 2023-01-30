/*
 * database.h
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */

#ifndef DATABASE_H_
#define DATABASE_H_
#include<iostream>
#include<string.h>
using namespace std;

#include "../Query/IDList.h"
#include "../Query/ResultSet.h"
#include "../Query/SPARQLquery.h"
#include "../Query/BasicQuery.h"
#include "../Triple/Triple.h"
#include "../Signature/SigEntry.h"
#include "../KVstore/KVstore.h"
#include "../VSTree/VSTree.h"
#include "../Parser/DBparser.h"
#include "../Parser/RDFParser.h"
#include "../util/util.h"
#include<stdio.h>
#include<sys/time.h>
#include "fstream"

class Edges {

public:
    int* offset_;
    int* edge_;
    int vertex_count_;
    int edge_count_;
    int max_degree_;

    Edges() {
        offset_ = NULL;
        edge_ = NULL;
        vertex_count_ = 0;
        edge_count_ = 0;
        max_degree_ = 0;
    }

    ~Edges() {
        delete[] offset_;
        delete[] edge_;
    }
};


class Database{

public:

	static const bool debug_1 = false;
	static const bool debug_2 = false;
	static const bool only_sub2idpre2id = true;
	static const bool debug_vstree = false;
	static const int internal = 100*1000;
	static FILE* fp_debug;
	static void log(std::string _str);
	void test();
	void test_build_sig();
	void test_join();
	void printIDlist(int _i, int* _list, int _len, std::string _log);
	void printPairList(int _i, int* _list, int _len, std::string _log);

	static const int TRIPLE_NUM_MAX = 1000*1000*1000;

	/* In order to differentiate the sub-part and literal-part of object
	 * let subid begin with 0, while literalid begins with LITERAL_FIRST_ID */
	static const int LITERAL_FIRST_ID = 1000*1000*1000;

	/* when encode EntitySig, one way uses STRING-hash, the other one uses ID-hash
	 * depending on this->encode_mode  */
	static const int STRING_MODE = 1;
	static const int ID_MODE = 2;
	Database() {}
	Database(std::string _name);
	~Database();

	void preload(std::string _name);

	bool load();
	bool unload();
	bool query(const string _query, ResultSet& _result_set);

	/*
	 * 1. if subject of _triple doesn't exist,
	 *    then assign a new subid, and insert a new SigEntry
	 * 2. assign new tuple_id to tuple, if predicate or object doesn't exist before too;
	 * 3. if subject exist, update SigEntry, and update spo, ops... etc. if needed
	 * 4.
	 *  */

    bool insert(const string& _insert_rdf_file);
    bool remove(const string& _rdf_file);
    bool build(const string& _rdf_file);

	/* name of this DB*/
	string getName();
	/* root Path of this DB + sixTuplesFile */
	string getSixTuplesFile();

	/* root Path of this DB + signatureBFile */
	string getSignatureBFile();

    /* root Path of this DB + DBInfoFile */
    string getDBInfoFile();

// public:
	string name;
	bool is_active;
	int triples_num;
	int entity_num;
	int sub_num;
	int pre_num;
	int literal_num;

	int encode_mode;

	VSTree* vstree;
	KVstore* kvstore;

	SigEntry** _entity_bitset;

    /* metadata of this database: sub_num, pre_num, obj_num, literal_num, etc. */
    string db_info_file;

	/* six tuples: <sub pre obj sid pid oid> */
	string six_tuples_file;
	/* B means binary */
	string signature_binary_file;

    bool saveDBInfoFile();
    bool loadDBInfoFile();

	string getStorePath();

	/* encode relative signature data of all Basic Graph Query, who union together into SPARQLquery */
	void buildSparqlSignature(SPARQLquery & _sparql_q);

    /* encode Triple into Subject EntityBitSet */
    bool encodeTriple2SubEntityBitSet(EntityBitSet& _bitset, const Triple* _p_triple);
    /* encode Triple into Object EntityBitSet */
    bool encodeTriple2ObjEntityBitSet(EntityBitSet& _bitset, const Triple* _p_triple);

	bool calculateEntityBitSet(int _sub_id, EntityBitSet & _bitset);

	/* check whether the relative 3-tuples exist
	 * usually, through sp2olist */
	bool exist_triple(int _sub_id, int _pre_id, int _obj_id);

	/*
	 * _rdf_file denotes the path of the RDF file, where stores the rdf data
	 * there are many step in this function, each one responds to an sub-function
	 * 1. map sub2id and pre2id
	 * 2. map literal2id and encode RDF data into signature,
	 *    storing in binary file: this->getSignatureBFile(), the order responds to subID
	 *    also, store six_tuples in file: this->getSixTuplesFile()
	 * 3. build: subID2objIDlist, <subIDpreID>2objIDlist subID2<preIDobjID>list
	 * 4. build: objID2subIDlist, <objIDpreID>2subIDlist objID2<preIDsubID>list
	 *  */
	//encodeRDF_new invoke new rdfParser to solve task 1 & 2 in one time scan.
	bool encodeRDF(const string _rdf_file);
	bool encodeRDF_new(const string _rdf_file);

    int insertTriple(const TripleWithObjType& _triple);
    bool removeTriple(const TripleWithObjType& _triple);

	bool sub2id_pre2id_obj2id_RDFintoSignature(const string _rdf_file, int**& _p_id_tuples, int & _id_tuples_max);
	bool sub2id_pre2id(const string _rdf_file, int**& _p_id_tuples, int & _id_tuples_max);
	bool literal2id_RDFintoSignature(const string _rdf_file, int** _p_id_tuples, int _id_tuples_max);
	
	bool s2o_sp2o_s2po(int** _p_id_tuples, int _id_tuples_max);
	bool o2s_op2s_o2ps(int** _p_id_tuples, int _id_tuples_max);
	bool p2s_p2o(int** _p_id_tuples, int _id_tuples_max);

	int *preStatDict;
	int *IntersectDict;
	FILE *p2s_fp, *p2o_fp;
	int *p2s, *p2o;

	static int _spo_cmp(const void* _a, const void* _b);
	static int _ops_cmp(const void* _a, const void* _b);
	static int _pso_cmp(const void* _a, const void* _b);
	static int _pos_cmp(const void* _a, const void* _b);
	bool objIDIsEntityID(int _id);
	/*
	 * join on the vector of CandidateList, available after retrieve from the VSTREE
	 * and store the resut in _result_set
	 *  */
	bool join(SPARQLquery& _sparql_query);

	void filter_before_join(BasicQuery* basic_query);
	void literal_edge_filter(BasicQuery* basic_query, int _var_i);
	void preid_filter(BasicQuery* basic_query, int _var_i);
	void only_pre_filter_after_join(BasicQuery* basic_query);
	void add_literal_candidate(BasicQuery* basic_query);
	bool join_basic(BasicQuery* _basic_query);
	bool join(vector<int*>& _result_list, int _var_id, int _pre_id, int _var_id2, const char _edge_type,
	          int _var_num, bool shouldAddLiteral, IDList& _can_list);


	// ================================================================================================
	bool join_basic_dfs_style(BasicQuery* basic_query);

	void dfs(int cur_depth, bool *dealed_id_list, bool *dealed_triple, int *matching_order, int *result_var, int *idx_result_var, 
		vector<int*>* p_result_list, BasicQuery* basic_query, int **bn_idx, int *bn_count, int var_num, int entity_num, Edges ***edge_matrix);

	void generateBN(BasicQuery* basic_query, int *order, int **&bn_idx, int * &bn_count);
	void generateBN(BasicQuery* basic_query, int *order, int **&bn_idx, int * &bn_count, int pre_var_num);

	int selectGQLStartVertex(BasicQuery* basic_query);

	void updateValidVertices(BasicQuery* basic_query, int query_vertex, std::vector<bool> &visited,
                                    std::vector<bool> &adjacent);

	void buildTable(BasicQuery* basic_query, Edges ***edge_matrix, int var_num);
	// ================================================================================================

	bool select(vector<int*>& _result_list, int _var_id, int _pre_id, int _var_id2, const char _edge_type, int _var_num);

	void mapVarVec(vector<string> &a, vector<string> &b, vector<int> &mapvar, int &total_vars);
	void mergeJoin(SPARQLquery::TempResult &a, SPARQLquery::TempResult &b, SPARQLquery::TempResult &r);
	void mergeUnion(SPARQLquery::TempResult &a, SPARQLquery::TempResult &b, SPARQLquery::TempResult &r);
	void mergeLeftOuterJoin(SPARQLquery::TempResult &a, SPARQLquery::TempResult &b, SPARQLquery::TempResult &r);
	void doFilter(SPARQLquery::TempResult &a, SPARQLquery::FilterTree *ft, SPARQLquery::TempResult &r);
	void getFilterStr(string &str , vector<int> &r, map<string, int> &dict, string &ftarg);
	bool matchFilter(vector<int> &r,map<string, int> &dict, SPARQLquery::FilterTree *ft);

	int genEvaPlan(SPARQLquery::PatternGroup& pg, SPARQLquery& query, int id);
	void doEvaPlan(SPARQLquery &query);

	/* get the final string result_set from SPARQLquery */
	bool getFinalResult(SPARQLquery& _sparql_q, ResultSet& _result_set);
};



#endif /* DATABASE_H_ */
