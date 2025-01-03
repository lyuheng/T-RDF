#ifndef APP_RDF_H_
#define APP_RDF_H_

#include "../system/workerOL.h"
#include "../system/task.h"

#include "../Database/Database.h"
#include "../intersection/computesetintersection.h"
#include <string>
#include <sstream>
#include <fstream>
#include <sys/timeb.h> 
#include <utility>
#include "parallelsort.h"

#define TIME_THRESHOLD 100 // 10ms
#define MAX_QUERY_VERTEX 16 // restrict the max variable in query for simplicity


#define UNKNOWN -1000


Database db;
struct timeb gtime_start[32];

struct ConstEdge
{
    int lit_id, pre_id;
    char edge_type;
    ConstEdge() {}
    ConstEdge(int _lit_id, int _pre_id, char _edge_type):
                lit_id(_lit_id), pre_id(_pre_id), edge_type(_edge_type) {}

};

struct ContextValue
{
    int cur_depth; 
    int* result_var;

    int var_num;
    int pre_var_num;
};

ofbinstream & operator>>(ofbinstream & m, ContextValue & c)
{
    m >> c.cur_depth;
    m >> c.var_num;
    m >> c.pre_var_num;

    c.result_var = new int[c.var_num + c.pre_var_num + 1];
    for(int i=0; i < c.var_num + c.pre_var_num + 1; ++i)
    {
        m >> c.result_var[i];
    }
    return m;
}
ifbinstream & operator<<(ifbinstream & m, const ContextValue & c) 
{
    m << c.cur_depth;
    m << c.var_num;
    m << c.pre_var_num;

    for(int i=0; i<c.var_num + c.pre_var_num + 1; ++i)
    {
        m << c.result_var[i];
    }
    return m;
}

struct RDFQuery
{
    DBparser parser;
	SPARQLquery sparql_q;

    ResultSet rs;

    vector<int *> p_result_list[32]; // result collector


    int* matching_order;

    int **bn_idx, *bn_count; 

    vector<vector<ConstEdge> > constBN; // constant neighbor

    vector<vector<ConstEdge> > preConst;

    unordered_map<int, ConstEdge> edge_node_anchor; // record an anchor for each PU not connected by constant e.g. ?S ?PU ?O

    unordered_map<pair<int,int>, int, IIVHashCode> varIDvarID2preID; // first < second, varID includes constant

    struct timeb start_t, end_t;

    int max_cand_size;

    vector<int> global_task_num;

    RDFQuery(): max_cand_size(0), global_task_num(32, 0)
    {}
    ~RDFQuery()
    {
    }
};

typedef Task<ContextValue> RDFTask;


class RDFComper: public Comper<RDFTask, RDFQuery>
{
public:

    // IDList can_lists[MAX_QUERY_VERTEX];
    int idx[MAX_QUERY_VERTEX];
    int idx_count[MAX_QUERY_VERTEX];

    int **can_lists;
    int *can_counts;
    int *temp_buffer;
    int temp_count;
    bool *visited_arr;

    RDFComper(): can_lists(NULL), temp_buffer(NULL), can_counts(NULL), visited_arr(NULL)
    {}

    std::string getQueryFromFile(const char* _file_path)
    {
        char buf[10000];
        std::string query_file;

        ifstream fin(_file_path);
        if(!fin)
        {
            cout << "can not open: " << _file_path << endl;
            return "";
        }

        memset(buf, 0, sizeof(buf));
        stringstream _ss;
        while(!fin.eof())
        {
            fin.getline(buf, 9999);
            _ss << buf << "\n";
        }
        fin.close();

        return _ss.str();
    }

    virtual bool toQuery(string &line, RDFQuery &q)
    {

        ftime(&q.start_t);
        // try
        {
            q.parser.sparqlParser(getQueryFromFile(line.c_str()), q.sparql_q);
        }
        // catch(const char* e)
        // {
        //     fprintf(stderr, "%s\n", e);
        //     return false;
        // }
        return true;
    }

