set i nodes /1*4/;
set a OD pair /1/;
alias (i, j);

parameter c(i,j) link travel cost /
1.2 3
1.3 1
3.2 1
2.4 1
3.4 4
1.4 7
/;

parameter cap(i,j) link capacity /
1.2 1
1.3 1
3.2 1
2.4 1
3.4 1
1.4 1
/;

parameter origin(a,i) /
1.1 2
/;

parameter destination(a,i) /
1.4 2
/;

parameter intermediate(a,i);
intermediate(a,i) = (1- origin(a,i))* (1- destination(a,i));

*-----------------------------------------------------------------------
* subproblems
*-----------------------------------------------------------------------

positive variables xsub(i,j);
variables zsub;

parameters
   origin_sub(i)
   destination_sub(i)
   intermediate_sub(i)
   pi1(i,j) 'dual of limit'
   pi2(a)   'dual of convexity constraint'
   pi2a
;

equations
comm_flow_on_node_origin_sub(i)         origin node flow
comm_flow_on_node_intermediate_sub(i)   intermediate node flow
comm_flow_on_node_destination_sub(i)      destination node flow
rc1_sub          'phase 1 objective'
rc2_sub          'phase 2 objective'
;

comm_flow_on_node_origin_sub(i)$(origin_sub(i)>0.1).. sum(j$(c(i,j)>0.1),xsub(i,j)) =e= origin_sub(i);
comm_flow_on_node_intermediate_sub(i)$(intermediate_sub(i)>0.1).. sum(j$(c(i,j)>0.1), xsub(i,j))-sum(j$(c(j,i)>0.1), xsub(j,i))=e= 0;
comm_flow_on_node_destination_sub(i)$(destination_sub(i)>0.1) ..  sum(j$(c(j,i)>0.1), xsub(j,i))=e= destination_sub(i);
rc1_sub..       zsub =e= sum((i,j), -pi1(i,j)*xsub(i,j)) - pi2a;
rc2_sub..       zsub =e= sum((i,j), (c(i,j)-pi1(i,j))*xsub(i,j)) - pi2a;

model sub1 'phase 1 subproblem' /comm_flow_on_node_origin_sub, comm_flow_on_node_intermediate_sub,comm_flow_on_node_destination_sub, rc1_sub/;
model sub2 'phase 2 subproblem' /comm_flow_on_node_origin_sub, comm_flow_on_node_intermediate_sub,comm_flow_on_node_destination_sub, rc2_sub/;

loop(a,
       c(i,j) = c(i,j);
       origin_sub(i) = origin(a,i);
       destination_sub(i) = destination(a,i);
       intermediate_sub(i) = (1- origin_sub(i))* (1- destination_sub(i));
***************************************************************
       pi1(i,j) = 0;
       pi2a = 0;
*solve sub1 using lp minimizing zsub;
       solve sub2 using lp minimizing zsub;
***************************************************************
);

display xsub.l,zsub.l;

*-----------------------------------------------------------------------
* Column generation
*-----------------------------------------------------------------------
set k 'proposal count' /proposal1*proposal10/;
set ak(a,k);
ak(a,k) = no;

parameter proposal(i,j,a,k);
parameter proposalcost(a,k);
proposal(i,j,a,k) = 0;
proposalcost(a,k) = 0;

set kk(k) 'current proposal';
kk('proposal1') = yes;

loop(a,
         proposal(i,j,a,kk) = xsub.l(i,j);
         proposalcost(a,kk) = sum((i,j), c(i,j)*xsub.l(i,j));
         ak(a,kk) = yes;
         kk(k) = kk(k-1);
);
display proposal,proposalcost;
*proposal<=>column

$ontext
----     80 VARIABLE xsub.L

            2           3           4

1                   2.000
2                               2.000
3       2.000


----     80 VARIABLE zsub.L                =        6.000

----    103 PARAMETER proposal

INDEX 1 = 1

      proposal1

3.1       2.000

INDEX 1 = 2

      proposal1

4.1       2.000

INDEX 1 = 3

      proposal1

2.1       2.000


----    103 PARAMETER proposalcost

    proposal1

1       6.000
$offtext
