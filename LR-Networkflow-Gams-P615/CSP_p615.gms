$ontext
         Lagrangian Relaxation for CSP
         Network Flow P615
         Created by Entai Wang
         2019.8.29

$offtext


set i/1*6/;
set i_o(i)/1/;
set i_d(i)/6/;
set i_m(i)/2*5/;

alias(i,j);

parameter c(i,j)/
1.2      1
1.3      10
2.4      1
2.5      2
3.2      1
3.4      5
3.5      12
4.5      10
4.6      1
5.6      2
/;

parameter t(i,j)/
1.2      10
1.3      3
2.4      1
2.5      3
3.2      2
3.4      7
3.5      3
4.5      1
4.6      7
5.6      2
/;

scalar T_M/14/;

binary variable x(i,j);
variable z;

equation cost_function;
equation netflow_origin(i_o);
equation netflow_destination(i_d);
equation netflow_middle(i_m);
equation travel_time_constraint;


cost_function.. z=e=sum((i,j)$c(i,j),c(i,j)*x(i,j));
netflow_origin(i_o).. sum(j$c(i_o,j),x(i_o,j))=e=1;
netflow_destination(i_d).. sum(j$c(j,i_d),x(j,i_d))=e=1;
netflow_middle(i_m).. sum(j$c(j,i_m),x(j,i_m))=e=sum(j$c(i_m,j),x(i_m,j));
travel_time_constraint.. sum((i,j)$t(i,j),t(i,j)*x(i,j))=l=T_M;

model cssp/cost_function,
           netflow_origin,
           netflow_destination,
           netflow_middle,
           travel_time_constraint/;

******lr
variable z_lr;
parameter u;
equation lr_function;
lr_function..
        z_lr=e=sum((i,j)$c(i,j),c(i,j)*x(i,j))+u*(sum((i,j)$c(i,j),t(i,j)*x(i,j))-T_M);
model lr_cssp/lr_function,
           netflow_origin,
           netflow_destination,
           netflow_middle
/;

u=0;
set iter/1*60/;
scalar theta;
scalar gamma;
scalar UB/24/;
parameter results(iter,*);
parameter lambda/
*lambda(iter)=0.8;
1       0.8
2       0.8
3       0.8
4       0.8
5       0.8
6       0.8
7       0.8
8       0.8
9       0.8
10      0.8
11      0.8
12      0.8
13      0.8
14      0.8
15      0.8
16      0.4
17      0.4
18      0.4
19      0.4
20      0.4
21      0.4
22      0.4
23      0.4
24      0.4
25      0.4
26      0.4
27      0.4
28      0.4
29      0.4
30      0.4
31      0.4
32      0.4
33      0.4
34      0.4
35      0.4
36      0.4
37      0.4
38      0.4
39      0.4
40      0.4
41      0.2
42      0.2
43      0.2
44      0.2
45      0.2
46      0.2
47      0.2
48      0.2
49      0.2
50      0.2
51      0.2
52      0.2
53      0.2
54      0.2
55      0.2
56      0.2
57      0.2
58      0.2
59      0.2
60      0.2
/;

loop(iter,
         solve lr_cssp using mip minimazing z_lr;

         display u;
         display x.l;
         display z_lr.l;

         gamma=sum((i,j)$t(i,j),t(i,j)*x.l(i,j))-T_M;
         if(gamma<=0,
                 UB=sum((i,j)$c(i,j),c(i,j)*x.l(i,j));
         );

         theta=lambda(iter)*(UB-z_lr.l)/(gamma*gamma);


         results(iter,'u')=u;
         results(iter,'gamma')=gamma;
         results(iter,'UB')=UB;
         results(iter,'LB')=z_lr.l;
         results(iter,'lambda')=lambda(iter);
         results(iter,'theta')=theta;


         u=u+theta*gamma;

);

display results;