    virtual bool task_spawn(RDFQuery &q)
    {
        struct timeb start_t, end_t;
        double totaltime;
        q.sparql_q.encodeQuery(db.kvstore);

        // assume only 1 BasicQuery
        BasicQuery* basic_query;
        basic_query = &(q.sparql_q.getBasicQuery(0));

        /**
        cout << "========== Estimate Workload ==========" << endl;

        size_t estimate_workload = 0;
        for (int i=0; i<basic_query->getVarNum(); i++)
        {
            int var_degree = basic_query->getVarDegree(i);
            if (var_degree == 1) continue;

            set<int> in_edge_pre_id;
            set<int> out_edge_pre_id;
            for (int j = 0; j < var_degree; j++)
            {
                char edge_type = basic_query->getEdgeType(i, j);
                int triple_id = basic_query->getEdgeID(i, j);
                Triple triple = basic_query->getTriple(triple_id);
                string neighbor;
                if (edge_type == BasicQuery::EDGE_OUT)
                    neighbor = triple.object;
                else
                    neighbor = triple.subject;
                if(neighbor[0] != '?') continue;

                int pre_id = basic_query->getEdgePreID(i, j);
                if (pre_id < 0) continue;

                if (edge_type == BasicQuery::EDGE_OUT)
                {
                    out_edge_pre_id.insert(pre_id);
                }
                else
                {
                    in_edge_pre_id.insert(pre_id);
                }
            }
            for (auto it = in_edge_pre_id.begin(); it != in_edge_pre_id.end(); ++it)
                estimate_workload += db.preStatDict[4*(*it)+3];
            for (auto it = out_edge_pre_id.begin(); it != out_edge_pre_id.end(); ++it)
                estimate_workload += db.preStatDict[4*(*it)+1];
        }
        cout << "Estimate Workload: " << estimate_workload << endl;

        cout << "========== Estimate Workload Done ==========" << endl;
        */

        /*
        cout << "========== VSTree Pruning ==========" << endl;

        ftime(&start_t);
    
        // (db.vstree)->retrieve(q.sparql_q);

        vector<BasicQuery*>& queryList = q.sparql_q.getBasicQueryVec();
        // enumerate each BasicQuery and retrieve their variables' mapping entity in the VSTree.
        vector<BasicQuery*>::iterator iter=queryList.begin();
        for ( ;iter!=queryList.end();iter++)
        {
            int varNum = (*iter)->getVarNum();
            for (int i=0;i<varNum;i++)
            {
                int var_degree = basic_query->getVarDegree(i);
                if (var_degree == 1) continue;

                const EntityBitSet& entityBitSet = (*iter)->getVarBitSet(i);
                IDList* idListPtr = &( (*iter)->getCandidateList(i) );
                (db.vstree)->retrieveEntity_IM(entityBitSet, idListPtr);
            }
        }

        ftime(&end_t);
        totaltime = (end_t.time-start_t.time)*1000+(double)(end_t.millitm-start_t.millitm);
        cout << "VSTree Pruning (before sorting) takes " << totaltime << " ms." << endl;


        for(int i=0; i<basic_query->getVarNum(); i++)
        {
            cout << "Var " << i << " has " << basic_query->getCandidateList(i).size() << " candidates." << endl;
        }

        for (int i = 0; i < basic_query->getVarNum(); i++)
        {
            IDList &can_list = basic_query->getCandidateList(i);
            // can_list.sort();
            if (can_list.size() > 0)
                parasort(can_list.size(), can_list.id_list.data(), 16); // VSTree needs sorting
            db.literal_edge_filter(basic_query, i);
        }
 
        // db.add_literal_candidate(basic_query);

        for (int i = 0; i < basic_query->getVarNum(); i++)
        {
            if (!basic_query->isLiteralVariable(i))
            {
                basic_query->setAddedLiteralCandidate(i);
            }
        }
        ftime(&end_t);
        totaltime = (end_t.time-start_t.time)*1000+(double)(end_t.millitm-start_t.millitm);
        cout << "VSTree Pruning takes " << totaltime << " ms." << endl;

        cout << "========== VSTree Pruning Done ==========" << endl;

        cout << "========== Candidate clear ... ==========" << endl;

        for (int i = 0; i < basic_query->getVarNum(); i++)
        {
            basic_query->getCandidateList(i).clear();
        }

        cout << "========== Join(Disk) Pruning ========== " << endl;

        */

        ftime(&start_t);

        vector<bool> light_query_node(basic_query->getVarNum(), false);
        vector<int> approximate_cands(basic_query->getVarNum(), 0);

        int pre_var_num = 0;
        unordered_map<string, int> pre_var_map;
        
        int pre_var_id = -10; 
        int cur_pre_var_id;

        q.constBN.resize(basic_query->getVarNum());

        // Step 1. Do Filtering for light query node
        for (int i=0; i<basic_query->getVarNum(); i++)
        {
            int var_degree = basic_query->getVarDegree(i);
            for (int j = 0; j < var_degree; j++)
            {
                char edge_type = basic_query->getEdgeType(i, j);
                int triple_id = basic_query->getEdgeID(i, j);
                Triple triple = basic_query->getTriple(triple_id);
                string neighbor;
                if (edge_type == BasicQuery::EDGE_OUT)
                    neighbor = triple.object;
                else
                    neighbor = triple.subject;
                
                if(neighbor[0] == '?') continue;

                int lit_id = (db.kvstore)->getIDByEntity(neighbor);
                if (lit_id == -1) lit_id = (db.kvstore)->getIDByLiteral(neighbor);
                
                if (lit_id == -1)
                {
                    cout << "NO RESULT FOR THIS QUERY ..." << endl;
                    return true;
                }

                if (triple.predicate.rfind("<PU", 0) == 0) // PU stands for predicate unknown
                {
                    if (pre_var_map.find(triple.predicate) == pre_var_map.end())
                    {
                        cur_pre_var_id = pre_var_id--;
                        pre_var_map[triple.predicate] = cur_pre_var_id;
                        q.preConst.resize(q.preConst.size()+1);
                        q.preConst[pre_var_num].push_back(ConstEdge(lit_id, UNKNOWN, edge_type)); // only store const literal ID
                        pre_var_num++;
                    }
                    else
                    {
                        cur_pre_var_id = pre_var_map[triple.predicate];
                        q.preConst[pre_var_num].push_back(ConstEdge(lit_id, UNKNOWN, edge_type)); // only store const literal ID
                    }
                }

                int id_list_len = 0;
                int* id_list = NULL;


                int pre_id = basic_query->getEdgePreID(i, j);
                if (pre_id < 0)
                {
                    // 100 means constant variable for simplicity
                    q.varIDvarID2preID[make_pair(i, 100)] = cur_pre_var_id;
    
                    if (edge_type == BasicQuery::EDGE_OUT)
                        db.kvstore->getsubIDlistByobjID(lit_id, id_list, id_list_len);
                    else
                        db.kvstore->getobjIDlistBysubID(lit_id, id_list, id_list_len);

                    q.constBN[i].push_back(ConstEdge(lit_id, cur_pre_var_id, edge_type)); // TODO: FIXME: 
                    
                }
                else // pre_id >= 0
                {
                    if (edge_type == BasicQuery::EDGE_OUT)
                        db.kvstore->getsubIDlistByobjIDpreID(lit_id, pre_id, id_list, id_list_len);
                    else
                        db.kvstore->getobjIDlistBysubIDpreID(lit_id, pre_id, id_list, id_list_len);

                    q.constBN[i].push_back(ConstEdge(lit_id, pre_id, edge_type)); // TODO: FIXME: 
                }
                if (id_list_len == 0)
                {
                    cout << "NO RESULT FOR THIS QUERY ..." << endl;
                    return true;
                }

                if (approximate_cands[i] == 0)
                    approximate_cands[i] = id_list_len;
                else
                    approximate_cands[i] = approximate_cands[i] < id_list_len ? approximate_cands[i]:id_list_len;

                light_query_node[i] = true;
            }
        }

        // Step 2. Do Filtering for heavy query node
    
        for (int i=0; i<basic_query->getVarNum(); i++)
        {
            if (light_query_node[i]) continue;

            IDList& cans = basic_query->getCandidateList(i);

            int var_degree = basic_query->getVarDegree(i);
            // if (var_degree == 1) continue;


            set<int> in_edge_pre_id;
            set<int> out_edge_pre_id;
            for (int j = 0; j < var_degree; j++)
            {
                char edge_type = basic_query->getEdgeType(i, j);
                int triple_id = basic_query->getEdgeID(i, j);
                Triple triple = basic_query->getTriple(triple_id);
                string neighbor;
                if (edge_type == BasicQuery::EDGE_OUT)
                    neighbor = triple.object;
                else
                    neighbor = triple.subject;
                
                if(neighbor[0] != '?') continue;

                if (triple.predicate.rfind("<PU", 0) == 0) // PU stands for predicate unknown
                {
                    if (pre_var_map.find(triple.predicate) == pre_var_map.end())
                    {
                        cur_pre_var_id = pre_var_id --;
                        pre_var_map[triple.predicate] = cur_pre_var_id;
                        pre_var_num ++;
                    }
                    else
                        cur_pre_var_id = pre_var_map[triple.predicate];
                }
            
                int pre_id = basic_query->getEdgePreID(i, j);

                if (pre_id < 0)
                {
                    int neighbor_id = basic_query->var_str2id[neighbor];
                    if (neighbor_id < i)
                        q.varIDvarID2preID[make_pair(neighbor_id, i)] = cur_pre_var_id;
                    else
                        q.varIDvarID2preID[make_pair(i, neighbor_id)] = cur_pre_var_id;
                }
                else
                {
                    if (edge_type == BasicQuery::EDGE_OUT)
                    {
                        out_edge_pre_id.insert(pre_id);
                    }
                    else
                    {
                        in_edge_pre_id.insert(pre_id);
                    }
                }
            }
            if (in_edge_pre_id.empty() && out_edge_pre_id.empty()) continue;
            
            /** conduct intersection during phase 1, necessary?
            int* list = NULL;
	        int len = 0;
            for (auto it = in_edge_pre_id.begin(); it != in_edge_pre_id.end(); ++it)
            {
                // fseek(db.p2o_fp, sizeof(int)*db.preStatDict[4*(*it)+2], SEEK_SET);
                // len = db.preStatDict[4*(*it)+3];
                // list = new int[db.preStatDict[4*(*it)+3]];
                // fread(list, sizeof(int), len, db.p2o_fp);
                len = db.preStatDict[4*(*it)+3];
                list = db.p2o + db.preStatDict[4*(*it)+2];

                if(cans.size() == 0)
		            cans.unionList(list, len);
		        else
		            cans.intersectList(list, len);
                // delete[] list;
            }

            for (auto it = out_edge_pre_id.begin(); it != out_edge_pre_id.end(); ++it)
            {
                // fseek(db.p2s_fp, sizeof(int)*db.preStatDict[4*(*it)], SEEK_SET);
                // len = db.preStatDict[4*(*it)+1];
                // list = new int[db.preStatDict[4*(*it)+1]];
                // fread(list, sizeof(int), len, db.p2s_fp);

                len = db.preStatDict[4*(*it)+1];
                list = db.p2s + db.preStatDict[4*(*it)];

                if(cans.size() == 0)
		            cans.unionList(list, len);
		        else
		            cans.intersectList(list, len);
                // delete[] list;
            }
            //*/
            vector<int> approximate_candidate;
            // oo
            for (auto it = in_edge_pre_id.begin(); it != in_edge_pre_id.end(); ++it)
            {
                for (auto it2 = in_edge_pre_id.begin(); it2 != in_edge_pre_id.end(); ++it2)
                {
                    if (*it == *it2)
                    {
                        approximate_candidate.push_back(db.preStatDict[4*(*it)+3]);
                    }
                    else if (*it < *it2)
                    {
                        approximate_candidate.push_back(db.IntersectDict[(*it2)*db.pre_num+(*it)]);
                    }
                    else // *it > *it2
                    {
                        approximate_candidate.push_back(db.IntersectDict[(*it)*db.pre_num+(*it2)]);
                    }
                }
            }
            // ss
            for (auto it = out_edge_pre_id.begin(); it != out_edge_pre_id.end(); ++it)
            {
                for (auto it2 = out_edge_pre_id.begin(); it2 != out_edge_pre_id.end(); ++it2)
                {
                    if (*it == *it2) 
                    {
                        approximate_candidate.push_back(db.preStatDict[4*(*it)+1]);
                    }
                    else if (*it < *it2)
                    {
                        approximate_candidate.push_back(db.IntersectDict[(*it)*db.pre_num+(*it2)]);
                    }
                    else // *it > *it2
                    {
                        approximate_candidate.push_back(db.IntersectDict[(*it2)*db.pre_num+(*it)]);
                    }
                }
            }
            // so
            for (auto it = out_edge_pre_id.begin(); it != out_edge_pre_id.end(); ++it)
            {
                for (auto it2 = in_edge_pre_id.begin(); it2 != in_edge_pre_id.end(); ++it2)
                {
                    assert(*it != *it2);
                    approximate_candidate.push_back(db.IntersectDict[db.pre_num*db.pre_num + (*it)*db.pre_num+(*it2)]);
                }
            }

            int min_val = approximate_candidate[0];
            for (int k=1; k < approximate_candidate.size(); ++k)
            {
                min_val = min_val < approximate_candidate[k] ? min_val : approximate_candidate[k];
            }
            approximate_cands[i] = min_val;
        }

        ftime(&end_t);
        totaltime = (end_t.time-start_t.time)*1000+(double)(end_t.millitm-start_t.millitm);

        // FIXME: For debug, CHECK q.varIDvarID2preID 
        for (auto it = q.varIDvarID2preID.begin(); it != q.varIDvarID2preID.end(); ++it)
        {
            cout << "(" << it->first.first << ", " << it->first.second << "): " << it->second << endl;
        }

        cout << "Join(Disk) Pruning takes " << totaltime << " ms." << endl;

        // for(int i=0; i<basic_query->getVarNum(); i++)
        // {
        //     cout << "Var " << i << " has " << basic_query->getCandidateList(i).size() << " candidates." << endl;
        // }
        for(int i=0; i<basic_query->getVarNum(); i++)
        {
            if (approximate_cands[i] == 0)
            {
                approximate_cands[i] = db.entity_num + db.literal_num;
            }
            cout << "Var " << i << " has " << approximate_cands[i] << " candidates approximately." << endl;
        }
        
        cout << "========== Join(Disk) Pruning Done ========== " << endl;

        
        // int basic_query_num = q.sparql_q.getBasicQueryNum();

        // db.filter_before_join(basic_query);
        // for (int i = 0; i < basic_query->getVarNum(); i++)
        // {
        //     IDList &can_list = basic_query->getCandidateList(i);
        //     // can_list.sort();
        //     if (can_list.size() > 0)
        //         parasort(can_list.size(), can_list.id_list.data(), 16);
        //     db.literal_edge_filter(basic_query, i);
        // }
 
        // // db.add_literal_candidate(basic_query);

        // for (int i = 0; i < basic_query->getVarNum(); i++)
        // {
        //     if (!basic_query->isLiteralVariable(i))
        //     {
        //         basic_query->setAddedLiteralCandidate(i);
        //     }
        // }

        int var_num = basic_query->getVarNum();
	    int triple_num = basic_query->getTripleNum();

	    // initial p_result_list, push min_var_list in
        vector<int*>* p_result_list = &basic_query->getResultList();
        p_result_list->clear();

        q.matching_order = new int[var_num + pre_var_num];
        std::vector<bool> visited_vertices(var_num, false);
        std::vector<bool> adjacent_vertices(var_num, false);
          
        //set reason: don't add the same PU (e.g., <PU1> ... <PU1>) twice
        bool start_with_edge = false;
        unordered_set<int> already_in_order;
        pair<int, int> key;
        stack<int> var_stack;
        int cnt = 0;

        // first add valid predicate IDs

        for (auto it = q.varIDvarID2preID.begin(); it != q.varIDvarID2preID.end(); ++it)
        {
            if (it->first.second == 100 && already_in_order.find(it->second) == already_in_order.end() ) // means constant
            {
                q.matching_order[cnt ++] = it->second;
                already_in_order.insert(it->second);
                start_with_edge = true;

                // set the variable (other side) as adjacent (i.e., ensure connectivity)
                adjacent_vertices[it->first.first] = true;
            }
        }
        // generate matching order 

        // ================================== DFS order ===============================
        // var_stack.push(start_vertex);
        // while (!var_stack.empty())
        // {
        //     int var_id = var_stack.top();
        //     var_stack.pop();
        //     if (visited_vertices[var_id]) continue; // DEBUG: prevent duplicated vertices
        
        //     q.matching_order[cnt ++] = var_id;
        //     visited_vertices[var_id] = true;
        //     // before put var_id2 in matching order
        //     // check if there's any PU between var_id and those nodes 
        //     // that already in matching order
        //     // TODO:  
        //     for (int j = 0; j < var_num; ++j)
        //     { 
        //         if (visited_vertices[j]) continue;

        //         if (var_id < j)
        //             key = make_pair(var_id, j);
        //         else
        //             key = make_pair(j, var_id);
                
        //         if (q.varIDvarID2preID.find(key) != q.varIDvarID2preID.end())
        //         {
        //             int pre_id = q.varIDvarID2preID[key];
        //             if (already_in_order.find(pre_id) == already_in_order.end()) 
        //             {
        //                 q.matching_order[cnt ++] = pre_id;
        //                 already_in_order.insert(pre_id);
        //             }
        //         }
        //     }

        //     int var_degree = basic_query->getVarDegree(var_id);
        //     for (int i = 0; i < var_degree; i++)
        //     {
        //         int var_id2 = basic_query->getEdgeNeighborID(var_id, i);
        //         if (var_id2 == -1 || visited_vertices[var_id2])
        //         {
        //             continue;
        //         }

        //         var_stack.push(var_id2);
        //     }
        // }

        // ================================== DFS order ===============================

        // ============ Computing Starting Vertex Begins ===========

        // int start_vertex = basic_query->getVarID_FirstProcessWhenJoin();

        int start_vertex = -1;
        int start_size = Database::TRIPLE_NUM_MAX;

        if (start_with_edge) // TODO: test this!
        {
            for (int i = 0; i < var_num; i ++)
            {
                if (adjacent_vertices[i] && approximate_cands[i] != 0 && approximate_cands[i] < start_size)
                {
                    start_vertex = i;
                    start_size = approximate_cands[i]; 
                }
            }
            if (start_vertex == -1)
                start_vertex = 0;
        }
        else // else start with vertex
        {
            for(int i = 0; i < var_num; i ++)
            {
                if (approximate_cands[i] != 0 && approximate_cands[i] < start_size)
                // if (approximate_cands[i] < start_size)
                {
                    start_vertex = i;
                    start_size = approximate_cands[i];
                }
            }
            if (start_vertex == -1)
                start_vertex = 0;
        }

        // ============ Computing Starting Vertex Ends ===========

        // ================================== GraphQL order ============================
        
        q.matching_order[cnt ++] = start_vertex;
        visited_vertices[start_vertex] = true;
        int var_degree = basic_query->getVarDegree(start_vertex);
        for (int i = 0; i < var_degree; i++)
        {
            int var_id2 = basic_query->getEdgeNeighborID(start_vertex, i);
            if (var_id2 == -1 || visited_vertices[var_id2])
                continue;
            adjacent_vertices[var_id2] = true;
        }
        for (int j = 0; j < var_num; ++j)
        {
            if (visited_vertices[j]) continue;

            if (start_vertex < j)
                key = make_pair(start_vertex, j);
            else
                key = make_pair(j, start_vertex);
            
            if (q.varIDvarID2preID.find(key) != q.varIDvarID2preID.end())
            {
                int pre_id = q.varIDvarID2preID[key];
                if (already_in_order.find(pre_id) == already_in_order.end()) 
                {
                    q.matching_order[cnt ++] = pre_id;
                    already_in_order.insert(pre_id);

                    q.edge_node_anchor[pre_id].lit_id = start_vertex;
                }
            }
        }

        for (int i = 0; i < var_num; ++i)
        {   
            if (i == start_vertex) continue;
            int next_vertex;
            int min_value = db.entity_num + db.literal_num + 100;
            for (int j = 0; j < var_num; ++j)
            {
                if (!visited_vertices[j] && adjacent_vertices[j])
                {
                    if (approximate_cands[j] < min_value)
                    {
                        min_value = approximate_cands[j];
                        next_vertex = j;
                    }
                }
            }

            q.matching_order[cnt ++] = next_vertex;
            visited_vertices[next_vertex] = true;

            int var_degree = basic_query->getVarDegree(next_vertex);
            for (int i = 0; i < var_degree; i++)
            {
                int var_id2 = basic_query->getEdgeNeighborID(next_vertex, i);
                if (var_id2 == -1 || visited_vertices[var_id2])
                    continue;
                adjacent_vertices[var_id2] = true;
            }
            for (int j = 0; j < var_num; ++j)
            {
                if (visited_vertices[j]) continue;

                if (next_vertex < j)
                    key = make_pair(next_vertex, j);
                else
                    key = make_pair(j, next_vertex);
                
                if (q.varIDvarID2preID.find(key) != q.varIDvarID2preID.end())
                {
                    int pre_id = q.varIDvarID2preID[key];
                    if (already_in_order.find(pre_id) == already_in_order.end()) 
                    {
                        q.matching_order[cnt ++] = pre_id;
                        already_in_order.insert(pre_id);

                        q.edge_node_anchor[pre_id].lit_id = next_vertex;
                    }
                }
            }
        }

        for (int i = 0; i<basic_query->getVarNum(); i++)
        {
            int var_degree = basic_query->getVarDegree(i);
            for (int j = 0; j < var_degree; j++)
            {
                char edge_type = basic_query->getEdgeType(i, j);
                int triple_id = basic_query->getEdgeID(i, j);
                Triple triple = basic_query->getTriple(triple_id);

                if (pre_var_map.find(triple.predicate) != pre_var_map.end())
                {
                    int pre_id = pre_var_map[triple.predicate];
                    
                    if (q.edge_node_anchor.find(pre_id) != q.edge_node_anchor.end())
                    {
                        int anchor_id = q.edge_node_anchor[pre_id].lit_id;

                        if (anchor_id == i)
                        {
                            q.edge_node_anchor[pre_id].edge_type = edge_type;
                        }
                    }
                }     
            }
        }

        cout << "q.edge_node_anchor: " <<endl;
        for (auto it = q.edge_node_anchor.begin(); it != q.edge_node_anchor.end(); ++it)
        {
            cout << it->first << ": (" << it->second.lit_id << ", " << it->second.edge_type << ")" << endl;
        }

        // ================================== GraphQL order ============================

        cout << "Matching Order: ";
        for(int i=0; i<var_num+pre_var_num; ++i)
        {
            cout << q.matching_order[i] << " ";
        }
        cout << endl;

        db.generateBN(basic_query, q.matching_order, q.bn_idx, q.bn_count, pre_var_num); 
        cout << "BN: " << endl;
        for(int i=0; i<var_num+pre_var_num; ++i)
        {
            if (q.bn_count[i] == 0) cout << "EMPTY";
            for(int j=0; j<q.bn_count[i]; ++j)
            {
                cout << q.bn_idx[i][j]  << " ";
            }
            cout << endl;
        }
        cout << "q.preConst: " << endl;
        // for (int i=0; i < pre_var_num; ++i)
        // {
        //     for(int j=0; j<q.preConst[i].size(); ++j)
        //     {
        //         cout << q.preConst[i][j].lit_id << " ";
        //     }
        //     cout << endl;
        // } 

        // compute candidates of start_vertex
        if (!start_with_edge)
        {
            cout << "Computing candidates of start vertex " << start_vertex << " ..." << endl;
            IDList& cans = basic_query->getCandidateList(start_vertex);
            int var_degree = basic_query->getVarDegree(start_vertex);

            bool has_const_neighbor = false;

            for (int j = 0; j < q.constBN[start_vertex].size(); ++j)
            {
                int lit_id = q.constBN[start_vertex][j].lit_id;
                int pre_id = q.constBN[start_vertex][j].pre_id; // TODO: FIXME: 
                char edge_type = q.constBN[start_vertex][j].edge_type;

                int len = 0;
                int *list = NULL;

                if (pre_id < 0)
                {
                    if (edge_type == BasicQuery::EDGE_OUT)
                        db.kvstore->getsubIDlistByobjID(lit_id, list, len);
                    else 
                        db.kvstore->getobjIDlistBysubID(lit_id, list, len);
                }
                else
                {
                    if (edge_type == BasicQuery::EDGE_OUT)
                        db.kvstore->getsubIDlistByobjIDpreID(lit_id, pre_id, list, len);
                    else
                        db.kvstore->getobjIDlistBysubIDpreID(lit_id, pre_id, list, len);
                }
                
                if(cans.size() == 0)
                    cans.unionList(list, len);
                else
                    cans.intersectList(list, len);

                has_const_neighbor = true;
            }
            
            if (!has_const_neighbor)
            {
                set<int> in_edge_pre_id;
                set<int> out_edge_pre_id;
                for (int j = 0; j < var_degree; j++)
                {
                    char edge_type = basic_query->getEdgeType(start_vertex, j);
                    int triple_id = basic_query->getEdgeID(start_vertex, j);
                    Triple triple = basic_query->getTriple(triple_id);
                    string neighbor;
                    if (edge_type == BasicQuery::EDGE_OUT)
                        neighbor = triple.object;
                    else
                        neighbor = triple.subject;
                    
                    if(neighbor[0] != '?') continue;
                
                    int pre_id = basic_query->getEdgePreID(start_vertex, j);
                    if (pre_id < 0) continue;

                    if (edge_type == BasicQuery::EDGE_OUT)
                    {
                        out_edge_pre_id.insert(pre_id);
                    }
                    else
                    {
                        in_edge_pre_id.insert(pre_id);
                    }
                }
                int *list = NULL;
                int len = 0;
                for (auto it = in_edge_pre_id.begin(); it != in_edge_pre_id.end(); ++it)
                {
                    len = db.preStatDict[4*(*it)+3];
                    list = db.p2o + db.preStatDict[4*(*it)+2];

                    if(cans.size() == 0)
                        cans.unionList(list, len);
                    else
                        cans.intersectList(list, len);
                }

                for (auto it = out_edge_pre_id.begin(); it != out_edge_pre_id.end(); ++it)
                {
                    len = db.preStatDict[4*(*it)+1];
                    list = db.p2s + db.preStatDict[4*(*it)];

                    if(cans.size() == 0)
                        cans.unionList(list, len);
                    else
                        cans.intersectList(list, len);
                }
            }
            cout << "Computing candidates of start vertex has " << cans.size() << " candidates" << endl;  
        }

        // for(int i=0; i<var_num; ++i)
        // {
        //     q.max_cand_size = std::max(q.max_cand_size, basic_query->getCandidateSize(i));
        // }

        RDFTask *t = new RDFTask();
        t->context.cur_depth = 0;
        t->context.var_num = var_num; 
        t->context.pre_var_num = pre_var_num;
        t->context.result_var = new int[var_num + pre_var_num + 1];
        memset(t->context.result_var, 0, sizeof(int) * (var_num + pre_var_num + 1));

        add_task(t);

        ftime(&q.end_t);
        totaltime = (q.end_t.time-q.start_t.time)*1000+(double)(q.end_t.millitm-q.start_t.millitm);
        cout<<"Query "<<get_queryID()<<" till spawn time: "<<totaltime << " ms" <<endl;

        return true;
    }

