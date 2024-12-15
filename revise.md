
## Baseline 

MaGiQ: https://github.com/yzchen/MAGiQ-CombBLAS


## Dataset

WatDiv-100M: https://dsg.uwaterloo.ca/watdiv/

Query samples: https://dsg.uwaterloo.ca/watdiv/basic-testing.shtml

tripleNum is 108997714 \
entityNum is 5212745 \
preNum is 86 \
literalNum is 5038202

===========================

Berlin SPARQL Benchmark (BSBM): http://wbsg.informatik.uni-mannheim.de/bizer/berlinsparqlbenchmark/spec/BenchmarkRules/index.html#datagenerator

Run the following to obtain dataset.nt
```
./generate -pc 100 -s nt
```