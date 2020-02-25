function testNLP
nlp0 = NonlinearProgram(0);
sol = nlp0.solve([]);
if(~isempty(sol))
  error('Solution should be empty for this problem');
end
nlp1 = NonlinearProgram(3);
nlp1 = nlp1.setCheckGrad(true);
warning('Off','optimlib:fmincon:WillRunDiffAlg');
warning('Off','optimlib:fmincon:SwitchingToMediumScale');
%%%%%%%%%%%
% x1^2+4*x2^2<=4
% (x1-2)^2+x2^2<=5
% x1 >= 0
cnstr1 = FunctionHandleConstraint(-inf(2,1),[4;5],2,@cnstr1_userfun);
xind1 = [1;2];
nlp1 = nlp1.addConstraint(cnstr1,xind1);
nlp1 = nlp1.addConstraint(BoundingBoxConstraint(0,inf),1);
x1 = [1;2;1];
[g1,h1,dg1,dh1] = nlp1.nonlinearConstraints(x1);
[g1_user,dg1_user] = cnstr1_userfun(x1(xind1));
valuecheck(g1,g1_user);
valuecheck(dg1(:,xind1),dg1_user);
if(~isempty(h1) || ~isempty(dh1))
  error('There should be no nonlinear equality constraint');
end
%%%%%%%%%
% x1^2+4*x2^2<=4
% (x1-2)^2+x2^2<=5
% x1 >= 0
% 0<=x1+2*x3 <=10
% x1+3*x3 = 0
A = [1 0 2;1 0 3];
nlp1 = nlp1.addLinearConstraint(LinearConstraint([0;0],[10;0],[1 2;1 3]),[1;3]);
valuecheck(nlp1.beq,0);
valuecheck(nlp1.Aeq,[1 0 3]);
valuecheck(nlp1.Ain,[A(1,:);-A(1,:)]);
valuecheck(nlp1.bin,[10;0]);
x0 = [1;2;4];
nlp1 = nlp1.setSolverOptions('snopt','print','print.out');
[x1,F,info] = testAllSolvers(nlp1,x0);
c1 = cnstr1_userfun(x1);
if(c1(1)>4+1e-5 || c1(2)>5+1e-5)
  error('Wrong transcription nonlinear constraint');
end
b1 = A*x1;
if(b1(1)>1+1e-5 || b1(1)<0-1e-5 || abs(b1(2))>1e-5)
  error('Wrong transcription for linear constraint');
end
if(x1(1)<-1e-5)
  error('Wrong transcription for x_lb');
end
%%%%%%%%%%%%%%%%
% min x2^2+x1*x3+x3
% x1^2+4*x2^2<=4
% (x1-2)^2+x2^2<=5
% x1 >= 0
% 0<=x1+2*x3 <=10
% x1+3*x3 = 0
nlp1 = nlp1.addCost(FunctionHandleObjective(1,@cost1_userfun),2);
nlp1 = nlp1.addCost(FunctionHandleObjective(2,@cost2_userfun),[1;3]);
nlp1 = nlp1.addCost(LinearConstraint(-inf,inf,[0 1]),[1;3]);
[x2,F,info] = testAllSolvers(nlp1,x0);
c2 = cnstr1_userfun(x2);
if(c2(1)>4+1e-5 || c2(2)>5+1e-5)
  error('Wrong transcription for nonlinear constraint');
end
b2 = A*x2;
if(b2(1)>1+1e-5 || b2(1)<0-1e-5 || abs(b2(2))>1e-5)
  error('Wrong transcription for linear constraint');
end
if(x2(1)<-1e-5)
  error('Wrong transcription for x_lb');