    /** return elapsed time in milliseconds */
    double countElaspedTime()
    {
        struct timeb cur_time;
        ftime(&cur_time);
        return (cur_time.time-gtime_start[thread_id].time)*1000+(double)(cur_time.millitm-gtime_start[thread_id].millitm);
    }   
    /**
    // Iterative-style DFS with simple intersection
    void dfs_iterative_style(int enter_depth, int *result_var, int var_num, vector<int *> &p_result_list, BasicQuery *basic_query, RDFQuery &q)
    {
        int cur_depth = enter_depth;
        if(cur_depth == 0)
        {
            int var_id = q.matching_order[0];
            idx[0] = 0;
            idx_count[0] = basic_query->getCandidateSize(var_id);
            IDList can_list = basic_query->getCandidateList(var_id); // return reference
            for(int i=0; i<idx_count[0]; ++i)
            {
                can_lists[0].addID(can_list[i]);
            }
        }
        else
        {
            int var_id = q.matching_order[cur_depth];
            can_lists[cur_depth].clear();
            //do intersection
            for (int i = 0; i < q.bn_count[cur_depth]; ++i)
            {
                int var_id2_idx = q.bn_idx[cur_depth][i];
                int edge_id = basic_query->getEdgeID(var_id, var_id2_idx);
                int var_id2 = basic_query->getEdgeNeighborID(var_id, var_id2_idx); // BN
                if (var_id2 == -1)
                {
                    continue;
                }
                int pre_id = basic_query->getEdgePreID(var_id, var_id2_idx);
                char edge_type = basic_query->getEdgeType(var_id, var_id2_idx);
                
                // do intersection by BN 
                int* id_list;
                int id_list_len;
                if (edge_type == BasicQuery::EDGE_OUT)
                {
                    db.kvstore->getsubIDlistByobjIDpreID(result_var[var_id2],
                            pre_id, id_list, id_list_len);
                }
                else // EDGE_IN
                {
                    db.kvstore->getobjIDlistBysubIDpreID(result_var[var_id2],
                            pre_id, id_list, id_list_len);
                }
                
                if (i == 0)
                {
                    can_lists[cur_depth].unionList(id_list, id_list_len);
                }
                else
                {
                    can_lists[cur_depth].intersectList(id_list, id_list_len);
                }
            }
            idx[cur_depth] = 0;
            idx_count[cur_depth] = can_lists[cur_depth].size();
        }

        while (true)
        {
            while (idx[cur_depth] < idx_count[cur_depth])
            {
                int var_id = q.matching_order[cur_depth];
                // IDList can_list = basic_query->getCandidateList(var_id);
                int v = can_lists[cur_depth][idx[cur_depth]];
                // bool shouldVar2AddLiteralCandidateWhenJoin = basic_query->isFreeLiteralVariable(var_id) && !basic_query->isAddedLiteralCandidate(var_id);    
                // bool found_in_id_list = can_list.bsearch_uporder(v) >= 0;

                idx[cur_depth] += 1;

                //@@: If shouldAddLiteral = true, this variable is a free literal
                // bool should_add_this_literal = shouldVar2AddLiteralCandidateWhenJoin && !db.objIDIsEntityID(v);

                // if (found_in_id_list || should_add_this_literal)
                {
                    result_var[var_id] = v;
                    if (cur_depth == var_num - 1)
                    {
                        // int* result_var_cp = new int[var_num + 1];
                        // memcpy(result_var_cp, result_var, sizeof(int)*(var_num + 1));
                        // p_result_list.push_back(result_var_cp);
                        continue;
                    }
                

                    if (countElaspedTime() < TIME_THRESHOLD)
                    {
                        cur_depth += 1;
                        idx[cur_depth] = 0;

                        int var_id = q.matching_order[cur_depth];
                        can_lists[cur_depth].clear();
                        //do intersection
                        for (int i = 0; i < q.bn_count[cur_depth]; ++i)
                        {
                            int var_id2_idx = q.bn_idx[cur_depth][i];
                            int edge_id = basic_query->getEdgeID(var_id, var_id2_idx);
                            int var_id2 = basic_query->getEdgeNeighborID(var_id, var_id2_idx); // BN
                            if (var_id2 == -1)
                            {
                                continue;
                            }
                            int pre_id = basic_query->getEdgePreID(var_id, var_id2_idx);
                            char edge_type = basic_query->getEdgeType(var_id, var_id2_idx);
                            
                            // do intersection by BN 
                            int* id_list;
                            int id_list_len;
                            if (edge_type == BasicQuery::EDGE_OUT)
                            {
                                db.kvstore->getsubIDlistByobjIDpreID(result_var[var_id2],
                                        pre_id, id_list, id_list_len);
                            }
                            else // EDGE_IN
                            {
                                db.kvstore->getobjIDlistBysubIDpreID(result_var[var_id2],
                                        pre_id, id_list, id_list_len);
                            }
                            
                            if (i == 0)
                            {
                                can_lists[cur_depth].unionList(id_list, id_list_len);
                            }
                            else
                            {
                                can_lists[cur_depth].intersectList(id_list, id_list_len);
                            }
                        }
                        idx_count[cur_depth] = can_lists[cur_depth].size();
                    }
                    else
                    {
                        RDFTask *t = new RDFTask();
                        t->context.cur_depth = cur_depth+1;
                        t->context.result_var = new int[var_num + 1];
                        memcpy(t->context.result_var, result_var, sizeof(int)*(var_num + 1));
                        t->context.var_num = var_num;
                        add_task(t);
                    }
                }
            }
            cur_depth -= 1;
            if (cur_depth < enter_depth)
                break;
        }
    }
    **/

