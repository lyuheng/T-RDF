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

## YAGO-2.5.3
### Q1: finding two different people who acted in the same movie and places they were born in.
```
SELECT ?n1 ?n2 ?city1 ?city2
WHERE 
{
    ?a1 <hasFamilyName> ?n1 .
    ?a2 <hasFamilyName> ?n2 .
    ?a1 <wasBornIn> ?city1 . 
    ?a2 <wasBornIn> ?city2 . 
    ?a1 <actedIn> ?m . 
    ?a2 <actedIn> ?m . 
}
```
### Q2: finding two different people who were born in the same place and had the same family name.
```
SELECT ?a1 ?a2 ?x
WHERE  
{ 
    ?a1 <diedOnDate> ?d1 . 
    ?a2 <diedOnDate> ?d2 . 
    ?a1 <wasBornIn> ?x . 
    ?a2 <wasBornIn> ?x . 
    ?a1 <hasFamilyName> ?name .
    ?a2 <hasFamilyName> ?name .
}
```
### Q3: finding two different people who were born in the same place, at the same time, and had won the same award.
```
SELECT ?a1 ?a2 ?city1 ?d1
WHERE 
{
    ?a1 <hasWonPrize> ?award .
    ?a2 <hasWonPrize> ?award .
    ?a1 <wasBornIn> ?city1 .
    ?a2 <wasBornIn> ?city1 .
    ?a1 <wasBornOnDate> ?d1 .
    ?a2 <wasBornOnDate> ?d1 .
}
```
### Q4: finding a couple who had won the same award.
```
SELECT ?n1 ?n2 ?award1 ?city1
WHERE { 
    ?a1 <hasFamilyName> ?n1 .
    ?a2 <hasFamilyName> ?n2 .
    ?a1 <hasWonPrize> ?award1 . 
    ?a2 <hasWonPrize> ?award2 . 
    ?a1 <wasBornIn> ?city1 . 
    ?a2 <wasBornIn> ?city2 . 
    ?a1 <isMarriedTo> ?a2 .
}
```

## LUBM
### Q1: find student and his advisor that got their UG degrees from the same university.
```
SELECT ?S ?T
WHERE
{
  ?S  <advisor>  ?T .
  ?S  <undergraduateDegreeFrom>  ?X .
  ?T  <undergraduateDegreeFrom>  ?X .
}
```

### Q2: find the course a student take is taught by the student's advisor.
```
SELECT ?X ?Y ?Z
WHERE 
{ 
  ?X <teacherOf> ?Y .
  ?Z <takesCourse> ?Y . 
  ?Z <advisor> ?X .
}
```

### Q3: find affiliation people belong to is where they got their UG degrees.
```
SELECT ?x ?y ?z 
WHERE 
{
    ?z <subOrganizationOf> ?y . 
    ?x <memberOf> ?z . 
    ?x <undergraduateDegreeFrom> ?y . 
}
```

### Q4: find the place where student's UG and advisor's PhD are the same, and advisor's research interest.
```
SELECT ?S ?T ?R ?X
WHERE
{
  ?S <advisor> ?T .
  ?T <doctoralDegreeFrom> ?X .
  ?S <undergraduateDegreeFrom> ?X .
  ?T <researchInterest> ?R .
}
```
### Q5: find student and his advisor and their common relationship to University222.
```
SELECT ?S1 ?T
WHERE
{
  ?S1 <advisor> ?T .
  ?S1 <PU1> <http://www.University222.edu> .
  ?T <PU1> <http://www.University222.edu> .
}
```
### Q6: find the relationship of student and his advisor's course. 
```
SELECT ?X ?Y ?Z 
WHERE 
{ 
  ?X <teacherOf> ?Y .
  ?Z <PU1> ?Y . 
  ?Z <advisor> ?X .
}
```
