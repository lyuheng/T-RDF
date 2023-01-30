# Query Description

## YAGO-2.3.0
### Q1: finding two different people who acted in the same movie and won the same prize.
```
SELECT ?n1 ?n2 ?city1 ?city2 
WHERE { 
    ?a1 <hasPreferredName> ?n1 . 
    ?a2 <hasPreferredName> ?n2 . 
    ?a1 <wasBornIn> ?city1 . 
    ?a2 <wasBornIn> ?city2 . 
    ?a1 <actedIn> ?m . 
    ?a2 <actedIn> ?m . 
    ?a1 <hasWonPrize> ?w .
    ?a2 <hasWonPrize> ?w .
}
```

### Q2: finding two different people who were born in the same place and had the same family name.
```
SELECT ?n1 ?n2 
WHERE { 
    ?a1 <hasPreferredName> ?n1 . 
    ?a2 <hasPreferredName> ?n2 . 
    ?a1 <diedOnDate> ?d1 . 
    ?a2 <diedOnDate> ?d2 . 
    ?a1 <wasBornIn> ?x . 
    ?a2 <wasBornIn> ?x . 
    ?a1 <hasFamilyName> ?name .
    ?a2 <hasFamilyName> ?name .
}
```
### Q3: finding two different people who won the same prize and individually was born and died in the same city, and these two cities located in the same country.
```
SELECT ?a1 ?a2
WHERE { 
    ?a1 <hasPreferredName> ?n1 . 
    ?a2 <hasPreferredName> ?n2 .
    ?a1 <wasBornIn> ?city1 . 
    ?a2 <wasBornIn> ?city2 . 
    ?a1 <diedIn> ?city1 . 
    ?a2 <diedIn> ?city2 . 
    ?city1 <isLocatedIn> ?c .
    ?city2 <isLocatedIn> ?c .
    ?a1 <hasWonPrize> ?w .
    ?a2 <hasWonPrize> ?w .
}
```
### Q4: finding two different people who were born in the same place and won the same prize.
```
SELECT ?n1 ?n2 ?award1 ?city1
WHERE { 
    ?a1 <hasFamilyName> ?n1 .
    ?a2 <hasFamilyName> ?n2 .
    ?a1 <hasWonPrize> ?award1 . 
    ?a2 <hasWonPrize> ?award1 . 
    ?a1 <wasBornIn> ?city1 . 
    ?a2 <wasBornIn> ?city1 .
}
```
