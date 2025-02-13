# T-RDF: A Task-Based Parallel System for Efficiently Answering SPARQL Queries on RDF Data

## Compile
Under the root directory of the project, use following commands to compile.
```
chmod 755 ./scripts/pre.sh ./experiment/*
./scripts/pre.sh
make
```

## Usage
### Step 1 (If Step 1 has been done before, please directly go to Step 2)
If user is querying on a new RDF data, please execute the following commands for adjacency list and index construction, etc. 
```
./gload [DB_FOLDER_PATH] [RDF_DATA_PATH]
```
DB_FOLDER_PATH: Path where constructed files are stored.

RDF_DATA_PATH: Path where RDF data is stored.

For example:
```
./gload ./lubm_folder ./demo/lubm-1.nt
```

### Step 2
Please execute the following command to start the server side.
```
./run [DB_FOLDER_PATH] [THREAD_NUM]
```
DB_FOLDER_PATH: Path where constructed files are stored.

THREAD_NUM: Number of threads used in the server.

For exmaple:
```
./run ./lubm_folder 4
```

> <em>Caveat: If you see segment fault is popped out immediately, please use following command to clear messages in IPC.</em>
```
./experiment/clean
```

### Step 3
Open a new terminal as client side (should be on the same machine as server), run following commands to start client:
```
cd client
make
./run
```

Copy your query file <em>path</em> in the terminal after "Please enter your query ..." is displayed on the server side, press enter and the results will be shown in the server side.

For example, copy the following path in the client side
```
./query/LUBM/q1
```

If everything goes right, you will see something like 
![result](https://github.com/lyuheng/T-RDF/blob/main/demo/lubm_q1_result.png)


## RDF Data Acquisition

### [YAGO-2.3.0](https://yago-knowledge.org/downloads/yago-2)
Click Turtle and data will be downloaded automatically.

> <em>Please comment the first line <code>@base</code> in the file to remove redunduncy.</em>

### [YAGO-2.5.3](https://yago-knowledge.org/downloads/yago-2s)
Click Full Turtle format and data will be downloaded automatically.

> <em>Please comment the first line <code>@base</code> in the file to remove redunduncy.</em>

### [LUBM](http://swat.cse.lehigh.edu/projects/lubm)
LUBM is a synthetic dataset generated by Java program. Please refer [data generation](https://ipads.se.sjtu.edu.cn:1312/opensource/wukong/-/blob/old-gstore/docs/INSTALL.md#step-1-generate-lubm-datasets-with-raw-format) and [data conversion](https://ipads.se.sjtu.edu.cn:1312/opensource/wukong/-/blob/old-gstore/datagen/README.md#manual-convert) for more details.

## Query Acquisition
Please refer to <code>query</code> folder for all queries used in experiments and their descriptions.

## Special Case for Queries with Unknown Predicate

T-RDF temporarily doesn't support normal predicate variable names starting with ?, such as ?P1, ?P2. But we tailor made this situation with a special naming prefix <PU>. Different predicate variables can be named as \<PU1\>, \<PU2\>, \<PU3\>, etc. 

For example, in <code>query/LUBM/q6</code>, \<PU1\> refers to a predicate variable linking ?Z and ?Y.
```
SELECT ?X ?Y ?Z 
WHERE 
{ 
  ?X <teacherOf> ?Y .
  ?Z <PU1> ?Y . 
  ?Z <advisor> ?X .
}
```