    void set_visited_arr(bool value, int idx)
    {
        if (idx >= Database::LITERAL_FIRST_ID)
            visited_arr[idx-Database::LITERAL_FIRST_ID+db.entity_num] = value;
        else
            visited_arr[idx] = value;
    }

    bool get_visited_arr(int idx)
    {
        if (idx >= Database::LITERAL_FIRST_ID)
            return visited_arr[idx-Database::LITERAL_FIRST_ID+db.entity_num];
        else
            return visited_arr[idx];
    }

    int preID2resultID(int id, int var_num)
    {
        if (id < 0)
            return -(id+10)+var_num; // [0,1,2,3] + var_num
        else
            return id;
    }

    // Iterative-style DFS with optimized SIMD intersection 
    void dfs_iterative_style_HW(int enter_depth, int *result_var, int var_num, int pre_var_num, vector<int *> &p_result_list, BasicQuery *basic_query, RDFQuery &q)
    {
        int cur_depth = enter_depth;
        if(cur_depth == 0)
        {
            int var_id = q.matching_order[0];
            if (var_id < 0)
            {
                // edge candidate
                int id_list_len = 0;
                int* id_list = NULL;
                int valid_pre_id = -(var_id + 10);
                for (int i = 0; i < q.preConst[valid_pre_id].size(); ++i)
                {
                    int lit_id = q.preConst[valid_pre_id][i].lit_id;
                    int edge_type = q.preConst[valid_pre_id][i].edge_type;
                    if (edge_type == BasicQuery::EDGE_OUT)
                        db.kvstore->getpreIDsubIDlistByobjID(lit_id, id_list, id_list_len);
                    else
                        db.kvstore->getpreIDobjIDlistBysubID(lit_id, id_list, id_list_len);

                    int new_cnt = 0;
                    for (int j = 0; j < id_list_len; j += 2)
                    {
                        if (new_cnt == 0 || id_list[new_cnt-1] != id_list[j])
                        {
                            id_list[new_cnt ++] = id_list[j];
                        }
                    }
                    id_list_len = new_cnt;
                    if (i == 0)
                    {
                        memcpy(can_lists[cur_depth], id_list, sizeof(int)*id_list_len);
                        can_counts[cur_depth] = id_list_len;
                    }
                    else
                    {
                        ComputeSetIntersection::ComputeCandidates(can_lists[cur_depth], can_counts[cur_depth], id_list, id_list_len, 
                                                                            temp_buffer, temp_count);
                        memcpy(can_lists[cur_depth], temp_buffer, sizeof(int)*temp_count);
                        can_counts[cur_depth] = temp_count;
                    }
                }

                idx[0] = 0;
                idx_count[0] = can_counts[cur_depth];
            }
            else
            {
                idx[0] = 0;
                idx_count[0] = basic_query->getCandidateSize(var_id);
                IDList &can_list = basic_query->getCandidateList(var_id); // return reference
                // for(int i=0; i<idx_count[0]; ++i)
                // {
                //     can_lists[0][i] = can_list[i];
                // }
                memcpy(can_lists[0], can_list.id_list.data(), sizeof(int)*idx_count[0]);
                // can_lists[0] = can_list.id_list.data();
            }
        }
        else
        {
            int var_id = q.matching_order[cur_depth];

            if (var_id < 0)
            {
                // edge candidate
                int id_list_len = 0;
                int* id_list = NULL;
                int valid_pre_id = -(var_id + 10);

                if (valid_pre_id < q.preConst.size())
                {
                    for (int i = 0; i < q.preConst[valid_pre_id].size(); ++i)
                    {
                        int lit_id = q.preConst[valid_pre_id][i].lit_id;
                        char edge_type = q.preConst[valid_pre_id][i].edge_type;
                        if (edge_type == BasicQuery::EDGE_OUT)
                            db.kvstore->getpreIDsubIDlistByobjID(lit_id, id_list, id_list_len);
                        else
                            db.kvstore->getpreIDobjIDlistBysubID(lit_id, id_list, id_list_len);

                        int new_cnt = 0;
                        for (int j = 0; j < id_list_len; j += 2)
                        {
                            if (new_cnt == 0 || id_list[new_cnt-1] != id_list[j])
                            {
                                id_list[new_cnt ++] = id_list[j];
                            }
                        }
                        id_list_len = new_cnt;
                        if (i == 0)
                        {
                            memcpy(can_lists[cur_depth], id_list, sizeof(int)*id_list_len);
                            can_counts[cur_depth] = id_list_len;
                        }
                        else
                        {
                            ComputeSetIntersection::ComputeCandidates(can_lists[cur_depth], can_counts[cur_depth], id_list, id_list_len, 
                                                                                temp_buffer, temp_count);
                            memcpy(can_lists[cur_depth], temp_buffer, sizeof(int)*temp_count);
                            can_counts[cur_depth] = temp_count;
                        }
                    }
                }
                else // check edge_node_anchor 
                {
                    int id_list_len = 0;
                    int* id_list = NULL;
                    char edge_type = q.edge_node_anchor[var_id].edge_type;
                    int anchor_id = q.edge_node_anchor[var_id].lit_id;
                    if (edge_type == BasicQuery::EDGE_OUT)
                        db.kvstore->getpreIDobjIDlistBysubID(result_var[anchor_id], id_list, id_list_len);
                    else
                        db.kvstore->getpreIDsubIDlistByobjID(result_var[anchor_id], id_list, id_list_len);

                    int new_cnt = 0;
                    for (int j = 0; j < id_list_len; j += 2)
                    {
                        if (new_cnt == 0 || id_list[new_cnt-1] != id_list[j])
                        {
                            id_list[new_cnt ++] = id_list[j];
                        }
                    }
                    id_list_len = new_cnt;
                    memcpy(can_lists[cur_depth], id_list, sizeof(int)*id_list_len);
                    can_counts[cur_depth] = id_list_len;
                }

                idx[0] = 0;
                idx_count[0] = can_counts[cur_depth];
                for (int i = 0; i < enter_depth; ++i)
                {
                    if (q.matching_order[i] >= 0)
                        set_visited_arr(true, result_var[q.matching_order[i]]);
                }
            }
            else 
            {
                // do intersection for constant neighbor
                bool has_const_neighbor = false;
                for (int i = 0; i < q.constBN[var_id].size(); ++i)
                {
                    int lit_id = q.constBN[var_id][i].lit_id;
                    int pre_id = q.constBN[var_id][i].pre_id;
                    char edge_type = q.constBN[var_id][i].edge_type;

                    if (pre_id < 0)
                        pre_id = result_var[preID2resultID(pre_id, var_num)];

                    int id_list_len = 0;
                    int* id_list = NULL;
                    if (edge_type == BasicQuery::EDGE_OUT)
                        db.kvstore->getsubIDlistByobjIDpreID(lit_id, pre_id, id_list, id_list_len);
                    else
                        db.kvstore->getobjIDlistBysubIDpreID(lit_id, pre_id, id_list, id_list_len);
                    if (i == 0)
                    {
                        memcpy(can_lists[cur_depth], id_list, sizeof(int)*id_list_len);
                        can_counts[cur_depth] = id_list_len;
                    }
                    else
                    {
                        ComputeSetIntersection::ComputeCandidates(can_lists[cur_depth], can_counts[cur_depth], id_list, id_list_len, 
                                                                            temp_buffer, temp_count);
                        memcpy(can_lists[cur_depth], temp_buffer, sizeof(int)*temp_count);
                        can_counts[cur_depth] = temp_count;
                    }
                    has_const_neighbor = true;
                }
                
                // do intersection for backward neighbor
                for (int i = 0; i < q.bn_count[cur_depth]; ++i)
                {
                    int var_id2_idx = q.bn_idx[cur_depth][i];
                    int edge_id = basic_query->getEdgeID(var_id, var_id2_idx);
                    int var_id2 = basic_query->getEdgeNeighborID(var_id, var_id2_idx); // BN
                    if (var_id2 == -1)
                    {
                        continue;
                    }
                    int pre_id = basic_query->getEdgePreID(var_id, var_id2_idx);
                    char edge_type = basic_query->getEdgeType(var_id, var_id2_idx);

                    if (pre_id < 0)
                    {
                        pair<int, int> key; 
                        if (var_id < var_id2)
                            key = make_pair(var_id, var_id2);
                        else
                            key = make_pair(var_id2, var_id);
                        pre_id = q.varIDvarID2preID[key];
                        pre_id = result_var[preID2resultID(pre_id, var_num)];
                    }
                    
                    // do intersection by BN 
                    
                    int* id_list;
                    int id_list_len;
                    if (edge_type == BasicQuery::EDGE_OUT)
                    {
                        db.kvstore->getsubIDlistByobjIDpreID(result_var[var_id2],
                                pre_id, id_list, id_list_len);
                    }
                    else // EDGE_IN
                    {
                        db.kvstore->getobjIDlistBysubIDpreID(result_var[var_id2],
                                pre_id, id_list, id_list_len);
                    }
                    
                    if (i == 0 && !has_const_neighbor)
                    {
                        // for(int j=0; j<id_list_len; ++j) can_lists[cur_depth][j] = id_list[j];
                        memcpy(can_lists[cur_depth], id_list, sizeof(int)*id_list_len);
                        can_counts[cur_depth] = id_list_len;
                    }
                    else
                    {
                        ComputeSetIntersection::ComputeCandidates(can_lists[cur_depth], can_counts[cur_depth], id_list, id_list_len,
                                                                            temp_buffer, temp_count);
                        // for(int i = 0; i < temp_count; ++i)
                        // {
                        //     can_lists[cur_depth][i] = temp_buffer[i];
                        // }
                        memcpy(can_lists[cur_depth], temp_buffer, sizeof(int)*temp_count);
                        can_counts[cur_depth] = temp_count;
                    }
                }
                idx[cur_depth] = 0;
                idx_count[cur_depth] = can_counts[cur_depth];

                for (int i = 0; i < enter_depth; ++i)
                {
                    if (q.matching_order[i] >= 0)
                        set_visited_arr(true, result_var[q.matching_order[i]]);
                }
            }
        }

        while (true)
        {
            // cout << "cur_depth : " <<  cur_depth << endl;
            while (idx[cur_depth] < idx_count[cur_depth])
            {
                int var_id = q.matching_order[cur_depth];
                // IDList can_list = basic_query->getCandidateList(var_id);
                int v = can_lists[cur_depth][idx[cur_depth]]; // v can be literal
                
                bool shouldVar2AddLiteralCandidateWhenJoin = basic_query->isFreeLiteralVariable(var_id) && !basic_query->isAddedLiteralCandidate(var_id);    
                // bool found_in_id_list = can_list.bsearch_uporder(v) >= 0;
                // bool found_in_id_list = can_list.getID(ComputeSetIntersection::BinarySearchForGallopingSearchAVX2(can_list.id_list.data(), 0, can_list.size()-1, v)) == v;

                bool found_in_id_list = false;
                if (var_id >= 0 && db.objIDIsEntityID(v))
                    found_in_id_list = db._entity_bitset[v]->cover(q.sparql_q.getBasicQuery(0).var_sig[var_id]);


                // cout << "Match ("<< var_id << ", "<< v << ") together" << endl;

                idx[cur_depth] += 1;

                if (var_id >= 0 && get_visited_arr(v))
                    continue;


                //@@: If shouldAddLiteral = true, this variable is a free literal
                bool should_add_this_literal = shouldVar2AddLiteralCandidateWhenJoin && !db.objIDIsEntityID(v);

                if (found_in_id_list || should_add_this_literal)
                // if (found_in_id_list)
                {   
                    if (var_id >= 0)
                        set_visited_arr(true, v);
                    result_var[preID2resultID(var_id, var_num)] = v;
                    if (cur_depth == var_num + pre_var_num - 1)
                    {
                        int* result_var_cp = new int[var_num + pre_var_num + 1];
                        memcpy(result_var_cp, result_var, sizeof(int)*(var_num + pre_var_num + 1));
                        p_result_list.push_back(result_var_cp);
                        if (var_id >= 0)
                            set_visited_arr(false, v);
                        continue;
                    }
                

                    if (countElaspedTime() < TIME_THRESHOLD)
                    {
                        cur_depth += 1;
                        idx[cur_depth] = 0;

                        int var_id_n = q.matching_order[cur_depth];

                        if (var_id_n < 0)
                        {
                            // edge candidate
                            int id_list_len = 0;
                            int* id_list = NULL;
                            int valid_pre_id = -(var_id_n + 10);
                            
                            if (valid_pre_id < q.preConst.size())
                            {
                                for (int i = 0; i < q.preConst[valid_pre_id].size(); ++i)
                                {
                                    int lit_id = q.preConst[valid_pre_id][i].lit_id;
                                    char edge_type = q.preConst[valid_pre_id][i].edge_type;

                                    if (edge_type == BasicQuery::EDGE_OUT)
                                        db.kvstore->getpreIDsubIDlistByobjID(lit_id, id_list, id_list_len);
                                    else
                                        db.kvstore->getpreIDobjIDlistBysubID(lit_id, id_list, id_list_len);

                                    int new_cnt = 0;
                                    for (int j = 0; j < id_list_len; j += 2)
                                    {
                                        if (new_cnt == 0 || id_list[new_cnt-1] != id_list[j])
                                        {
                                            id_list[new_cnt ++] = id_list[j];
                                        }
                                    }
                                    id_list_len = new_cnt;
                                    if (i == 0)
                                    {
                                        memcpy(can_lists[cur_depth], id_list, sizeof(int)*id_list_len);
                                        can_counts[cur_depth] = id_list_len;
                                    }
                                    else
                                    {
                                        ComputeSetIntersection::ComputeCandidates(can_lists[cur_depth], can_counts[cur_depth], id_list, id_list_len, 
                                                                                            temp_buffer, temp_count);
                                        memcpy(can_lists[cur_depth], temp_buffer, sizeof(int)*temp_count);
                                        can_counts[cur_depth] = temp_count;
                                    }
                                }
                            }
                            else // check edge_node_anchor 
                            {
                                int id_list_len = 0;
                                int* id_list = NULL;
                                char edge_type = q.edge_node_anchor[var_id_n].edge_type;
                                int anchor_id = q.edge_node_anchor[var_id_n].lit_id;

                                if (edge_type == BasicQuery::EDGE_OUT)
                                    db.kvstore->getpreIDobjIDlistBysubID(result_var[anchor_id], id_list, id_list_len);
                                else
                                    db.kvstore->getpreIDsubIDlistByobjID(result_var[anchor_id], id_list, id_list_len);

                                int new_cnt = 0;
                                for (int j = 0; j < id_list_len; j += 2)
                                {
                                    if (new_cnt == 0 || id_list[new_cnt-1] != id_list[j])
                                    {
                                        id_list[new_cnt ++] = id_list[j];
                                    }
                                }
                                id_list_len = new_cnt;

                                memcpy(can_lists[cur_depth], id_list, sizeof(int)*id_list_len);
                                can_counts[cur_depth] = id_list_len;
                            }

                            idx_count[cur_depth] = can_counts[cur_depth];
                        }
                        else
                        {
                            // do intersection for constant neighbor
                            bool has_const_neighbor = false;
                            for (int i = 0; i < q.constBN[var_id_n].size(); ++i)
                            {
                                int lit_id = q.constBN[var_id_n][i].lit_id;
                                int pre_id = q.constBN[var_id_n][i].pre_id;
                                char edge_type = q.constBN[var_id_n][i].edge_type;
                                
                                // cout << "before Pre_ID = " << pre_id << endl;
                                if (pre_id < 0)
                                    pre_id = result_var[preID2resultID(pre_id, var_num)];
                                // cout << "after Pre_ID = " << pre_id << endl;

                                // cout << "lit_id=" << lit_id << " pre_id=" << pre_id <<endl;

                                int id_list_len = 0;
                                int* id_list = NULL;
                                if (edge_type == BasicQuery::EDGE_OUT)
                                    db.kvstore->getsubIDlistByobjIDpreID(lit_id, pre_id, id_list, id_list_len);
                                else
                                    db.kvstore->getobjIDlistBysubIDpreID(lit_id, pre_id, id_list, id_list_len);

                                // cout << "id_list_len: " << id_list_len << endl;
                                if (i == 0)
                                {
                                    memcpy(can_lists[cur_depth], id_list, sizeof(int)*id_list_len);
                                    can_counts[cur_depth] = id_list_len;
                                }
                                else
                                {
                                    ComputeSetIntersection::ComputeCandidates(can_lists[cur_depth], can_counts[cur_depth], id_list, id_list_len, 
                                                                                        temp_buffer, temp_count);
                                    memcpy(can_lists[cur_depth], temp_buffer, sizeof(int)*temp_count);
                                    can_counts[cur_depth] = temp_count;
                                }
                                has_const_neighbor = true;
                            }

                            // cout << "OOOOOO can_counts[cur_depth] = " << can_counts[cur_depth] << endl;

                            // do intersection for backward neighbor
                            for (int i = 0; i < q.bn_count[cur_depth]; ++i)
                            {
                                int var_id2_idx = q.bn_idx[cur_depth][i];
                                int edge_id = basic_query->getEdgeID(var_id_n, var_id2_idx);
                                int var_id2 = basic_query->getEdgeNeighborID(var_id_n, var_id2_idx); // BN
                                if (var_id2 == -1)
                                {
                                    continue;
                                }
                                int pre_id = basic_query->getEdgePreID(var_id_n, var_id2_idx); // now pre_id=-1, need to recover correct pre_id
                                char edge_type = basic_query->getEdgeType(var_id_n, var_id2_idx);

                                if (pre_id < 0)
                                {
                                    pair<int, int> key; 
                                    if (var_id_n < var_id2)
                                        key = make_pair(var_id_n, var_id2);
                                    else
                                        key = make_pair(var_id2, var_id_n);
                                    pre_id = q.varIDvarID2preID[key];
                                    pre_id = result_var[preID2resultID(pre_id, var_num)]; 
                                }
                                
                                // do intersection by BN 
                                
                                int* id_list;
                                int id_list_len;
                                if (edge_type == BasicQuery::EDGE_OUT)
                                {
                                    db.kvstore->getsubIDlistByobjIDpreID(result_var[var_id2],
                                            pre_id, id_list, id_list_len);
                                }
                                else // EDGE_IN
                                {
                                    db.kvstore->getobjIDlistBysubIDpreID(result_var[var_id2],
                                            pre_id, id_list, id_list_len);
                                }
                                if (id_list == NULL)
                                {
                                    can_counts[cur_depth] = 0;
                                } 
                                else 
                                {
                                    if (i == 0 && !has_const_neighbor)
                                    {
                                        // for(int j=0; j<id_list_len; ++j) can_lists[cur_depth][j] = id_list[j];
                                        memcpy(can_lists[cur_depth], id_list, sizeof(int)*id_list_len);
                                        can_counts[cur_depth] = id_list_len;
                                    }
                                    else
                                    {
                                        ComputeSetIntersection::ComputeCandidates(can_lists[cur_depth], can_counts[cur_depth], id_list, id_list_len,
                                                                                temp_buffer, temp_count);
                                        // for(int i = 0; i < temp_count; ++i)
                                        // {
                                        //     can_lists[cur_depth][i] = temp_buffer[i];
                                        // }
                                        memcpy(can_lists[cur_depth], temp_buffer, sizeof(int)*temp_count);
                                        can_counts[cur_depth] = temp_count;
                                    }
                                }
                            }
                            idx_count[cur_depth] = can_counts[cur_depth];
                        }  
                    }
                    else
                    {
                        RDFTask *t = new RDFTask();
                        t->context.cur_depth = cur_depth+1;
                        t->context.result_var = new int[var_num + pre_var_num + 1];
                        memcpy(t->context.result_var, result_var, sizeof(int)*(var_num + pre_var_num + 1));
                        t->context.var_num = var_num;
                        t->context.pre_var_num = pre_var_num;
                        add_task(t);

                        q.global_task_num[thread_id]++;

                        if (var_id >= 0)
                            set_visited_arr(false, v);
                    }
                }
            }
            cur_depth -= 1;
            if (cur_depth < enter_depth)
                break;
            else
            {
                if (q.matching_order[cur_depth] >= 0)
                    set_visited_arr(false, result_var[q.matching_order[cur_depth]]);
            }
        }

        for (int i = 0; i < enter_depth; ++i)
        {   
            if (q.matching_order[i] >= 0)
                set_visited_arr(false, result_var[q.matching_order[i]]);
        }
    }
    


