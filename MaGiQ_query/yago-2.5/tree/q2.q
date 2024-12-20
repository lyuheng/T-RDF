SELECT ?n1 ?n2 ?award1 ?city1 WHERE { 
    ?a1 <49> ?n1 .
    ?a2 <49> ?n2 .
    ?a1 <17> ?award1 . 
    ?a2 <17> ?award2 . 
    ?a1 <10> ?city1 . 
    ?a2 <10> ?city2 . 
    ?a1 <25> ?a2 .
}
#EOQ#
