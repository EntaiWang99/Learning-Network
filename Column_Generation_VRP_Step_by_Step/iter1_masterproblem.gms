set i nodes /1*4/;
set a OD pair /1/;
alias (i, j);

parameter cap(i,j) link capacity /
1.2 1
1.3 1
3.2 1
2.4 1
3.4 1
1.4 1
/;


set k 'proposal count' /proposal1*proposal10/;
set ak(a,k);
**********************************************************
ak('1','proposal1') = yes;

parameter proposal(i,j,a,k)/
1.3.1.proposal1  2
3.2.1.proposal1  2
2.4.1.proposal1  2
/
;
*********************************************************
parameter proposalcost(a,k);
*proposal(i,j,a,k) = 0;
proposalcost(a,k) = 6;


positive variables
   lambda(a,k)
   excess   'artificial variable'
;
variable zmaster;

equations
    obj1_master    'phase 1 objective'
    obj2_master    'phase 2 objective'
    capacity_limit_master(i,j)
    convex_master
;

obj1_master..  zmaster =e= excess;
obj2_master..  zmaster =e= sum(ak, proposalcost(ak)*lambda(ak));

capacity_limit_master(i,j)..
   sum(ak, proposal(i,j,ak)*lambda(ak)) =l= cap(i,j) + excess;

convex_master(a).. sum(ak(a,k), lambda(a,k)) =e= 1;

model master1 'phase 1 master' /obj1_master, capacity_limit_master, convex_master/;
model master2 'phase 2 master' /obj2_master, capacity_limit_master, convex_master/;

solve master1 minimizing zmaster using lp;
display lambda.l,excess.l;


parameters
   pi1(i,j) 'dual of limit'
   pi2(a)   'dual of convexity constraint'
;
pi1(i,j) = capacity_limit_master.m(i,j);
pi2(a) = convex_master.m(a);
display pi1,pi2;


$ontext
----     57 VARIABLE lambda.L

    proposal1

1       1.000


----     57 VARIABLE excess.L              =        1.000  artificial variable

----     66 PARAMETER pi1  dual of limit

            3

1      -1.000


----     66 PARAMETER pi2  dual of convexity constraint

1 2.000
$offtext
