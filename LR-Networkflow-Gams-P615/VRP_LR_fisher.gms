$ontext
    VRPTW for fisher
    Lagrangian Relaxation/Variable Splitting
    2019.8
$offtext

set k vehicle /1*2/;
set n nodes /0*5/;
set n_o(n)/0/;
set n_d(n)/0/;
set n_m(n)/1*5/;

alias(i,j,n);
alias(i1,j1,n_m);

parameter xx(i)/
0        40
1        45
2        45
3        42
4        42
5        42
/;
parameter yy(i)/
0        50
1        68
2        70
3        66
4        68
5        65
/;

parameter QQ(k);
    QQ(k)=50;

parameter q(i)/
1        10
2        30
3        10
4        10
5        10
/;
parameter s(i)/
0        0
1        90
2        90
3        90
4        90
5        90
/;

parameter c(i,j);
c(i,j) = sqrt((yy(j)-yy(i))*((yy(j)-yy(i)))+(xx(j)-xx(i))*(xx(j)-xx(i)));

parameter e(i)/
0        0
1        912
2        825
3        65
4        727
5        15
/;
parameter u(i)/
0        1236
1        967
2        870
3        146
4        782
5        67
/;

scalar M/9999/;
parameter lambda(i,k);
lambda(i,k)=0.0;

******************************
variable z;
variable z_1;
variable z_2;
binary variable x(i,j,k);
binary variable y(i,k);

positive variable t(i);
positive variable tA(k);
positive variable tD(k);

******************************
equation obj_fun;
equation subproblem_1;
equation subproblem_2;

equation network_o;
equation network_d;
equation network_m;

equation time_process_m;
equation time_process_o;
equation time_process_d;

equation time_e;
equation time_u;
equation vehicle_time_start;
equation vehicle_time_end;

equation capacity_const;
equation other1;
equation other2;

******************************
obj_fun..
*z=e=sum((i,j,k)$c(i,j),c(i,j)*x(i,j,k))+sum(k,tA(k)-tD(k));
         z=e=sum((i,j,k)$c(i,j),c(i,j)*x(i,j,k));
subproblem_1..
        z_1=e=-sum((i,k),lambda(i,k)*y(i,k));
subproblem_2..
        z_2=e=sum((i,j,k)$c(i,j),(c(i,j)+lambda(i,k)*x(i,j,k)));


******************************
network_o(n_o,k)..
         sum(j$c(n_o,j),x(n_o,j,k))=e=1;
network_d(n_d,k)..
         sum(j$c(j,n_d),x(j,n_d,k))=e=1;
network_m(n_m,k)..
         sum(j$c(j,n_m),x(j,n_m,k))=e=sum(j$c(n_m,j),x(n_m,j,k));
******************************
time_process_m(i1,j1,k)$c(i1,j1)..
         t(i1)+s(i1)+c(i1,j1)-M*(1-x(i1,j1,k))=l=t(j1);
time_process_o(n_o,j1,k)$c(n_o,j1)..
         TD(k)+c(n_o,j1)-M*(1-x(n_o,j1,k))=l=t(j1);
time_process_d(i1,n_d,k)$c(i1,n_d)..
         t(i1)+s(i1)+c(i1,n_d)-M*(1-x(i1,n_d,k))=l=TA(k);
******************************
time_e(i1)..
         t(i1)=g=e(i1);
time_u(i1)..
         t(i1)=l=u(i1);
vehicle_time_start(k)..
         tD(k)=g=e('0');
vehicle_time_end(k)..
         tA(k)=l=u('0');
******************************
capacity_const(k)..
         sum((i1,j1)$c(i1,j1),q(i1)*x(i1,j1,k))=l=QQ(k);
other1(i1)..
         sum(k,y(i1,k))=e=1;
other2(i,k)..
         sum(j$c(i,j),x(i,j,k))=e=y(i,k);

******************************
model obj/all/;
model sub_1/subproblem_1,other1/;
model sub_2/subproblem_2,network_d,network_m,network_o,time_process_d,time_process_m,time_process_o,time_e,time_u,vehicle_time_end,vehicle_time_start,capacity_const/;
******************************
*************LR***************


parameter z_x,z_y;
parameter subgrad_lambda(i,k);
parameter i_value;
i_value=1;
parameter step;
step=1;
parameter z_lb;


sets iter subgradient iteration index/iter1*iter2/;

loop(iter,
    solve sub_1 using MIP minimizing z_1;
    solve sub_2 using MIP minimizing z_2;

*   Update multipliers??
    subgrad_lambda(i,k)= sum(k,x.l(i,j,k))-y.l(i,k);
    lambda(i,k)=max(0,lambda(i,k)+step*subgrad_lambda(i,k));

    i_value=i_value+1;
    step=1/i_value;
    z_lb=z_1.l+z_2.l;

    display x.l,y.l;
    display subgrad_lambda;
    display lambda;

);


