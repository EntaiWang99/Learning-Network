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
*why not use 1.1 2; 2.1 2???

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
   pi2a
;
parameter
   pi2(a) 'dual of convexity constraint' /
         1       14/;
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
********************************************************************
       pi1('1','3') = -1;
       pi1('2','4') = -3;
       pi2a = pi2(a);
*solve sub1 using lp minimizing zsub;
       solve sub2 using lp minimizing zsub;
*corresponding to the master problem in last iteration  Why???
**Why use sub1 or sub2? In what cases use which???
********************************************************************
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
kk('proposal4') = yes;

loop(a,
         proposal(i,j,a,kk) = xsub.l(i,j);
         proposalcost(a,kk) = sum((i,j), c(i,j)*xsub.l(i,j));
         ak(a,kk) = yes;
         kk(k) = kk(k-1);
);
display proposal;
display proposalcost;
*proposal<=>column

$ontext
----     83 VARIABLE xsub.L

            3           4

1       2.000
3                   2.000


----     83 VARIABLE zsub.L                =       -2.000

----    106 PARAMETER proposal

INDEX 1 = 1

      proposal4

3.1       2.000

INDEX 1 = 3

      proposal4

4.1       2.000


----    107 PARAMETER proposalcost

    proposal4

1      10.000
$offtext