    void dfs(int cur_depth, int *result_var, vector<int *> &p_result_list, BasicQuery *basic_query, int var_num, RDFQuery &q)
    {
        struct timeb cur_time;
		double drun_time;

        if (cur_depth == var_num)
        {
            int* result_var_cp = new int[var_num + 1];
            memcpy(result_var_cp, result_var, sizeof(int)*(var_num + 1));
            p_result_list.push_back(result_var_cp);
        }
        else
        {
            int var_id = q.matching_order[cur_depth];

            if (cur_depth == 0)
            {
                IDList* p_min_var_list = &(basic_query->getCandidateList(var_id));
                int start_var_size = basic_query->getCandidateSize(var_id);
                for (int i = 0; i < start_var_size; ++i)
                {
                    result_var[var_id] = p_min_var_list->getID(i);
                    
                    // dfs(cur_depth+1, result_var, p_result_list, basic_query, var_num, q);

                    ftime(&cur_time);
                    drun_time = (cur_time.time-gtime_start[thread_id].time)*1000+(double)(cur_time.millitm-gtime_start[thread_id].millitm);
                    if(drun_time < TIME_THRESHOLD) 
                        dfs(cur_depth+1, result_var, p_result_list, basic_query, var_num, q);
                    else
                    {
                        RDFTask *t = new RDFTask();
                        t->context.cur_depth = cur_depth+1;
                        t->context.result_var = new int[var_num + 1];
                        memcpy(t->context.result_var, result_var, sizeof(int)*(var_num + 1));
                        t->context.var_num = var_num;
                        add_task(t);
                    }
                }
            }
            else
            {
                IDList local_cands;
                IDList can_list = basic_query->getCandidateList(var_id);

                // get backward neighbors
                for (int i = 0; i < q.bn_count[cur_depth]; ++i)
                {
                    int var_id2_idx = q.bn_idx[cur_depth][i];

                    int edge_id = basic_query->getEdgeID(var_id, var_id2_idx);
                    
                    int var_id2 = basic_query->getEdgeNeighborID(var_id, var_id2_idx); // BN
                    if (var_id2 == -1)
                    {
                        continue;
                    }
                    int pre_id = basic_query->getEdgePreID(var_id, var_id2_idx);
                    char edge_type = basic_query->getEdgeType(var_id, var_id2_idx);
                    
                    // do intersection by BN 

                    IDList temp_list;

                    int* id_list;
                    int id_list_len;
                    if (edge_type == BasicQuery::EDGE_OUT)
                    {
                        db.kvstore->getsubIDlistByobjIDpreID(result_var[var_id2],
                                pre_id, id_list, id_list_len);
                    }
                    else // EDGE_IN
                    {
                        db.kvstore->getobjIDlistBysubIDpreID(result_var[var_id2],
                                pre_id, id_list, id_list_len);

                    }
                    temp_list.unionList(id_list, id_list_len);

                    if (i == 0)
                    {
                        local_cands.unionList(temp_list);
                    }
                    else
                    {
                        local_cands.intersectList(temp_list);
                    }
                }

                bool shouldVar2AddLiteralCandidateWhenJoin = basic_query->isFreeLiteralVariable(var_id) && !basic_query->isAddedLiteralCandidate(var_id);

                for (int i = 0; i < local_cands.size(); ++i)
                {
                    bool found_in_id_list = can_list.bsearch_uporder(local_cands[i]) >= 0;

                    //@@: If shouldAddLiteral = true, this variable is a free literal
                    bool should_add_this_literal = shouldVar2AddLiteralCandidateWhenJoin && !db.objIDIsEntityID(local_cands[i]);

                    if (found_in_id_list || should_add_this_literal)
                    {
                        result_var[var_id] = local_cands[i];

                        ftime(&cur_time);
                        drun_time = (cur_time.time-gtime_start[thread_id].time)*1000+(double)(cur_time.millitm-gtime_start[thread_id].millitm);
                        if(drun_time < TIME_THRESHOLD) 
                            dfs(cur_depth+1, result_var, p_result_list, basic_query, var_num, q);
                        else
                        {
                            RDFTask *t = new RDFTask();
                            t->context.cur_depth = cur_depth+1;
                            t->context.result_var = new int[var_num + 1];
                            memcpy(t->context.result_var, result_var, sizeof(int)*(var_num + 1));
                            t->context.var_num = var_num;
                            add_task(t);
                        }
                    }
                }
            }
        }
    }