end
f2 = cost1_userfun(x2(2))+cost2_userfun(x2([1;3]))+x2(3);
valuecheck(F,f2);
%%%%%%%%%%%%%%%%%%%%%
% min x2^2+x1*x3+x3
% x1^2+4*x2^2<=4
% (x1-2)^2+x2^2<=5
% x1 >= 0
% 0<=x1+2*x3 <=10
% x1+3*x3 = 0
% x1*x2*x3 = 1/6;
% -10<=x2^2+x2*x3+2*x3^2<=30
nc2 = FunctionHandleConstraint([1/6;-10],[1/6;30],3,@cnstr2_userfun);
nc2 = nc2.setSparseStructure([1;1;1;2;2],[1;2;3;2;3]);
nlp1 = nlp1.addConstraint(nc2);
x0 = [1;2;3];
[x,F,info] = testAllSolvers(nlp1,x0);
c2 = cnstr2_userfun(x);
valuecheck(c2(1),1/6,1e-5);
% min x2^2+x1*x3+x3
% x1^2+4*x2^2<=4
% (x1-2)^2+x2^2<=5
% x1 >= 0
% 0<=x1+2*x3 <=10
% x1+3*x3 = 0
% x1*x2*x3 = 1/6;
% -10<=x2^2+x2*x3+2*x3^2<=30
% x2+x3<=10
% -x2+x3 = 0.1;
nlp1 = nlp1.addLinearConstraint(LinearConstraint(0.1,0.1,[0 -1 1]));
nlp1 = nlp1.addLinearConstraint(LinearConstraint(-inf,10,[0 1 1]));
valuecheck(nlp1.bin,[10;0;10]);
valuecheck(nlp1.beq,[0;0.1]);
valuecheck(nlp1.Ain,[A(1,:);-A(1,:);0 1 1]);
valuecheck(nlp1.Aeq,[1 0 3;0 -1 1]);
[x,F,info] = testAllSolvers(nlp1,x0);
valuecheck(-x(2)+x(3),0.1,1e-5);
if(x(2)+x(3)>10+1e-5)
  error('Linear constraint not correct');
end

display('test addDecisionVar');
nlp1 = nlp1.addDecisionVariable(2);
[x,F,info] = testAllSolvers(nlp1,[x0;randn(2,1)]);

display('test replaceCost');
% min x2^2+x1^2+x1x2+x1*x3+x3
% x1^2+4*x2^2<=4
% (x1-2)^2+x2^2<=5
% x1 >= 0
% 0<=x1+2*x3 <=10
% x1+3*x3 = 0
% x1*x2*x3 = 1/6;
% -10<=x2^2+x2*x3+2*x3^2<=30
% x2+x3<=10
% -x2+x3 = 0.1;
qc1 = QuadraticConstraint(-inf,inf,[1 0.5;0.5 1],zeros(2,1));
nlp1 = nlp1.replaceCost(qc1,1,[1;2]);
[x,F,info] = testAllSolvers(nlp1,[x0;randn(2,1)]);
F_obj = qc1.eval(x(1:2));
F_obj = F_obj+cost2_userfun(x([1;3]))+x(3);
valuecheck(F,F_obj,1e-5);

%%%%%%%%%%%%%%%%%%%%%
display('test multi-input constraint and cost')
% min x1*x2
% -10<=x1^2+4*x2^2<=4
% -10<=(x1-2)^2+x2^2<=5
% x1 >= 0
% 0<=x1+2*x3 <=10
% x1+3*x3 = 0
nlp3 = NonlinearProgram(3);
nlp3 = nlp3.setCheckGrad(true);
cnstr3 = FunctionHandleConstraint([-10;-10],[4;5],2,@cnstr3_userfun);
cost3 = FunctionHandleObjective(2,@cost3_userfun);
xind1 = {1;2};
[nlp3,cnstr3_id] = nlp3.addConstraint(cnstr3,xind1);
cnstr3_id_old = cnstr3_id;
nlp3 = nlp3.addCost(cost3,xind1);
bbcon1 = BoundingBoxConstraint(0,inf);
[nlp3,bbcon1_id] = nlp3.addBoundingBoxConstraint(bbcon1,1);

A = [1 0 2;1 0 3];
lincon1 = LinearConstraint([0;0],[10;0],[1 2;1 3]);
[nlp3,lincon1_id] = nlp3.addLinearConstraint(lincon1,[1;3]);
x0 = [1;2;4];
[x3,F,info] =testAllSolvers(nlp3,x0);
c3 = cnstr3_userfun(x3(1),x3(2));
if(c3(1)>4+1e-5 || c3(2)>5+1e-5)
  error('Wrong transcription for nonlinear constraint');
end
b1 = A*x3;
if(b1(1)>1+1e-5 || b1(1)<0-1e-5 || abs(b1(2))>1e-5)
  error('Wrong transcription for linear constraint');
end
if(x3(1)<-1e-5)
  error('Wrong transcription for x_lb');
end

%%%%%%%%%%%%%%%%%%%
display('test deleteNonlinearConstraint')
display('first try to delete all nonlinear constraint');
nlp4 = nlp3.deleteNonlinearConstraint(cnstr3_id);
if(nlp4.isNonlinearConstraintID(cnstr3_id))
  error('The deleted ID should not be contained in the program any more');
end
valuecheck(nlp4.num_cin,0);
valuecheck(nlp4.num_ceq,0);
if(~isempty(nlp4.cin_lb) || ~isempty(nlp4.cin_ub) )
  error('The lower and upper bound of cin should be empty');
