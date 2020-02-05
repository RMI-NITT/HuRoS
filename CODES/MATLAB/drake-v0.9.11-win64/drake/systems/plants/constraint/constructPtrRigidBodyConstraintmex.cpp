#include "constructPtrRigidBodyConstraint.h"
#include "RigidBodyManipulator.h"
#include "RigidBodyConstraint.h"
#include "drakeMexUtil.h"
#include "controlUtil.h"
using namespace Eigen;
using namespace std;

void checkBodyOrFrameID(const int body, const RigidBodyManipulator* model, const char* body_var_name="body")
{
  if(body >= static_cast<int>(model->bodies.size())) {
    mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","%s must be less than %d (got %d)",body_var_name,model->bodies.size(),body);
  } else if(body < -static_cast<int>(model->frames.size()) - 1) {
    mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","%s must be greater than %d (got %d)",body_var_name,-model->frames.size(),body);
  } else if(body == -1){
    mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Recieved %s == 0, which is reserved for the center of mass. Please use a WorldCoMConstraint instead.");
  }
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if(!mxIsNumeric(prhs[0]))
  {
    if (isa(prhs[0],"DrakeMexPointer")) {  // then it's calling the destructor
      destroyDrakeMexPointer<RigidBodyConstraint*>(prhs[0]);
      return;
    }
    mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","prhs[0] should be the constraint type");
  }
  int constraint_type = (int) mxGetScalar(prhs[0]);
  switch(constraint_type)
  {
    // QuasiStaticConstraint
    case RigidBodyConstraint::QuasiStaticConstraintType:
      {
        if(nrhs != 2 && nrhs!= 3 && nrhs != 4)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrPtrRigidBodyConstraintmex(RigidBodyConstraint::QuasiStaticConstraintType,robot_ptr,tspan,robotnum)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        Vector2d tspan;
        int* robotnum;
        size_t num_robot;
        if(nrhs <= 3)
        {
          num_robot = 1;
          robotnum = new int[num_robot];
          robotnum[0] = 0;
        }
        else
        {
          num_robot = mxGetNumberOfElements(prhs[3]);
          double* robotnum_tmp = new double[num_robot];
          robotnum = new int[num_robot];
          memcpy(robotnum_tmp,mxGetPrSafe(prhs[3]),sizeof(double)*num_robot);
          for(int i = 0;i<num_robot;i++)
          {
            robotnum[i] = (int) robotnum_tmp[i]-1;
          }
          delete[] robotnum_tmp;
        }
        if(nrhs<=2)
        {
          tspan<<-mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[2],tspan);
        }
        set<int> robotnumset(robotnum,robotnum+num_robot);
        QuasiStaticConstraint* cnst = new QuasiStaticConstraint(model,tspan,robotnumset);
        plhs[0] = createDrakeConstraintMexPointer((void*) cnst,"QuasiStaticConstraint");
        delete[] robotnum;
      }
      break;
    // PostureConstraint
    case RigidBodyConstraint::PostureConstraintType:
      {
        if(nrhs != 2 && nrhs != 3)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::PostureConstraintType,robot.mex_model_ptr,tspan)");
        }
        Vector2d tspan;
        if(nrhs < 3)
        {
          tspan<<-mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[2],tspan);
        }
        RigidBodyManipulator* robot = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        PostureConstraint* cnst = new PostureConstraint(robot,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"PostureConstraint");
      }
      break;
      // SingleTimeLinearPostureConstraint
    case RigidBodyConstraint::SingleTimeLinearPostureConstraintType:
      {
        if(nrhs != 7 && nrhs != 8)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::SingleTimeLinearPostureConstraintType,robot.mex_model_ptr,iAfun,jAvar,A,lb,ub,tspan");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        Vector2d tspan;
        if(nrhs<=7)
        {
          tspan<<-mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[7],tspan);
        }
        if(!mxIsNumeric(prhs[2]) || !mxIsNumeric(prhs[3]) || !mxIsNumeric(prhs[4]))
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","iAfun, jAvar and A must be numeric");
        }
        size_t lenA = mxGetM(prhs[2]);
        if(mxGetM(prhs[3]) != lenA || mxGetM(prhs[4]) != lenA || mxGetN(prhs[2]) != 1 || mxGetN(prhs[3]) != 1 || mxGetN(prhs[4]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","iAfun,jAvar,A must be column vectors of the same size");
        }
        VectorXi iAfun(lenA);
        VectorXi jAvar(lenA);
        VectorXd A(lenA);
        for(int i = 0;i<lenA;i++)
        {
          iAfun(i) = (int) *(mxGetPrSafe(prhs[2])+i)-1;
          jAvar(i) = (int) *(mxGetPrSafe(prhs[3])+i)-1;
          A(i) = *(mxGetPrSafe(prhs[4])+i);
        }
        if(!mxIsNumeric(prhs[5]) || !mxIsNumeric(prhs[6]) || mxGetM(prhs[5]) != mxGetM(prhs[6]) || mxGetN(prhs[5]) != 1 || mxGetN(prhs[6]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","lb and ub must be numeric column vectors of the same size");
        }
        size_t num_constraint = mxGetM(prhs[5]);
        VectorXd lb(num_constraint);
        VectorXd ub(num_constraint);
        memcpy(lb.data(),mxGetPrSafe(prhs[5]),sizeof(double)*num_constraint);
        memcpy(ub.data(),mxGetPrSafe(prhs[6]),sizeof(double)*num_constraint);
        SingleTimeLinearPostureConstraint* cnst = new SingleTimeLinearPostureConstraint(model,iAfun,jAvar,A,lb,ub,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"SingleTimeLinearPostureConstraint");
      }
      break;
      // AllBodiesClosestDistanceConstraint
    case RigidBodyConstraint::AllBodiesClosestDistanceConstraintType:
      {
        //DEBUG
        //cout << "nrhs = " << nrhs << endl;
        //END_DEBUG
        if(nrhs != 5 && nrhs != 6)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs",
              "Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::AllBodiesClosestDistanceConstraintType, robot.mex_model_ptr,lb,ub,active_collision_options,tspan)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        Vector2d tspan;
        if(nrhs == 5)
        {
          tspan<< -mxGetInf(), mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[5],tspan);
        }

        double lb = (double) mxGetScalar(prhs[2]);
        double ub = (double) mxGetScalar(prhs[3]);

        // Parse `active_collision_options`
        vector<int> active_bodies_idx;
        set<string> active_group_names;
        // First get the list of body indices for which to compute distances
        const mxArray* active_collision_options = prhs[4];
        const mxArray* body_idx = mxGetField(active_collision_options,0,"body_idx");
        if (body_idx != NULL) {
          //DEBUG
          //cout << "collisionDetectmex: Received body_idx" << endl;
          //END_DEBUG
          size_t n_active_bodies = mxGetNumberOfElements(body_idx);
          //DEBUG
          //cout << "collisionDetectmex: n_active_bodies = " << n_active_bodies << endl;
          //END_DEBUG
          active_bodies_idx.resize(n_active_bodies);
          memcpy(active_bodies_idx.data(),(int*) mxGetData(body_idx),
              sizeof(int)*n_active_bodies);
          transform(active_bodies_idx.begin(),active_bodies_idx.end(),
              active_bodies_idx.begin(),
              [](int i){return --i;});
        }

        // Then get the group names for which to compute distances
        const mxArray* collision_groups = mxGetField(active_collision_options,0,
            "collision_groups");
	if (collision_groups != NULL) {
	  size_t num = mxGetNumberOfElements(collision_groups);
	  for (int i=0; i<num; i++) {
	    const mxArray *ptr = mxGetCell(collision_groups,i);
	    size_t buflen = mxGetN(ptr)*sizeof(mxChar)+1;
	    char* str = (char*)mxMalloc(buflen);
	    mxGetString(ptr, str, buflen);
	    active_group_names.insert(str);
	    mxFree(str);
	  }
	}
	
        auto cnst = new AllBodiesClosestDistanceConstraint(model,lb,ub,
            active_bodies_idx,active_group_names,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,
                                        "AllBodiesClosestDistanceConstraint");
      }
      break;
      // WorldEulerConstraint
    case RigidBodyConstraint::WorldEulerConstraintType:
      {
        if(nrhs != 6 && nrhs != 5)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldEulerConstraintType,robot.mex_model_ptr,body,lb,ub,tspan)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        WorldEulerConstraint* cnst = nullptr;
        Vector2d tspan;
        if(nrhs == 5)
        {
          tspan<< -mxGetInf(), mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[5],tspan);
        }
        if(!mxIsNumeric(prhs[2]))
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","body must be numeric");
        }
        int body = (int) mxGetScalar(prhs[2])-1;
        checkBodyOrFrameID(body, model);

        if(mxGetM(prhs[3]) != 3 || mxGetM(prhs[4]) != 3 || mxGetN(prhs[3]) != 1 || mxGetN(prhs[4]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","lb and ub should both be 3x1 double vectors");
        }
        Vector3d lb;
        Vector3d ub;
        memcpy(lb.data(),mxGetPrSafe(prhs[3]),sizeof(double)*3);
        memcpy(ub.data(),mxGetPrSafe(prhs[4]),sizeof(double)*3);
        cnst = new WorldEulerConstraint(model,body,lb,ub,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"WorldEulerConstraint");
      }
      break;
    // WorldGazeDirConstraint
    case RigidBodyConstraint::WorldGazeDirConstraintType:
      {
        if(nrhs != 7 && nrhs != 6)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldGazeDirConstraintType,robot.mex_model_ptr,body,axis,dir,conethreshold,tspan)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        WorldGazeDirConstraint* cnst = nullptr;
        Vector2d tspan;
        if(nrhs == 6)
        {
          tspan<< -mxGetInf(), mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[6],tspan);
        }
        if(!mxIsNumeric(prhs[2]))
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","body must be numeric");
        }
        int body = (int) mxGetScalar(prhs[2])-1;
        checkBodyOrFrameID(body, model);
        Vector3d axis;
        rigidBodyConstraintParse3dUnitVector(prhs[3], axis);
        Vector3d dir;
        rigidBodyConstraintParse3dUnitVector(prhs[4],dir);
        double conethreshold = rigidBodyConstraintParseGazeConethreshold(prhs[5]);
        cnst = new WorldGazeDirConstraint(model,body,axis,dir,conethreshold,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst, "WorldGazeDirConstraint");
      }
      break;
    // WorldGazeOrientConstraint
    case RigidBodyConstraint::WorldGazeOrientConstraintType:
      {
        if(nrhs != 7 && nrhs != 8)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldGazeOrientConstraintType, robot.mex_model_ptr,body,axis,quat_des,conethreshold,threshold,tspang.%C2%A0%5CnUsing+lower+temps%2C+better+cooling+and+increasing+retraction+settings+will+also+prevent+stringy+3D+prints+which+could+make+your+bridges+look+messy.%5Cn%C2%A0%5CnAdjust+Your+Slicer+Settings%5CnFirst%2C+adjust+your+slicer+settings+to+use+the+lowest+layer+thickness+possible.+This+is+effective+when+printing+a+difficult+3D+printer+overhang+because+there+is+less+material+being+deposited+with+each+pass+of+your+print+head.%5CnLess+mass+equals+quicker+cooling+time.%5CnSecond%2C+change+your+shell+settings%2Fperimeters+so+that+you+printing+from+the+inside+out+instead+of+the+outside+in.+This+will+help+anchor+your+topmost+layer+to+the+layer+underneath+as+you+print.%5Cn%C2%A0%5CnReduce+Your+Speed%5CnAgain%2C+rapid+cooling+is+essential+for+successfully+printing+angles+above+45%C2%B0.+Reducing+your+print+speed+can+help+you+speed+up+this+essential+cooling.+The+slower+the+speed+of+your+print+head%2C+the+longer+that+it+takes+for+your+material+to+get+from+the+print+nozzle+to+the+object.%5CnIn+addition%2C+slower+print+speeds+mean+that+your+layer+cooling+fan+spends+more+time+directing+air+flow+over+a+particular+section+of+your+object.%5CnOnce+you%E2%80%99ve+got+your+printer+up+to+speed+and+you%E2%80%99ve+dialled+in+your+settings%2C+it%E2%80%99s+a+good+idea+to+print+a+calibration+object.+A+calibration+object+will+allow+you+to+test+your+settings+before+you+pull+the+trigger+on+printing+an+object+that+you%E2%80%99re+going+to+use.%5CnWhen+it+comes+to+overhangs%2C+there+are+a+lot+of+designs+out+there%2C+like+this+one%C2%A0on+Thingverse%2C+that+will+push+your+printer+and+your+printing+skills+to+the+limit.%5Cn%C2%A0%5Cn3D+Printing+Bridges%5Cn%5CnBridges+present+much+of+the+same+printing+problems+as+overhang+3D+printing.+The+difference+is+that+bridges%2C+by+definition%2C+are+90%C2%B0+surfaces+supported+by+nothing+more+than+two+vertical+structures+at+either+end.%5CnLike+any+bridge%2C+it+is+the+tension+on+both+ends+of+the+string+of+print+material+that+prevents+the+middle+of+the+string+from+collapsing.+In+some+sense%2C+because+of+the+angle+involved%2C+you+could+describe+bridging+3D+printing+as+the+most+difficult+overhang+of+all.%5CnIn+general%2C+the+shorter+the+length+of+the+bridge%2C+the+greater+the+chance+that+the+bridge+will+succeed+structurally.+Conversely%2C+the+longer+the+bridge%2C+the+greater+the+chance+that+it+will+succumb+structural+stress.%5CnAs+a+result%2C+just+as+you+are+generally+in+safe+territory+with+overhangs+containing+angles+less+than+45%C2%B0%2C+you+are+also+generally+ok+with+bridge+lengths+of+5mm+or+less.%5CnHowever%2C+as+was+the+case+with+overhangs%2C+in+order+to+create+truly+beautiful+and+functional+objects%2C+you%E2%80%99ve+got+to+be+able+to+stretch+the+limits+of+what+you%2C+your+printer+and+the+filament+you%27re+using+can+do.%5CnAfter+all%2C+at+least+with+bridges+you+don%27t+need+to+worry+about%C2%A03D+printing+support+material+removal.%C2%A0Let%E2%80%99s+take+a+look+at+some+of+the+techniques+that+will+help+you+bridge+the+gap+in+your+designs+like+never+before.%5CnLuckily%2C+bridges+are+just+a+variant+of+overhangs.+This+means+that%2C+by+and+large%2C+the+same+techniques+that+help+you+print+angles+over+45%C2%B0+will+also+help+you+lengthen+the+distance+that+you+can+cross+with+your+bridges.%5Cn%5CnLearn+to+3D+print+without+support%5CnOur+3D+printing+bridging+tips+are%2C+firstly%2C+to%C2%A0make+sure+that+your+printer+is+operating+at+optimal+levels.+You+also+want+to+make+sure+that+that+you+are+cooling+your+printing+material+as+rapidly+as+possible.%5CnJust+as+with+overhangs%2C+the+longer+it+takes+for+your+material+to+cool%2C+the+more+likely+it+is+that+your+bridge+will+deform+or+fail.+Therefore%2C+use+your+layer+cooling+fans+aggressively.%5CnAlso%2C+lower+your+printing+temperatures+as+much+as+possible.+Adjust+your+slicer+settings+and+reduce+your+printing+speed+to+facilitate+the+cooling+that+will+let+your+bridge+the+largest+possible+distance.%5CnIt+helps+to+use+a+good+quality%2C+low+temperature+grade+material+to+begin+with.%5CnPrint+slower%21+Get+from+one+end+to+the+other+in+nice+and+smooth+way%3B+there%27s+no+need+to+rush.+The+extra+time+will+allow+for+better+adhesion+of+the+layers%2C+resulting+in+a+stronger%2C+neater+bridge.%C2%A0%5CnIn+addition%2C+there+are+one+or+two+other+tricks+that+you+can+try+to+make+printing+bridges+easier.+First%2C+if+you%27re+faced+with+an+extreme+bridge+distance%2C+try+changing+the+orientation+of+the+object+that+you%E2%80%99re+printing.%5CnA+difficult+bridge+may+be+impossible+when+the+object+is+viewed+vertically.+However%2C+the+impossible+bridge+is+a+piece+of+cake+if+the+object+is+rotated+90%C2%B0+and+printed+on+its+back.%5CnIn+some+respects%2C+printing+a+bridge+is+more+likely+to+succeed+than+printing+an+overhang%2C+because+you+have+two+anchor+points+for+a+bridge+-+one+at+each+end.%5CnHowever%2C+it%27s+worth+bearing+in+mind%C2%A0that+this+only+works+well+as+long+as+the+bridge+is+perfectly+level%2C+spanning+only+one+layer.+If+it%27s+tilting+upwards+just+slightly%2C+spanning+a+few+layers%2C+the+bridge+will+not+complete+quickly+%28or+neatly%29+in+a+single+layer.%C2%A0%5CnYou+can+also+make+difficult+bridging+situations+easier+by+slightly+altering+the+shape+of+the+bridge.+A+flat+bottomed+bridge+enforces+the+90%C2%B0+angle+that+is+so+difficult+to+print.%5CnBy+altering+that+angle+slightly%2C+say+with+a+slight+arch%2C+you+reduce+the+difficultly+of+the+print+angle.+A+straight+90%C2%B0+angle+suddenly+becomes+a+much+more+forgiving+60%C2%B0+to+70%C2%B0+angle+due+to+the+added+arch.+Much+more+achievable+in+the+long+run.%5CnFinally%2C+choose+a+printing+material+that+is+of+high+quality+to+get+those+overhangs+with+the+minimum+of+fuss.+The+quality+of+any+print+is+surprisingly+depended+on+the+quality+of+your+filament%2C+and+bridging+is+no+different.%C2%A0%5CnSo+allow+us+to+just+leave+this+here...%5Cn%21--split2--%21%5CnIf+you+found+this+article+helpful%2C+you%27ll+love+our+free+Easy+Cheat+Sheet+to+100%25+Reliable+3D+Printing%2C+which+can+be+found+at+the+bottom+of+this+page+just+below.%5CnIf+you+have+any+questions+regarding+this+article%2C+please+post+in+the+comments+below.+We%27ll+be+happy+to+help+out.%C2%A0%5CnPlease+share+this+article+if+you%C2%A0found+it+useful+and+hopefully+it+will+help+others+achieve+the+3D+printing+results+they%27re+looking+for+too.%C2%A0%22%7D%5D&sw=1920&sh=1080&v=2.8.30&r=stable&a=plshopify1.2&ec=1&o=30&it=1539857939976&coo=false&es=automatic                       U   https://rigid.ink/blogs/news/how-to-print-overhangs-bridges-and-exceeding-the-45-rule       `˜çe◊.     »          yete  ù˝   N   https://mail.google.com/mail/u/0/?pli=1#inbox/FMfcgxvzLDzqgfnDpKQFsrwKgDhLqxfk  5   s o c c e r b o t s   p r i n t   -   p s v s s a i k u m a r 1 9 9 8 @ g m a i l . c o m   -   G m a i l   $d   d     d                              `       X                                           ∞      %è~x è~x                           §   N   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1 # i n b o x / F M f c g x v z L D z q g f n D p K Q F s r w K g D h L q x f k                          p      h       ÿ       ¯                    8      P      `      x      ê      ∏      –      Ë                    f   /   
  ? %   B l i n k   s e r i a l i z e d   f o r m   s t a t e   v e r s i o n   9   
  = &                       N o   o w n e r               
      3                                                t e x t               
      0                                                t e x t               
      0                        
   h i s t _ s t a t e                         t e x t               
      1                     T   &   i n b o x / F M f c g x v z L D z q g f n D p K Q F s r w K g D h L q x f k     8      0       8             ?                                                                                      Ä      x       ê      ®      8      @
      à      à8       =      ¿H      ¿L      –P      0T      ¿U      ∞Y      @[      `       X       Ë       H              p             h      è~x è~x ®      ¿                    à   @   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? u i = 2 & v i e w = b s p & v e r = o h h l 4 r w 8 m b n 4               V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 $      s 1 b m q d y j p s a o q x            8      0       8                                                                                                            `       X       Ë       H              p             h      è~x è~x ®      ¿                    à   @   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? u i = 2 & v i e w = b s p & v e r = o h h l 4 r w 8 m b n 4               V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 $      h 1 b m q d y j p s a o q x            8      0       8                                                                                                            `       X               x               Ë              ‡       è~x è~x        8                          a b o u t : b l a n k                 n   3   < ! - - d y n a m i c F r a m e 5 9 F 3 2 F B 7 6 9 C E C 3 C 7 6 7 A 2 4 E E 1 7 A 2 B 8 1 2 B - - >          8      0       8                                                                                                            `       X       ê                    `             X      w"~x w"~x ò      ∞                    .    h t t p s : / / a c c o u n t s . g o o g l e . c o m / o / o a u t h 2 / p o s t m e s s a g e R e l a y ? p a r e n t = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m & j s h = m % 3 B % 2 F _ % 2 F s c s % 2 F a b c - s t a t i c % 2 F _ % 2 F j s % 2 F k % 3 D g a p i . g a p i . e n . B s t D m E h p a _ Q . O % 2 F r t % 3 D j % 2 F d % 3 D 1 % 2 F r s % 3 D A H p O o o - o q K d O J I c y A i 5 Y y A 6 p a h S k p _ o j s w % 2 F m % 3 D _ _ f e a t u r e s _ _ # r p c t o k e n = 2 2 0 8 5 0 5 5 5 & f o r c e s e c u r e = 1                 V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 n   3   < ! - - d y n a m i c F r a m e D A F F 2 B 3 3 9 1 9 7 D 2 C 4 9 E F F 0 1 D F 0 E 5 0 5 A C 0 - - >          8      0       8                                                                                                            `       X       Ë      H      ¿      †            ò      è~x è~x ÿ                          Ü  ?  h t t p s : / / c o n t a c t s . g o o g l e . c o m / w i d g e t / h o v e r c a r d / v / 2 ? o r i g i n = h t t p s : / / m a i l . g o o g l e . c o m & u s e g a p i = 1 & j s h = m ; / _ / s c s / a b c - s t a t i c / _ / j s / k % 3 D g a p i . g a p i . e n . B s t D m E h p a _ Q . O / r t % 3 D j / d % 3 D 1 / r s % 3 D A H p O o o - o q K d O J I c y A i 5 Y y A 6 p a h S k p _ o j s w / m % 3 D _ _ f e a t u r e s _ _ # i d = _ _ H C _ 9 4 2 5 3 2 2 9 & _ g f i d = _ _ H C _ 9 4 2 5 3 2 2 9 & p a r e n t = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m & p f n a m e = & r p c t o k e n = 5 6 5 1 8 9 1 6                 V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 n   3   < ! - - d y n a m i c F r a m e F C C C 8 6 2 A 4 4 6 4 7 7 1 0 1 B E C F D E A 2 6 4 6 2 A 1 7 - - >                 ‘  f  ˇˇo"ik"chSoFd0.8911456094217194"id"H1"url"øhttps://contacts.google.com/widget/hovercard/v/2?origin=https://mail.google.com&usegapi=1&jsh=m;/_/scs/abc-static/_/js/k%3Dgapi.gapi.en.BstDmEhpa_Q.O/rt%3Dj/d%3D1/rs%3DAHpOoo-oqKdOJIcyAi5YyA6pahSkp_ojsw/m%3D__features__#id=__HC_94253229&_gfid=__HC_94253229&parent=https%3A%2F%2Fmail.google.com&pfname=&rpctoken=56518916"tco"ik"chSoFd0.8911456094217194"Sc"V0"Oe"û/widget/hovercard/v/2?origin=https://mail.google.com&usegapi=1&jsh=m;/_/scs/abc-static/_/js/k%3Dgapi.gapi.en.BstDmEhpa_Q.O/rt%3Dj/d%3D1/rs%3DAHpOoo-oqKdOJIcyAi5YyA6pahSkp_ojsw/m%3D__features__#id=__HC_94253229&_gfid=__HC_94253229&parent=https://mail.google.com&pfname=&rpctoken=56518916"tc_{{            8      0       8                                                                                                            `       X       P      ∞                                  !è~x "è~x X      p                      Ù  h t t p s : / / h a n g o u t s . g o o g l e . c o m / w e b c h a t / u / 0 / l o a d ? c l i e n t = s m & p r o p = g m a i l & n a v = t r u e & f i d = g t n - r o s t e r - i f r a m e - i d & o s = W i n 3 2 & s t i m e = 1 5 3 9 8 5 7 9 3 6 2 7 4 & x p c = % 7 B % 2 2 c n % 2 2 % 3 A % 2 2 6 0 q s j 8 % 2 2 % 2 C % 2 2 t p % 2 2 % 3 A 1 % 2 C % 2 2 i f r i d % 2 2 % 3 A % 2 2 g t n - r o s t e r - i f r a m e - i d % 2 2 % 2 C % 2 2 p u % 2 2 % 3 A % 2 2 h t t p s % 3 A % 2 F % 2 F h a n g o u t s . g o o g l e . c o m % 2 F w e b c h a t % 2 F u % 2 F 0 % 2 F % 2 2 % 7 D & e c = % 5 B % 2 2 c i % 3 A e c % 2 2 % 2 C t r u e % 2 C t r u e % 2 C f a l s e % 5 D & p v t = A M P 3 u W a D 5 g l 7 l 0 K b k H n 5 V 7 k R s J 7 I p p 4 Z H I 5 h B C c j 6 j O f 2 W p H M D 5 c j 5 1 9 Y B 6 F 6 s R I o O a m p T A E 0 y t _ h 2 G 0 E Y r z o F F U T M J j w - P C W g % 3 D % 3 D & h r e f = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m % 2 F m a i l % 2 F u % 2 F 0 % 2 F % 3 F r e l % 3 D 1 % 2 6 p l i % 3 D 1 % 2 3 i n b o x & p o s = l & u i v = 2 & h l = e n & h p c = t r u e & h s m = t r u e & h r c = t r u e & p a l = 1 & u q p = f a l s e & g o o a = f a l s e & g l t s i = t r u e & g u s m = t r u e & s l = f a l s e & h s = % 5 B % 2 2 h _ h s % 2 2 % 2 C n u l l % 2 C n u l l % 2 C % 5 B 1 % 2 C 0 % 5 D % 5 D & m o l e h = 3 8 0 & m m o l e h = 3 6 & t w o = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m & h o s t = 1 & z x = d i u j e z 4 3 g l r 7               V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 n   3   < ! - - d y n a m i c F r a m e 7 1 9 7 5 7 6 0 1 C 9 0 6 D 5 F F 3 9 C D 2 A 9 3 7 1 E 0 4 A 8 - - >          8      0       8                                                                                                                   
      p      `       X       »      ¿	              0
             (
      ;è~x <è~x h
      Ä
                    h  ∞  h t t p s : / / 7 . c l i e n t - c h a n n e l . g o o g l e . c o m / c l i e n t - c h a n n e l / c l i e n t ? c f g = % 7 B % 2 2 2 % 2 2 % 3 A % 2 2 h a n g o u t s % 2 2 % 2 C % 2 2 6 % 2 2 % 3 A % 2 2 g m a i l % 2 2 % 2 C % 2 2 7 % 2 2 % 3 A % 2 2 c h a t _ f r o n t e n d _ 2 0 1 8 1 0 1 3 . 0 6 _ p 1 % 2 2 % 2 C % 2 2 8 % 2 2 % 3 A f a l s e % 2 C % 2 2 1 3 % 2 2 % 3 A f a l s e % 7 D & c t y p e = h a n g o u t s & x p c = % 7 B % 2 2 c n % 2 2 % 3 A % 2 2 N V s J U 6 A f e Y % 2 2 % 2 C % 2 2 t p % 2 2 % 3 A n u l l % 2 C % 2 2 o s h % 2 2 % 3 A n u l l % 2 C % 2 2 p p u % 2 2 % 3 A % 2 2 h t t p s % 3 A % 2 F % 2 F h a n g o u t s . g o o g l e . c o m % 2 F r o b o t s . t x t % 2 2 % 2 C % 2 2 l p u % 2 2 % 3 A % 2 2 h t t p s % 3 A % 2 F % 2 F 7 . c l i e n t - c h a n n e l . g o o g l e . c o m % 2 F r o b o t s . t x t % 2 2 % 7 D                 Ù  h t t p s : / / h a n g o u t s . g o o g l e . c o m / w e b c h a t / u / 0 / l o a d ? c l i e n t = s m & p r o p = g m a i l & n a v = t r u e & f i d = g t n - r o s t e r - i f r a m e - i d & o s = W i n 3 2 & s t i m e = 1 5 3 9 8 5 7 9 3 6 2 7 4 & x p c = % 7 B % 2 2 c n % 2 2 % 3 A % 2 2 6 0 q s j 8 % 2 2 % 2 C % 2 2 t p % 2 2 % 3 A 1 % 2 C % 2 2 i f r i d % 2 2 % 3 A % 2 2 g t n - r o s t e r - i f r a m e - i d % 2 2 % 2 C % 2 2 p u % 2 2 % 3 A % 2 2 h t t p s % 3 A % 2 F % 2 F h a n g o u t s . g o o g l e . c o m % 2 F w e b c h a t % 2 F u % 2 F 0 % 2 F % 2 2 % 7 D & e c = % 5 B % 2 2 c i % 3 A e c % 2 2 % 2 C t r u e % 2 C t r u e % 2 C f a l s e % 5 D & p v t = A M P 3 u W a D 5 g l 7 l 0 K b k H n 5 V 7 k R s J 7 I p p 4 Z H I 5 h B C c j 6 j O f 2 W p H M D 5 c j 5 1 9 Y B 6 F 6 s R I o O a m p T A E 0 y t _ h 2 G 0 E Y r z o F F U T M J j w - P C W g % 3 D % 3 D & h r e f = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m % 2 F m a i l % 2 F u % 2 F 0 % 2 F % 3 F r e l % 3 D 1 % 2 6 p l i % 3 D 1 % 2 3 i n b o x & p o s = l & u i v = 2 & h l = e n & h p c = t r u e & h s m = t r u e & h r c = t r u e & p a l = 1 & u q p = f a l s e & g o o a = f a l s e & g l t s i = t r u e & g u s m = t r u e & s l = f a l s e & h s = % 5 B % 2 2 h _ h s % 2 2 % 2 C n u l l % 2 C n u l l % 2 C % 5 B 1 % 2 C 0 % 5 D % 5 D & m o l e h = 3 8 0 & m m o l e h = 3 6 & t w o = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m & h o s t = 1 & z x = d i u j e z 4 3 g l r 7               n   3   < ! - - d y n a m i c F r a m e B 3 8 7 A 5 A 9 F 7 9 0 A 0 2 2 A A 2 A A 1 F 4 F 0 5 A D E 7 2 - - >          8      0       8                                                                                                            `       X       p      h              ÿ             –      5è~x 6è~x 	      (	                        h t t p s : / / c l i e n t s 4 . g o o g l e . c o m / i n v a l i d a t i o n / l c s / c l i e n t ? x p c = % 7 B % 2 2 c n % 2 2 % 3 A % 2 2 u v l 3 R r 0 n K 5 % 2 2 % 2 C % 2 2 t p % 2 2 % 3 A n u l l % 2 C % 2 2 o s h % 2 2 % 3 A n u l l % 2 C % 2 2 p p u % 2 2 % 3 A % 2 2 h t t p s % 3 A % 2 F % 2 F h a n g o u t s . g o o g l e . c o m % 2 F r o b o t s . t x t % 2 2 % 2 C % 2 2 l p u % 2 2 % 3 A % 2 2 h t t p s % 3 A % 2 F % 2 F c l i e n t s 4 . g o o g l e . c o m % 2 F r o b o t s . t x t % 2 2 % 7 D                 Ù  h t t p s : / / h a n g o u t s . g o o g l e . c o m / w e b c h a t / u / 0 / l o a d ? c l i e n t = s m & p r o p = g m a i l & n a v = t r u e & f i d = g t n - r o s t e r - i f r a m e - i d & o s = W i n 3 2 & s t i m e = 1 5 3 9 8 5 7 9 3 6 2 7 4 & x p c = % 7 B % 2 2 c n % 2 2 % 3 A % 2 2 6 0 q s j 8 % 2 2 % 2 C % 2 2 t p % 2 2 % 3 A 1 % 2 C % 2 2 i f r i d % 2 2 % 3 A % 2 2 g t n - r o s t e r - i f r a m e - i d % 2 2 % 2 C % 2 2 p u % 2 2 % 3 A % 2 2 h t t p s % 3 A % 2 F % 2 F h a n g o u t s . g o o g l e . c o m % 2 F w e b c h a t % 2 F u % 2 F 0 % 2 F % 2 2 % 7 D & e c = % 5 B % 2 2 c i % 3 A e c % 2 2 % 2 C t r u e % 2 C t r u e % 2 C f a l s e % 5 D & p v t = A M P 3 u W a D 5 g l 7 l 0 K b k H n 5 V 7 k R s J 7 I p p 4 Z H I 5 h B C c j 6 j O f 2 W p H M D 5 c j 5 1 9 Y B 6 F 6 s R I o O a m p T A E 0 y t _ h 2 G 0 E Y r z o F F U T M J j w - P C W g % 3 D % 3 D & h r e f = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m % 2 F m a i l % 2 F u % 2 F 0 % 2 F % 3 F r e l % 3 D 1 % 2 6 p l i % 3 D 1 % 2 3 i n b o x & p o s = l & u i v = 2 & h l = e n & h p c = t r u e & h s m = t r u e & h r c = t r u e & p a l = 1 & u q p = f a l s e & g o o a = f a l s e & g l t s i = t r u e & g u s m = t r u e & s l = f a l s e & h s = % 5 B % 2 2 h _ h s % 2 2 % 2 C n u l l % 2 C n u l l % 2 C % 5 B 1 % 2 C 0 % 5 D % 5 D & m o l e h = 3 8 0 & m m o l e h = 3 6 & t w o = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m & h o s t = 1 & z x = d i u j e z 4 3 g l r 7               n   3   < ! - - d y n a m i c F r a m e C 0 9 8 8 7 D 3 7 8 2 1 6 A 4 2 9 7 E 3 3 4 9 4 6 2 5 2 5 5 0 D - - >          8      0       8                                                                                                            `       X       †      ò	              
              
      9è~x :è~x @
      X
                    :  ô  h t t p s : / / h a n g o u t s . g o o g l e . c o m / h a n g o u t s / _ / h s c v ? p v t = A M P 3 u W a D 5 g l 7 l 0 K b k H n 5 V 7 k R s J 7 I p p 4 Z H I 5 h B C c j 6 j O f 2 W p H M D 5 c j 5 1 9 Y B 6 F 6 s R I o O a m p T A E 0 y t _ h 2 G 0 E Y r z o F F U T M J j w - P C W g % 3 D % 3 D & w i n U r l = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m & a u t h u s e r = 0 & x p c = % 7 B % 2 2 c n % 2 2 % 3 A % 2 2 G 3 n 8 D Q 1 n L a % 2 2 % 2 C % 2 2 t p % 2 2 % 3 A n u l l % 2 C % 2 2 o s h % 2 2 % 3 A n u l l % 2 C % 2 2 p p u % 2 2 % 3 A % 2 2 h t t p s % 3 A % 2 F % 2 F h a n g o u t s . g o o g l e . c o m % 2 F r o b o t s . t x t % 2 2 % 2 C % 2 2 l p u % 2 2 % 3 A % 2 2 h t t p s % 3 A % 2 F % 2 F h a n g o u t s . g o o g l e . c o m % 2 F r o b o t s . t x t % 2 2 % 7 D                       Ù  h t t p s : / / h a n g o u t s . g o o g l e . c o m / w e b c h a t / u / 0 / l o a d ? c l i e n t = s m & p r o p = g m a i l & n a v = t r u e & f i d = g t n - r o s t e r - i f r a m e - i d & o s = W i n 3 2 & s t i m e = 1 5 3 9 8 5 7 9 3 6 2 7 4 & x p c = % 7 B % 2 2 c n % 2 2 % 3 A % 2 2 6 0 q s j 8 % 2 2 % 2 C % 2 2 t p % 2 2 % 3 A 1 % 2 C % 2 2 i f r i d % 2 2 % 3 A % 2 2 g t n - r o s t e r - i f r a m e - i d % 2 2 % 2 C % 2 2 p u % 2 2 % 3 A % 2 2 h t t p s % 3 A % 2 F % 2 F h a n g o u t s . g o o g l e . c o m % 2 F w e b c h a t % 2 F u % 2 F 0 % 2 F % 2 2 % 7 D & e c = % 5 B % 2 2 c i % 3 A e c % 2 2 % 2 C t r u e % 2 C t r u e % 2 C f a l s e % 5 D & p v t = A M P 3 u W a D 5 g l 7 l 0 K b k H n 5 V 7 k R s J 7 I p p 4 Z H I 5 h B C c j 6 j O f 2 W p H M D 5 c j 5 1 9 Y B 6 F 6 s R I o O a m p T A E 0 y t _ h 2 G 0 E Y r z o F F U T M J j w - P C W g % 3 D % 3 D & h r e f = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m % 2 F m a i l % 2 F u % 2 F 0 % 2 F % 3 F r e l % 3 D 1 % 2 6 p l i % 3 D 1 % 2 3 i n b o x & p o s = l & u i v = 2 & h l = e n & h p c = t r u e & h s m = t r u e & h r c = t r u e & p a l = 1 & u q p = f a l s e & g o o a = f a l s e & g l t s i = t r u e & g u s m = t r u e & s l = f a l s e & h s = % 5 B % 2 2 h _ h s % 2 2 % 2 C n u l l % 2 C n u l l % 2 C % 5 B 1 % 2 C 0 % 5 D % 5 D & m o l e h = 3 8 0 & m m o l e h = 3 6 & t w o = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m & h o s t = 1 & z x = d i u j e z 4 3 g l r 7               n   3   < ! - - d y n a m i c F r a m e 1 A 6 8 F 5 9 2 C 3 5 5 5 7 3 0 9 A D E 6 E E 0 0 D 2 F 9 A D 1 - - >          8      0       8                                                                                                            `       X              `              –             »      è~x  è~x                            û  K  h t t p s : / / h a n g o u t s . g o o g l e . c o m / h a n g o u t s / _ / h s c v ? p v t = A M P 3 u W a D 5 g l 7 l 0 K b k H n 5 V 7 k R s J 7 I p p 4 Z H I 5 h B C c j 6 j O f 2 W p H M D 5 c j 5 1 9 Y B 6 F 6 s R I o O a m p T A E 0 y t _ h 2 G 0 E Y r z o F F U T M J j w - P C W g % 3 D % 3 D & a u t h u s e r = 0 & x p c = % 7 B % 2 2 c n % 2 2 % 3 A % 2 2 X O j x z U Z 8 0 8 % 2 2 % 2 C % 2 2 p p u % 2 2 % 3 A % 2 2 h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m % 2 F r o b o t s . t x t % 2 2 % 2 C % 2 2 l p u % 2 2 % 3 A % 2 2 h t t p s % 3 A % 2 F % 2 F h a n g o u t s . g o o g l e . c o m % 2 F r o b o t s . t x t % 2 2 % 7 D                 V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 n   3   < ! - - d y n a m i c F r a m e 8 8 B A 0 6 E 9 D 4 0 C 2 C 2 3 C A 7 7 8 A 9 A 0 B 7 A 2 2 F 5 - - >          8      0       8                                                                                                            `       X       8      ò                              #è~x $è~x P      h                    “  e  h t t p s : / / n o t i f i c a t i o n s . g o o g l e . c o m / u / 0 / w i d g e t ? s o u r c e i d = 2 3 & h l = e n & o r i g i n = h t t p s : / / m a i l . g o o g l e . c o m & u c = 1 & u s e g a p i = 1 & j s h = m ; / _ / s c s / a b c - s t a t i c / _ / j s / k % 3 D g a p i . g a p i . e n . B s t D m E h p a _ Q . O / r t % 3 D j / d % 3 D 1 / r s % 3 D A H p O o o - o q K d O J I c y A i 5 Y y A 6 p a h S k p _ o j s w / m % 3 D _ _ f e a t u r e s _ _ & p l i = 1 # p i d = 2 3 & _ m e t h o d s = o n E r r o r % 2 C o n I n f o % 2 C h i d e N o t i f i c a t i o n W i d g e t % 2 C p o s t S h a r e d M e s s a g e % 2 C r e a u t h % 2 C s e t N o t i f i c a t i o n W i d g e t H e i g h t % 2 C s e t N o t i f i c a t i o n W i d g e t S i z e % 2 C s w i t c h T o % 2 C n a v i g a t e T o % 2 C s e t N o t i f i c a t i o n T e x t % 2 C s e t N o t i f i c a t i o n A n i m a t i o n % 2 C g e t N o t i f i c a t i o n T e x t % 2 C v a l i d a t e U s e r % 2 C _ r e a d y & i d = I 0 _ 1 5 3 9 8 5 7 9 3 5 8 7 1 & _ g f i d = I 0 _ 1 5 3 9 8 5 7 9 3 5 8 7 1 & p a r e n t = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m & p f n a m e = & r p c t o k e n = 5 6 8 4 7 7 5 5                     V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 n   3   < ! - - d y n a m i c F r a m e F 9 2 4 E 2 B B 1 4 C 7 C A 2 9 4 D 6 6 A B 3 8 B F 0 B 3 4 F F - - >                    |  ˇˇo"VC"chSoFd0.5183289432457807"id"H1"url"Âhttps://notifications.google.com/u/0/widget?sourceid=23&hl=en&origin=https://mail.google.com&uc=1&usegapi=1&jsh=m;/_/scs/abc-static/_/js/k%3Dgapi.gapi.en.BstDmEhpa_Q.O/rt%3Dj/d%3D1/rs%3DAHpOoo-oqKdOJIcyAi5YyA6pahSkp_ojsw/m%3D__features__&pli=1#pid=23&_methods=onError%2ConInfo%2ChideNotificationWidget%2CpostSharedMessage%2Creauth%2CsetNotificationWidgetHeight%2CsetNotificationWidgetSize%2CswitchTo%2CnavigateTo%2CsetNotificationText%2CsetNotificationAnimation%2CgetNotificationText%2CvalidateUser%2C_ready&id=I0_1539857935871&_gfid=I0_1539857935871&parent=https%3A%2F%2Fmail.google.com&pfname=&rpctoken=56847755"ugo"VC"chSoFd0.5183289432457807"sh"V0"zn"•/u/0/widget?sourceid=23&hl=en&origin=https://mail.google.com&uc=1&usegapi=1&jsh=m;/_/scs/abc-static/_/js/k%3Dgapi.gapi.en.BstDmEhpa_Q.O/rt%3Dj/d%3D1/rs%3DAHpOoo-oqKdOJIcyAi5YyA6pahSkp_ojsw/m%3D__features__&pli=1#pid=23&_methods=onError,onInfo,hideNotificationWidget,postSharedMessage,reauth,setNotificationWidgetHeight,setNotificationWidgetSize,switchTo,navigateTo,setNotificationText,setNotificationAnimation,getNotificationText,validateUser,_ready&id=I0_1539857935871&_gfid=I0_1539857935871&parent=https://mail.google.com&pfname=&rpctoken=56847755"ug_{{       8      0       8                                                                                                            `       X       à      Ë              X             P      +è~x ,è~x ê      ®                    $    h t t p s : / / h a n g o u t s . g o o g l e . c o m / w e b c h a t / u / 0 / f r a m e 2 ? v = 1 5 3 9 7 8 6 8 3 8 & p v t = A M P 3 u W a D 5 g l 7 l 0 K b k H n 5 V 7 k R s J 7 I p p 4 Z H I 5 h B C c j 6 j O f 2 W p H M D 5 c j 5 1 9 Y B 6 F 6 s R I o O a m p T A E 0 y t _ h 2 G 0 E Y r z o F F U T M J j w - P C W g % 3 D % 3 D & p r o p = g m a i l & h l = e n & f i d = g t n - r o s t e r - i f r a m e - i d # e % 5 B % 2 2 w b l h 0 . 6 0 3 4 1 6 8 5 8 4 9 1 1 3 0 1 - 0 % 2 2 , 2 , 1 , % 5 B t r u e , % 5 B % 5 D % 5 D % 5 D                   V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 n   3   < ! - - d y n a m i c F r a m e 4 B 2 E A F E C 0 6 2 6 E 8 C 8 D 3 E 6 2 4 3 B 6 9 A 4 5 E 6 A - - >          8      0       8                                                                                                            `       X       ò      ¯              h             `      -è~x .è~x †      ∏                    2    h t t p s : / / h a n g o u t s . g o o g l e . c o m / w e b c h a t / u / 0 / f r a m e 2 ? v = 1 5 3 9 7 8 6 8 3 8 & p v t = A M P 3 u W a D 5 g l 7 l 0 K b k H n 5 V 7 k R s J 7 I p p 4 Z H I 5 h B C c j 6 j O f 2 W p H M D 5 c j 5 1 9 Y B 6 F 6 s R I o O a m p T A E 0 y t _ h 2 G 0 E Y r z o F F U T M J j w - P C W g % 3 D % 3 D & p r o p = g m a i l & h l = e n & f i d = g t n - r o s t e r - i f r a m e - i d # e % 5 B % 2 2 w b l h 0 . 6 0 3 4 1 6 8 5 8 4 9 1 1 3 0 1 - 1 % 2 2 , 2 , 1 , % 5 B n u l l , % 5 B 2 , 3 , 4 , 5 % 5 D % 5 D % 5 D                     V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 n   3   < ! - - d y n a m i c F r a m e 6 F B 7 5 7 5 B C 6 F B 8 9 B 8 3 D 8 A 6 8 E 6 0 8 1 9 A 7 7 C - - >          8      0       8                                                                                                            `       X       Ë      H              ∏             ∞      7è~x 8è~x                           Ü  ø   h t t p s : / / h a n g o u t s . g o o g l e . c o m / w e b c h a t / u / 0 / f r a m e ? v = 1 5 3 9 7 8 6 8 3 8 & h l = e n & p v t = A M P 3 u W a D 5 g l 7 l 0 K b k H n 5 V 7 k R s J 7 I p p 4 Z H I 5 h B C c j 6 j O f 2 W p H M D 5 c j 5 1 9 Y B 6 F 6 s R I o O a m p T A E 0 y t _ h 2 G 0 E Y r z o F F U T M J j w - P C W g % 3 D % 3 D & p r o p = g m a i l # e p r e l d                 V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 n   3   < ! - - d y n a m i c F r a m e D D D 6 6 1 9 3 6 4 B 8 8 D B 2 9 3 7 A 1 A 2 D 3 7 0 2 F A 9 4 - - >          8      0       8                                                                                                            `       X               x               Ë              ‡       =è~x >è~x        8                          a b o u t : b l a n k                 n   3   < ! - - d y n a m i c F r a m e 6 C 5 D 4 D E 5 1 1 5 E 4 6 3 6 E 7 9 0 F C B C 1 A 6 7 5 0 D A - - >          8      0       8                                                                                                            `       X               ‡               P             H      íMº~x ìMº~x à      †                    Ñ   >   c h r o m e - e x t e n s i o n : / / p n n f e m g p i l p d a o j p n k j d g f g b n n j o j f i k / b l a n k . h t m l                   n   3   < ! - - d y n a m i c F r a m e 7 2 9 1 1 4 E 0 9 9 7 4 2 4 6 C 6 3 E 5 F B 8 7 C D 5 E 0 F 6 E - - >          8      0       8                                                                                                                  `       X               –               @             8      YF~x ZF~x x      ê                    x   8   h t t p s : / / m a i l f o o g a e . a p p s p o t . c o m / i F r a m e s / w o r k e r I F r a m e . h t m l               n   3   < ! - - d y n a m i c F r a m e 3 3 4 4 0 9 7 7 B D B A 6 2 0 1 0 2 E B 9 4 B 2 9 7 C E 8 4 8 A - - >          8      0       8                                                                                                            `       X               x               Ë              ‡       Aè~x Bè~x        8                          a b o u t : b l a n k                 n   3   < ! - - d y n a m i c F r a m e E 0 8 D 7 7 6 A C 9 8 4 F 6 3 C B B 2 B F E 2 8 E 0 C 7 0 E 8 6 - - >          8      0       8                                                                                                            `       X               ‡               P             H      ñMº~x óMº~x à      †                    Ñ   >   c h r o m e - e x t e n s i o n : / / p n n f e m g p i l p d a o j p n k j d g f g b n n j o j f i k / b l a n k . h t m l                   n   3   < ! - - d y n a m i c F r a m e 4 2 B 6 4 8 0 5 4 E E B 1 A C E E F 1 2 A 9 F D 5 8 5 0 8 C 8 C - - >          8      0       8                                                                                                                  `       X               ∏               (                    „{ä~x „{ä~x `      x                    Z   )   h t t p s : / / h e a d w a y - w i d g e t . n e t / w i d g e t s / J 1 4 O g 7                     n   3   < ! - - d y n a m i c F r a m e D A 8 B 4 6 6 7 7 1 F 3 8 2 E 4 F F C C 7 3 B 0 B B 2 B 2 0 E 8 - - >          8      0       8                                                                                                                           N   https://mail.google.com/mail/u/0/?pli=1#inbox/FMfcgxvzLDzqgfnDpKQFsrwKgDhLqxfk      ö≤f◊.     »          yete  ù˝   N   https://mail.google.com/mail/u/0/?pli=1#inbox/FMfcgxvzLDzqgfnDpKQFsrwKgDhLqxfk  5   s o c c e r b o t s   p r i n t   -   p s v s s a i k u m a r 1 9 9 8 @ g m a i l . c o m   -   G m a i l   $d   d     d                              `       X                                           ∞      %è~x è~x                           §   N   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1 # i n b o x / F M f c g x v z L D z q g f n D p K Q F s r w K g D h L q x f k                          p      h       ÿ       ¯                    8      P      `      x      ê      ∏      –      Ë                    f   /   
  ? %   B l i n k   s e r i a l i z e d   f o r m   s t a t e   v e r s i o n   9   
  = &                       N o   o w n e r               
      3                                                t e x t               
      0                                                t e x t               
      0                        
   h i s t _ s t a t e                         t e x t               
      1                     T   &   i n b o x / F M f c g x v z L D z q g f n D p K Q F s r w K g D h L q x f k     8      0       8             ?                                                                                      Ä      x       ê      ®      8      @
      à      à8       =      ¿H      ¿L      –P      0T      ¿U      ∞Y      @[      `       X       Ë       H              p             h      è~x è~x ®      ¿                    à   @   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? u i = 2 & v i e w = b s p & v e r = o h h l 4 r w 8 m b n 4               V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 $      s 1 b m q d y j p s a o q x            8      0       8                                                                                                            `       X       Ë       H              p             h      è~x è~x ®      ¿                    à   @   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? u i = 2 & v i e w = b s p & v e r = o h h l 4 r w 8 m b n 4               V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 $      h 1 b m q d y j p s a o q x            8      0       8                                                                                                            `       X               x               Ë              ‡       è~x è~x        8                          a b o u t : b l a n k                 n   3   < ! - - d y n a m i c F r a m e 5 9 F 3 2 F B 7 6 9 C E C 3 C 7 6 7 A 2 4 E E 1 7 A 2 B 8 1 2 B - - >          8      0       8                                                                                                            `       X       ê                    `             X      w"~x w"~x ò      ∞                    .    h t t p s : / / a c c o u n t s . g o o g l e . c o m / o / o a u t h 2 / p o s t m e s s a g e R e l a y ? p a r e n t = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m & j s h = m % 3 B % 2 F _ % 2 F s c s % 2 F a b c - s t a t i c % 2 F _ % 2 F j s % 2 F k % 3 D g a p i . g a p i . e n . B s t D m E h p a _ Q . O % 2 F r t % 3 D j % 2 F d % 3 D 1 % 2 F r s % 3 D A H p O o o - o q K d O J I c y A i 5 Y y A 6 p a h S k p _ o j s w % 2 F m % 3 D _ _ f e a t u r e s _ _ # r p c t o k e n = 2 2 0 8 5 0 5 5 5 & f o r c e s e c u r e = 1                 V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 n   3   < ! - - d y n a m i c F r a m e D A F F 2 B 3 3 9 1 9 7 D 2 C 4 9 E F F 0 1 D F 0 E 5 0 5 A C 0 - - >          8      0       8                                                                                                            `       X       Ë      H      ¿      †            ò      è~x è~x ÿ                          Ü  ?  h t t p s : / / c o n t a c t s . g o o g l e . c o m / w i d g e t / h o v e r c a r d / v / 2 ? o r i g i n = h t t p s : / / m a i l . g o o g l e . c o m & u s e g a p i = 1 & j s h = m ; / _ / s c s / a b c - s t a t i c / _ / j s / k % 3 D g a p i . g a p i . e n . B s t D m E h p a _ Q . O / r t % 3 D j / d % 3 D 1 / r s % 3 D A H p O o o - o q K d O J I c y A i 5 Y y A 6 p a h S k p _ o j s w / m % 3 D _ _ f e a t u r e s _ _ # i d = _ _ H C _ 9 4 2 5 3 2 2 9 & _ g f i d = _ _ H C _ 9 4 2 5 3 2 2 9 & p a r e n t = h t t p s % 3 A % 2 F % 2 F m a i l . g o o g l e . c o m & p f n a m e = & r p c t o k e n = 5 6 5 1 8 9 1 6                 V   '   h t t p s : / / m a i l . g o o g l e . c o m / m a i l / u / 0 / ? p l i = 1                 n   3   < ! - - d y n a m i c F r a m e F C C C 8 6 2 A 4 4 6 4 7 7 1 0 1 B E C F D E A 2 6 4 6 2 A 1 7 - - >                 ‘  f  ˇˇo"ik"chSoFd0.8911456094217194"id"H1"url"øhttps://contacts.google.com/widget/hovercard/v/2?origin=https://mail.google.com&usegapi=1&jsh=m;/_/scs/abc-static/_/js/k%3Dgapi.gapi.en.BstDmEhpa_Q.O/rt%3Dj/d%3D1/rs%3DAHpOoo-oqKdOJIcyAi5YyA6pahSkp_ojsw/m%3D__features__#id=__HC_94253229&_gfid=__HC_94253229&parent=https%3A%2F%2Fmail.google.com&pfname=&rpctoken=56518916"tco"ik"chSoFd0.8911456094217194"Sc"V0"Oe"û/widget/hovercard/v/2?origin=https://mail.google.com&usegapi=1&jsh=m;/_/scs/abc-static/_/js/k%3Dgapi.gapi.en.BstDmEhpa_Q.O/rt%3Dj/d%3D1/rs%3DAHpOoo-oqKdOJIcyAi5YyA6pahSkp_ojsw/m%3D__features__#id=__HC_94253229&_gfid=__HC_94253229&parent=https://mail.google.com&pfname=&rpctoken=56518916"tc_{{            8      0       8                                                                                                            `       X       P      ∞                                  !è~x "è~x X      p                      Ù  h t t p s : / / h a n g o u t s . g o o g l e . c o m / w e b c h a t / u / 0 / l o a d ? c l i e n t = s m & p r o p = g m a i l & n a v = t r u e & f i d = g t n - r o s t e r - i f r a m e - i d & o s = W i n 3 2 & s t i m e = 1 5 3 9 8 5 7 9 3 6 2 7 4 & x p c = % 7 B % 2 2 c n % 2 2 % 3 A % 2 2 6 0 q s j 8 % 2 2 % 2 C % 2 2 t p % 2 2 % 3 A 1 % 2 C % 2 2 i f r i d % 2 2 % 3 A % 2 2 g t n - r o s t e r - i f r a m e - i d % 2 2 % 2 C % 2 2 p u % 2 2 % 3 A % 2 2 h t t p s % 3 A % 2 F % 2 F h a n g o u t s . g o o g l e . c o m % 2 F w e b c h a t % 2 F u % 2 F 0 % 2 F % 2 2 % 7 D & e c = % 5 B % 2 2 c i % 3 A e c % 2 2 % 2 C t r u e % 2 C t r u e % 2 C f a l s e % 5 D & p v t = A M P 3 u W a D 5 g l 7 l 0 K b k H n 5 V 7 k R s J 7 I p p 4 Z H I 5 h 