    virtual void compute(ContextT &context, RDFQuery &q)
    {

        if(temp_buffer == NULL)
        {
            // temp_buffer = new int[q.max_cand_size]; //@@: WARNING: Not precise, but doable
            temp_buffer = new int[db.entity_num+db.literal_num];
            can_lists = new int*[MAX_QUERY_VERTEX];
            can_counts = new int[MAX_QUERY_VERTEX];
            visited_arr = new bool[db.entity_num+db.literal_num];
            memset(visited_arr, false, sizeof(bool)*(db.entity_num+db.literal_num));
            for(int i=0; i<MAX_QUERY_VERTEX; ++i)
            {
                // can_lists[i] = new int[q.max_cand_size]; //@@: WARNING: Not precise, but doable
                can_lists[i] = new int[db.entity_num+db.literal_num];
            }
        }

        ftime(&gtime_start[thread_id]);
        // dfs(context.cur_depth, context.result_var, q.p_result_list[thread_id], &(q.sparql_q.getBasicQuery(0)), context.var_num, q);
        // dfs_iterative_style(context.cur_depth, context.result_var, context.var_num, q.p_result_list[thread_id], &(q.sparql_q.getBasicQuery(0)), q);
        dfs_iterative_style_HW(context.cur_depth, context.result_var, context.var_num, context.pre_var_num, q.p_result_list[thread_id], &(q.sparql_q.getBasicQuery(0)), q);
    }