end
[lb,ub] = nlp4.bounds();
valuecheck(lb,[-inf;-inf;-inf;0]);
valuecheck(ub,[inf;10;0;0]);
[iGfun,jGvar] = nlp4.getNonlinearGradientSparsity();
valuecheck([1 1 1],full(sparse(iGfun,jGvar,ones(size(iGfun)),1+nlp4.num_cin+nlp4.num_ceq,nlp4.num_vars)));
[g,h,dg,dh] = nlp4.nonlinearConstraints(x0);
if(~isempty(g) || ~isempty(h) || ~isempty(dg) || ~isempty(dh))
  error('The nonlinear constraint should be empty');
end
[x4,F,info] = testAllSolvers(nlp4,x0);

% min x1*x2
% -10<=x1^2+4*x2^2<=4
% -10<=(x1-2)^2+x2^2<=5
% x1 >= 0
% 0<=x1+2*x3 <=10
% x1+3*x3 = 0
% x1*x2+x3 = 0
% 0<=x1+x3^2<=10
% x1+2*x3*x4+x4^2 = 5
[nlp4,cnstr3_id] = nlp4.addConstraint(cnstr3,xind1);
nlp4 = nlp4.addDecisionVariable(1);
cnstr4 = FunctionHandleConstraint([0;0;5],[0;10;5],4,@cnstr4_userfun,2);
[nlp4,cnstr4_id] = nlp4.addConstraint(cnstr4);
[x4,F,info] = testAllSolvers(nlp4,[x0;0]);

nlp4 = nlp4.deleteNonlinearConstraint(cnstr3_id);
if(nlp4.isNonlinearConstraintID(cnstr3_id))
  error('The deleted ID should not be contained in the program any more');
end
valuecheck(nlp4.num_cin,1);
valuecheck(nlp4.num_ceq,2);
valuecheck(nlp4.cin_lb,0);
valuecheck(nlp4.cin_ub,10);
valuecheck(length(nlp4.nlcon),1);
sizecheck(nlp4.cin_name,1);
sizecheck(nlp4.ceq_name,2);
sizecheck(nlp4.nlcon_xind,1);
valuecheck(nlp4.nlcon_xind{1}{1},[1;2;3;4]);
[iGfun,jGvar] = nlp4.getNonlinearGradientSparsity();
valuecheck([1 1 1 0;ones(3,4)],full(sparse(iGfun,jGvar,ones(size(iGfun)),1+nlp4.num_cin+nlp4.num_ceq,nlp4.num_vars)));
[g,h,dg,dh] = nlp4.nonlinearConstraints([x0;1]);
[c4,dc4] = cnstr4_userfun([x0;1]);
valuecheck(g,c4(2));
valuecheck(h,c4([1;3])-[0;5]);
valuecheck(dg,dc4(2,:));
valuecheck(dh,dc4([1;3],:));
[x4,F,info] = testAllSolvers(nlp4,x4);

nlp4 = nlp4.deleteNonlinearConstraint(cnstr4_id);
if(nlp4.isNonlinearConstraintID(cnstr4_id))
  error('The deleted ID should not be contained in the program any more');
end
valuecheck(nlp4.num_cin,0);
valuecheck(nlp4.num_ceq,0);
if(~isempty(nlp4.cin_lb) || ~isempty(nlp4.cin_ub) )
  error('The lower and upper bound of cin should be empty');
end
[lb,ub] = nlp4.bounds();
valuecheck(lb,[-inf;-inf;-inf;0]);
valuecheck(ub,[inf;10;0;0]);
[iGfun,jGvar] = nlp4.getNonlinearGradientSparsity();
valuecheck([1 1 1 0],full(sparse(iGfun,jGvar,ones(size(iGfun)),1+nlp4.num_cin+nlp4.num_ceq,nlp4.num_vars)));
[g,h,dg,dh] = nlp4.nonlinearConstraints(x0);
if(~isempty(g) || ~isempty(h) || ~isempty(dg) || ~isempty(dh))
  error('The nonlinear constraint should be empty');
end
[x4,F,info] = testAllSolvers(nlp4,[x0;1]);

%%%%%%
display('check updateNonlinearConstraint');
nlp4 = nlp3.addDecisionVariable(1);
[nlp4,new_cnstr_id] = nlp4.updateNonlinearConstraint(cnstr3_id_old,cnstr4);
valuecheck(nlp4.num_cin,1);
valuecheck(nlp4.num_ceq,2);
valuecheck(nlp4.cin_lb,0);
valuecheck(nlp4.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 