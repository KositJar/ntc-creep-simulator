//最終更新2000年12月7日
#if !defined ___C3comp
#define ___C3comp

#include "GTE.h"

class C3comp:protected GTE{
public:
	class Value NextStep(class Value Prev_Step,class Control CurrCTR,ofstream& LOGF);
	
protected:
	virtual void Calc()=0;
	virtual double Equation(double x)=0;
	bool SolveEq(double &x,double Scale);
	ofstream* LOGFile;

private:

};

class Value  C3comp::NextStep(class Value Prev_Step,class Control CurrCTR,ofstream& LOGF){

	LOGFile=&LOGF;

	CTR=CurrCTR;
	Prev=Prev_Step;
	Curr=Prev;
	Curr.time=Prev.time+CTR.dt;

	Calc();

	if(!Curr.result){
			return(Curr);
	}

	//temporal
	Curr.Sigma_fy=Sigma_fy(Curr.ep,Curr.ep_ir);

	Curr.Param.history.RecHistory(DSigma_V(Curr.ep,Curr.ep_ir,Curr.epd_ir),DSigma_fy(Curr.ep,Curr.ep_ir),getDtau(Prev.epd_ir,(Prev.epd_ir+Curr.epd_ir)/2,CTR.dt/2)+Prev.tau);

	Curr.result=true;

	return(Curr);
}

/**********************************************************************
 *   Routine to solve an equation                                     *
 *   (Assuming the function is monotonically incresing or decreasing.)*
 **********************************************************************/
	//&x Start of iteration
	// SolveEq is true if |Equation(x)| < Presicion * Scale
	// within MaxIteration times.
	//We will find two values for x, so that "x1 < (real x) < x2".
    //We put one of them to be epd_ir_prev. The other is ......
bool C3comp::SolveEq(double &x,double Scale){
	double x1,x2, Fx1,Fx2;
	int i=0;

	x1 = x;
	Fx1 = Equation(x1);
	{
		double d,slope;

		if (x != 0.0){
			d = Equation(1.001 * x1);
			slope = (d - Fx1) / (0.001 * x1);
		}else{
			d = Equation(0.0001);// What value should be here ??
			slope = (d - Fx1) / (0.0001);
		}
		slope = slope/2;
		x2 = x1 - Fx1/slope;
		Fx2 = Equation(x2);
	};

	if (x1 > x2) {
		double x;
		
		x =  x1;  x1 =  x2;  x2 = x;
		x = Fx1; Fx1 = Fx2; Fx2 = x;
	}

	/* if Fx1*Fx2 >0 */
	if (Fx1 * Fx2 > 0){
		double x20, Fx20, step;
		x20 = x2; Fx20 = Fx2;
		cout << "Equation search start....\n\n";
		step = (x2 - x1)/100; 
		if(fabs(step)<0.0001){
			step=0.0001;
		}

		do{
			x2 = x2 + step; Fx2 = Equation(x2);
			x1 = x1 - step; Fx1 = Equation(x1);	
			i++;
		}while(Fx1 * Fx2 > 0);

		if (Fx1 * Fx2 > 0) {x2 = x20; Fx2 = Fx20;};
	};
	
	/* Then iteration starts */
	int    count = 0;
	double xmid,Fxmid; double ErrRatio;
	
	do{
		xmid = (x1 + x2)/2;
		Fxmid = Equation(xmid);
		if (Fxmid * Fx1 > 0) {
			x1 = xmid; Fx1 = Fxmid;
		}else {
			x2 = xmid; Fx2 = Fxmid;
		};
		if(Scale == 0){
			ErrRatio = fabs(Fxmid);
		}else {
			ErrRatio = fabs(Fxmid) / Scale;
		};
		count++;
		if(count > Curr.Param.MaxIteration){
			return(false);
		};
	}while (ErrRatio > Curr.Param.Precision);
	(*LOGFile) << Curr.time << "\t" << i << "\t" << count << "\n" ;
	x = xmid;


/*	int	count = 0;
	double xans,Fans; double ErrRatio;
		
	do{
		xans = x2 - Fx2*(x2-x1)/(Fx2-Fx1);
		Fans = Equation(xans);
		x1 = x2; Fx1 = Equation(x1);
		x2 = xans; Fx2 = Equation(x2);
		
		if(Scale == 0){
			ErrRatio = fabs(Fans);
		}else {
			ErrRatio = fabs(Fans) / Scale;
		};
		count++;
		if(count > Curr.Param.MaxIteration){
			return(false);
		};
	}while (ErrRatio > Curr.Param.Precision);
	(*LOGFile) << Curr.time << "\t" << i << "\t" << count << "\n" ;
	x = xans;*/

	return(true);
};

	
#endif