    virtual bool postprocess(RDFQuery &q)
    {   
        // accumulate results

        ftime(&q.end_t);
        double totaltime = (q.end_t.time-q.start_t.time)*1000+(double)(q.end_t.millitm-q.start_t.millitm);
        cout<<"Query "<<get_queryID()<<" Parallel+Preprocess time: "<<totaltime << " ms" <<endl;

        BasicQuery* basic_query = &q.sparql_q.getBasicQuery(0);
        vector<int *> &result_list = basic_query->getResultList();
        cout << "Found by each thread: "; 
        for(int i=0; i<32; ++i)
        {
            cout << q.p_result_list[i].size() << " ";
            result_list.insert(result_list.end(), q.p_result_list[i].begin(), q.p_result_list[i].end());
        }
        cout << endl;

        // db.only_pre_filter_after_join(basic_query);
        // basic_query->dupRemoval_invalidRemoval();

        q.sparql_q.evaPlan.clear();
        db.genEvaPlan(q.sparql_q.getPatternGroup(), q.sparql_q, 0);
        db.doEvaPlan(q.sparql_q);

        q.rs.select_var_num = q.sparql_q.getProjectionsNum();
        db.getFinalResult(q.sparql_q, q.rs);

        cout << "Final result is: "  << q.rs.ansNum << endl;
        // cout << "Final result is: "  << q.rs.to_str() << endl;

        ftime(&q.end_t);
        totaltime = (q.end_t.time-q.start_t.time)*1000+(double)(q.end_t.millitm-q.start_t.millitm);
        cout<<"Query "<<get_queryID()<<" total time: "<<totaltime << " ms" <<endl;

        int ttl_task_num = 0;
        for (int i=0; i<32; ++i)
            ttl_task_num += q.global_task_num[i];
        std::cout << "task num = " << ttl_task_num + 1 << std::endl;

        return false;
    }

    virtual bool is_bigTask(ContextValue &context)
    {
        return false;
    }
};

class RDFWorker: public Worker<RDFComper>
{
public:
    RDFWorker(int num_compers) : Worker(num_compers)
    {     
    }

    ~RDFWorker()
    {
    }

    void load_data(char* file_path)
    {
        std::string fp = std::string(file_path);
        db.preload(fp);
        db.load();
    }
};


#endif
