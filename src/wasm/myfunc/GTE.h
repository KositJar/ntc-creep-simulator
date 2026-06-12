//最終更新2000年12月7日
#if !defined ___GTE
#define ___GTE

#if !defined ___PI
#define ___PI 3.14159265358979
#endif

#include<math.h>

#include "History.h"
#include "Control.h"
#include "Parameter.h"
#include "Value.h"

class GTE{
public:

protected:
	class Value Prev, Curr;
	class Control CTR;
	//Total Stress
	double Sigma_t(double ep_c,double ep_ir_c);
	//Inviscid Stress
	double Sigma_f(double ep,double ep_ir);
	double Sigma_fy(double ep,double ep_ir);
	double Sigma_f0(double ep,double ep_ir);
	double Sigma_f00(double ep_0);
	//Viscous Stress
	double Sigma_V(double Dtau,double ep_c,double ep_ir_c,double epd_ir_c);
	double DSigma_V(double ep_c,double ep_ir_c,double epd_ir_c);
	//Elastic
	double Sigma_to_ep_e(double Sigma);
	double ep_e_to_Sigma(double ep_e);
	//Irreversible Strain Increment
	double getDtau(double epd_prev, double epd_curr, double dt);
	//Aging
	double DSigma_fy(double ep,double ep_ir);
private:
	//Aging
	double Sigma_fy0(double ep,double ep_ir);
	double A_f(double time);
	double alpha_f(double time);
	double Decay_f(double tau_c,double tau_i,double r);
	double Fr_f(double ep_ir);
	//Viscous Stress
	double Sigma_V_TESRA(double Dtau,double ep_c,double ep_ir_c,double epd_ir_c);
	double Sigma_V_ISOTACH(double ep_c,double ep_ir_c,double epd_ir_c);
	double Fr(double ep_ir);
	double Fgv(double epd_ir);
	double Fgdecay(double tau_c,double tau_i,double r);
	double Ftheta(double ep_ir);

};

//############
//TOTAL STRESS
//############
//Viscous Stress + Inviscid Stress (Strain ,Irreversible Strain)
double GTE::Sigma_t(double ep_c,double ep_ir_c){
    double result;
    double Dtau;
    double epd_ir_c;

	epd_ir_c=((ep_ir_c-Prev.ep_ir)*2)/CTR.dt-Prev.epd_ir;
	Dtau=getDtau(Prev.epd_ir,epd_ir_c,CTR.dt);    
	result=Sigma_V(Dtau,ep_c,ep_ir_c,epd_ir_c)+Sigma_f(ep_c,ep_ir_c);

    return(result);
}



//###############
//Inviscid Stress
//###############
//Reference Curve
double GTE::Sigma_f(double ep,double ep_ir){ 
	double result;

	result=Sigma_fy(ep,ep_ir);

	return(result);
}


//Sigma_v=gv*Sigma_f0
double GTE::Sigma_f0(double ep,double ep_ir){ 
//	double ep_0;
	double result;
	double ProportionX=1,ProportionY=1;
	
//	ep_0=(double)Curr.Param.DirectionX/ProportionX*(ep_ir-Curr.Param.OriginX);
//	result=ProportionY*Sigma_f00(ep_0);
	
	result=Sigma_fy(ep,ep_ir)-Curr.Param.OriginY0;

	return(result);
};

//Sigma_fの関数形
double GTE::Sigma_f00(double ep_0){ 
	double result;
/*	result=
		Curr.Param.RefParam[4]+
		Curr.Param.RefParam[5]*Curr.Param.RefParam[0]*(1-exp(-ep_0/(Curr.Param.RefParam[1]*Curr.Param.RefParam[6])))+
		Curr.Param.RefParam[5]*Curr.Param.RefParam[2]*(1-exp(-ep_0/(Curr.Param.RefParam[3]*Curr.Param.RefParam[6])));
	
	result=
		Curr.Param.RefParam[0]+
		Curr.Param.RefParam[1]*ep_0+
		Curr.Param.RefParam[2]*ep_0*ep_0+
		Curr.Param.RefParam[3]*ep_0*ep_0*ep_0+
		Curr.Param.RefParam[4]*ep_0*ep_0*ep_0*ep_0+
		Curr.Param.RefParam[5]*ep_0*ep_0*ep_0*ep_0*ep_0+
		Curr.Param.RefParam[6]*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0+
		Curr.Param.RefParam[7]*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0+
		Curr.Param.RefParam[8]*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0+
		Curr.Param.RefParam[9]*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0;


	//in Mr.ISHIHARA's Master Thesis
	//Param4:R0 Param5:CR Param0:A1 Param1:t1 Param6:Cep Param2:A2 Param3:t2

	return(result);
*/


	if(Curr.Param.RefFunction==0){	//Polynomial function
		result=	Curr.Param.RefParam[0]+
				Curr.Param.RefParam[1]*ep_0+
				Curr.Param.RefParam[2]*ep_0*ep_0+
				Curr.Param.RefParam[3]*ep_0*ep_0*ep_0+
				Curr.Param.RefParam[4]*ep_0*ep_0*ep_0*ep_0+
				Curr.Param.RefParam[5]*ep_0*ep_0*ep_0*ep_0*ep_0+
				Curr.Param.RefParam[6]*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0+
				Curr.Param.RefParam[7]*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0+
				Curr.Param.RefParam[8]*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0+
				Curr.Param.RefParam[9]*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0*ep_0;
	}
	else if(Curr.Param.RefFunction==1){	//Exponential function
		result=	Curr.Param.RefParam[4]+
				Curr.Param.RefParam[5]*Curr.Param.RefParam[0]*(1-exp(-ep_0/(Curr.Param.RefParam[1]*Curr.Param.RefParam[6])))+
				Curr.Param.RefParam[5]*Curr.Param.RefParam[2]*(1-exp(-ep_0/(Curr.Param.RefParam[3]*Curr.Param.RefParam[6])));
	}
	else if(Curr.Param.RefFunction==2){	//Bilinear for EPS
		result=	Curr.Param.RefParam[0]+
				(Curr.Param.RefParam[1]+Curr.Param.RefParam[2]*ep_0)*
				(1-exp(-Curr.Param.RefParam[3]*pow(ep_0,Curr.Param.RefParam[4])-Curr.Param.RefParam[5]*ep_0/Curr.Param.RefParam[1]));
	}
	else if(Curr.Param.RefFunction==3){	//direct shear1 using pulse equation mixed with y = a x^b
		result=	Curr.Param.RefParam[0]+
				Curr.Param.RefParam[2]*pow((1.0-exp(-(ep_0-Curr.Param.RefParam[1])/Curr.Param.RefParam[3])),Curr.Param.RefParam[4])*
				exp(-(ep_0-Curr.Param.RefParam[1])/Curr.Param.RefParam[5])+Curr.Param.RefParam[6]*pow(ep_0,Curr.Param.RefParam[7]);
	}
	else if(Curr.Param.RefFunction==4){	//direct shear2 using Tanaka's model
		result=	pow(((2.0*pow((Curr.Param.RefParam[2]*ep_0),0.5))/(Curr.Param.RefParam[2]+ep_0)),Curr.Param.RefParam[4])*
					Curr.Param.RefParam[0]*(1.0/(1.0+exp(-2.0*Curr.Param.RefParam[5]*(ep_0-Curr.Param.RefParam[6])))-1.0/(1.0+exp(-2.0*Curr.Param.RefParam[5]*(ep_0-Curr.Param.RefParam[2])))) +
					(Curr.Param.RefParam[1]+(Curr.Param.RefParam[0]-Curr.Param.RefParam[1])*exp(-pow(((ep_0-Curr.Param.RefParam[2])/Curr.Param.RefParam[3]),2.0)))/(1.0+exp(-2.0*Curr.Param.RefParam[5]*(ep_0-Curr.Param.RefParam[2])));
	}
	else if(Curr.Param.RefFunction==5){	//direct shear2 using Tanaka's model
		result=	pow(((2.0*pow((Curr.Param.RefParam[2]*ep_0),0.5))/(Curr.Param.RefParam[2]+ep_0)),Curr.Param.RefParam[4])*
					Curr.Param.RefParam[0]*(1.0/(1.0+exp(-2.0*Curr.Param.RefParam[5]*(ep_0-Curr.Param.RefParam[6])))-1.0/(1.0+exp(-2.0*Curr.Param.RefParam[5]*(ep_0-Curr.Param.RefParam[2])))) +
					(Curr.Param.RefParam[1]+(Curr.Param.RefParam[0]-Curr.Param.RefParam[1])*exp(-pow(((ep_0-Curr.Param.RefParam[2])/Curr.Param.RefParam[3]),Curr.Param.RefParam[7])))/(1.0+exp(-2.0*Curr.Param.RefParam[5]*(ep_0-Curr.Param.RefParam[2])));
	}

	return(result);

};

//
double GTE::Sigma_fy(double ep,double ep_ir){ 
	double result;
	double r;
	double epd_ir_c,tau_c,tau_c_p;
	class History *temp_history;

	//without aging
	result=Sigma_fy0(ep,ep_ir);
	//with aging
	temp_history=&Curr.Param.history;
	r=Fr_f(ep_ir);
	epd_ir_c=((ep_ir-Prev.ep_ir)*2)/CTR.dt-Prev.epd_ir;
	tau_c=Prev.tau+getDtau(Prev.epd_ir,epd_ir_c,CTR.dt);
	do{
		result=result+temp_history->DSigma_f*(Curr.Param.lambda_f+(1-Curr.Param.lambda_f)*Decay_f(tau_c,temp_history->tau,r));
		temp_history=temp_history->before;
	}while(temp_history);
		//前のステップから今のステップに至るまでに生じた非粘性応力を
		//減衰させながら加える。
	tau_c_p=getDtau(Prev.epd_ir,(epd_ir_c+Prev.epd_ir)/2,CTR.dt/2)+Prev.tau;
	result=result+DSigma_fy(ep,ep_ir)*(Curr.Param.lambda_f+(1-Curr.Param.lambda_f)*Decay_f(tau_c,tau_c_p,r));

	return(result);
}

//#########
//For Aging
//#########
//Sigma_fy without aging effect
double GTE::Sigma_fy0(double ep,double ep_ir){ 
	double result,ep_0;

	ep_0=ep_ir-Curr.Param.OriginX0;
	result=Curr.Param.OriginY0+Sigma_f00(ep_0);
	return(result);
}

double GTE::DSigma_fy(double ep,double ep_ir){ 
	double result;

	result=(Sigma_f00(ep)-Sigma_f00(Prev.ep))*((A_f(Curr.time)+A_f(Prev.time))/2-1)
		+((Curr.Param.a_f*Sigma_fy0(ep,ep_ir)+Curr.Param.b_f)*alpha_f(Curr.time)+
			(Curr.Param.a_f*Sigma_fy0(Prev.ep,Prev.ep_ir)+Curr.Param.b_f)*alpha_f(Prev.time))/2
			*(Curr.time-Prev.time);

	return(result);
}

double GTE::A_f(double time){ 
	double result;
	//double t0, t_ini;
	//double t1, t2, t3, t4;
	//Important note for the simplest case where there is no interaction
	//Write a function for A_f as a function of time
	//Write a function for alpha_f as the first derivation of A_f wrt time
	//Give a_f = 1.0 and b_f = 0.0
	
	//Temporal result=1 ->no time effect
	result=Curr.Param.A_f;	//user-set via Parameter Editor (was 0.957)
	
	//for test
/*	t0 = 50.0;
	t_ini = 253.25;
	t1=t_ini+3600.0;
	t2=t1+3600.0;
	t3=t2+3600.0;
	t4=t3+3600.0;

	
	if(time >= t1){
		result = (exp(-(time-t1)/t0)+1.0)/2.0;
		if(time >= t1+10.0){
			result = (exp(-10.0/t0)+1.0)/2.0;
			if(time >= t2){
				result = (exp(-(time-t2)/t0)+1.0)/2.0*(exp(-10.0/t0)+1.0)/2.0;
				if(time >= t2+10.0){
					result = pow((exp(-10.0/t0)+1.0)/2.0,2.0);
					if(time >= t3){
						result = (exp(-(time-t3)/t0)+1.0)/2.0*pow((exp(-10.0/t0)+1.0)/2.0,2.0);
						if(time >= t3+10.0){
							result = pow((exp(-10.0/t0)+1.0)/2.0,3.0);
							if(time >= t4){
								result = (exp(-(time-t4)/t0)+1.0)/2.0*pow((exp(-10.0/t0)+1.0)/2.0,3.0);
								if(time >= t4+10.0){
									result = pow((exp(-10.0/t0)+1.0)/2.0,4.0);
								}
							}
						}
					}
				}
			}
		}
	}*/

	/*t_ini= 0.0;
	t1=0.0;
	t2=t1+10.0;
	t3=t2+10.0;
	t4=t3+10.0;

	if(time >= t1){
		result = (exp(-(time-t1)/t0)+1.0)/2.0;
		if(time >= t2){
			result = (exp(-(time-t2)/t0)+1.0)/2.0*(exp(-10.0/t0)+1.0)/2.0;
			if(time >= t3){
				result = (exp(-(time-t3)/t0)+1.0)/2.0*pow((exp(-10.0/t0)+1.0)/2.0,2.0);
				if(time >= t4){
					result = (exp(-(time-t4)/t0)+1.0)/2.0*pow((exp(-10.0/t0)+1.0)/2.0,3.0);
					if(time >= t4+10.0){
						result = pow((exp(-10.0/t0)+1.0)/2.0,4.0);
					}
				}
			}
		}
	}

	t_ini= 0.0;
	t1=0.0;
	t2=t1+10.0;
	t3=t2+10.0;
	t4=t3+10.0;*/

	return(result);
}

double GTE::alpha_f(double time){ 
	double result;
	//double t0, t_ini;
	//double t1,t2, t3, t4;
	//Important note for the simplest case where there is no interaction
	//Write a function for A_f as a function of time
	//Write a function for alpha_f as the first derivation of A_f wrt time
	//Give a_f = 1.0 and b_f = 0.0

	//Temporal result=0 ->no time effect
	result=0.0;
	
	//for test
	/*t0 =50.0;
	t_ini = 253.25;
	t1=t_ini+3600.0;
	t2=t1+3600.0;
	t3=t2+3600.0;
	t4=t3+3600.0;


	if(time >= t1){
		result = -exp(-(time-t1)/t0)/2.0/t0;
		if(time >= t1+10.0){
			result = 0.0;
			if(time >= t2){
				result = (exp(-10.0/t0)+1.0)/2.0*-exp(-(time-t2)/t0)/2.0/t0;
				if (time >= t2+10.0){
					result = 0.0;
					if(time >= t3){
						result = pow((exp(-10.0/t0)+1.0)/2.0,2.0)*-exp(-(time-t3)/t0)/2.0/t0;
						if(time >= t3+10.0){
							result = 0.0;
							if(time >= t4){
								result = pow((exp(-10.0/t0)+1.0)/2.0,3.0)*-exp(-(time-t4)/t0)/2.0/t0;
								if(time >= t4+10.0){
									result = 0.0;
								}
							}
						}
					}
				}
			}
		}
	}*/


	/*t_ini= 0.0;
	t1=0.0;
	t2=t1+10.0;
	t3=t2+10.0;
	t4=t3+10.0;
	
	if(time >= t1){
		result = -exp(-(time-t1)/t0)/2.0/t0;
		if(time >= t2){
			result = (exp(-10.0/t0)+1.0)/2.0*-exp(-(time-t2)/t0)/2.0/t0;
			if(time >= t3){
				result = pow((exp(-10.0/t0)+1.0)/2.0,2.0)*-exp(-(time-t3)/t0)/2.0/t0;
				if(time >= t4){
					result = pow((exp(-10.0/t0)+1.0)/2.0,3.0)*-exp(-(time-t4)/t0)/2.0/t0;
					if(time >= t4+10.0){
						result = 0.0;
					}
				}
			}
		}
	}
	
	t_ini= 0.0;
	t1=0.0;
	t2=t1+10.0;
	t3=t2+10.0;
	t4=t3+10.0;*/
	
	return(result);

}

double GTE::Decay_f(double tau_c,double tau_i,double r){ 
    double result;
    
    result=pow(r,tau_c-tau_i);
    
    return(result);
}

double GTE::Fr_f(double ep_ir){
	double result;

	if(ep_ir<Curr.Param.ep_ir_0){
		result=(Curr.Param.r2_f+Curr.Param.r1_f)/2+(Curr.Param.r2_f-Curr.Param.r1_f)/2*cos(___PI*pow((ep_ir/Curr.Param.ep_ir_0_f),Curr.Param.c_f));
	}else{
		result=Curr.Param.r1_f;
	}

	return(result);
}

//##############
//Viscous Stress
//##############

/*double GTE::Sigma_V(double Dtau,double ep_c,double ep_ir_c,double epd_ir_c){
	double result=0;
	double tau_c,r,tau_c_p;
	class History *temp_V_history;

	if(Curr.Param.Integration==1){//by Integration
		tau_c=Prev.tau+Dtau;
		r=Fr(tau_c);
		temp_V_history=&Curr.Param.history;
		do{
			result=result+temp_V_history->DSigma_V*(Curr.Param.lambda_V+(1-Curr.Param.lambda_V)*Fgdecay(tau_c,temp_V_history->tau,r));
			temp_V_history=temp_V_history->before;
		}while(temp_V_history);
		tau_c_p=getDtau(Prev.epd_ir,(epd_ir_c+Prev.epd_ir)/2,CTR.dt/2)+Prev.tau;
		result=result+DSigma_V(ep_c,ep_ir_c,epd_ir_c)*(Curr.Param.lambda_V+(1-Curr.Param.lambda_V)*Fgdecay(tau_c,tau_c_p,r));
	}else{//by approximation
		result=Curr.Param.lambda_V*Sigma_V_ISOTACH(ep_c,ep_ir_c,epd_ir_c)
			+(1-Curr.Param.lambda_V)*Sigma_V_TESRA(Dtau,ep_c,ep_ir_c,epd_ir_c);
	}

	return(result);
}*/


double GTE::Sigma_V(double Dtau,double ep_c,double ep_ir_c,double epd_ir_c){
	double result=0;
	double tau_c,r,tau_c_p;
	class History *temp_V_history;
	double th;

	if(Curr.Param.Integration==1){//by Integration
		tau_c=Prev.tau+Dtau;
		r=Fr(tau_c);
		th=Ftheta(tau_c);
		temp_V_history=&Curr.Param.history;
		do{
			result=result+temp_V_history->DSigma_V*(th+(1-th)*Fgdecay(tau_c,temp_V_history->tau,r));
			temp_V_history=temp_V_history->before;
		}while(temp_V_history);
		tau_c_p=getDtau(Prev.epd_ir,(epd_ir_c+Prev.epd_ir)/2,CTR.dt/2)+Prev.tau;
		result=result+DSigma_V(ep_c,ep_ir_c,epd_ir_c)*(th+(1-th)*Fgdecay(tau_c,tau_c_p,r));
	}else{//by approximation
		tau_c=Prev.tau+Dtau;
		th=Ftheta(tau_c);
		result=th*Sigma_V_ISOTACH(ep_c,ep_ir_c,epd_ir_c)
			+(1-th)*Sigma_V_TESRA(Dtau,ep_c,ep_ir_c,epd_ir_c);
	}

	return(result);
}



/*double GTE::Sigma_V_TESRA(double Dtau,double ep_c,double ep_ir_c,double epd_ir_c){
    double result=0;
    double tau_c,r,tau_c_p;
	
	if(Curr.Param.lambda_V<1){
	    tau_c=Prev.tau+Dtau;
		r=Fr(tau_c);//(Fr(Prev.tau)+Fr(tau_c))/2;
	
//		result=(Prev.Sigma-Prev.Sigma_f-Curr.Param.lambda_V*Sigma_V_ISOTACH(Prev.ep,Prev.ep_ir,Prev.epd_ir))*Fgdecay(tau_c,Prev.tau,r);
		result=(Prev.Sigma_V-Curr.Param.lambda_V*Sigma_V_ISOTACH(Prev.ep,Prev.ep_ir,Prev.epd_ir))*Fgdecay(tau_c,Prev.tau,r);
//		result=(Prev.Sigma-Curr.Param.lambda_V*Sigma_V_ISOTACH(Prev.ep,Prev.ep_ir,Prev.epd_ir)-Prev.Sigma_f)*Fgdecay(tau_c,Prev.tau,r);
		//1-lambda_Vをかける前の状態にする
		result=result/(1-Curr.Param.lambda_V);
		//前のステップから今のステップに至るまでの平均的なtau
		tau_c_p=getDtau(Prev.epd_ir,(epd_ir_c+Prev.epd_ir)/2,CTR.dt/2)+Prev.tau;
		//前のステップから今のステップに至るまでに生じた粘性応力を
		//減衰させながら加える。
		result=result+DSigma_V(ep_c,ep_ir_c,epd_ir_c)*Fgdecay(tau_c,tau_c_p,r);
	}else{//lambda_V>=1
		result=0;
	}

	return(result);
}*/

double GTE::Sigma_V_TESRA(double Dtau,double ep_c,double ep_ir_c,double epd_ir_c){
    double result=0;
    double tau_c,r,tau_c_p;
	double th;
	
	tau_c=Prev.tau+Dtau;
	th=Ftheta(tau_c);

	if(th<1.0){//Curr.Param.lambda_V<1
	    tau_c=Prev.tau+Dtau;
		r=Fr(tau_c);
		//result=(Prev.Sigma_V-th*Prev.Sigma_V_Iso)*Fgdecay(tau_c,Prev.tau,r); // From Kosit mix
		result=(Prev.Sigma_V-th*Sigma_V_ISOTACH(Prev.ep,Prev.ep_ir,Prev.epd_ir))*Fgdecay(tau_c,Prev.tau,r);// From Stepped Isothermal Real
		result=result/(1-th);
		tau_c_p=getDtau(Prev.epd_ir,(epd_ir_c+Prev.epd_ir)/2,CTR.dt/2)+Prev.tau;
		result=result+DSigma_V(ep_c,ep_ir_c,epd_ir_c)*Fgdecay(tau_c,tau_c_p,r);
	}else{//lambda_V>=1
		result=0;
	}

	return(result);
}



double GTE::Sigma_V_ISOTACH(double ep_c,double ep_ir_c,double epd_ir_c){
    double result=0;

	result=Sigma_f0(ep_c,ep_ir_c)*Fgv(epd_ir_c);

	return(result);
}

/*
double GTE::Sigma_V_ISOTACH(double ep_c,double ep_ir_c,double epd_ir_c){ // From Kosit mix
	double n;
	double Sigma_f0_c,Sigma_f0_0;
	double result=0;

	Sigma_f0_c=Sigma_f0(ep_c,ep_ir_c);

	if(Curr.Param.flg_freeze){
		result=0;
	}else{
		if(Curr.Param.flg_unload){
			//Unloading
			if(Curr.Param.DirectionX<0){
			//Proportion by Py to increase viscous component
				n=Curr.Param.ProportionY;
				Sigma_f0_c=n*Sigma_f0_c;
			}else{
			//Reloading
				Sigma_f0_0=Sigma_f0(Curr.Param.OriginX,Curr.Param.OriginX);
				n=Curr.Param.ProportionX*(Curr.Param.Sigma_Un-Curr.Param.IniSigma_f)
					/(Curr.Param.Sigma_Un-Curr.Param.Sigma_Re); // Scale to return to where unload started
				Sigma_f0_c=n*(Sigma_f0_c-Sigma_f0_0)+Sigma_f0_0;
			}
		}
	}

	result=Sigma_f0_c*Fgv(epd_ir_c);

	//前???X?e?b?v??????�?????粘�???�???総?a
	//??�?????
//	result=Prev.Sigma_V_Iso;
	//前???X?e?b?v????�????X?e?b?v????????????�?????粘�???�???
	//??�???????????????。
//	result=result+DSigma_V(ep_c,ep_ir_c,epd_ir_c);

	return(result);
}*/


double GTE::Fr(double ep_ir){
	double result;

	if(ep_ir<Curr.Param.ep_ir_0){
		result=(Curr.Param.r2+Curr.Param.r1)/2+(Curr.Param.r2-Curr.Param.r1)/2*cos(___PI*pow((ep_ir/Curr.Param.ep_ir_0),Curr.Param.c));
	}else{
		result=Curr.Param.r1;
	}

	return(result);
}

/*
double GTE::Fr(double ep_ir){ // From Kosit mix
	double result;

	if(ep_ir<Curr.Param.ep_ir_0){
		result=(Curr.Param.r2+Curr.Param.r1)/2+(Curr.Param.r2-Curr.Param.r1)/2*cos(___PI*pow((ep_ir/Curr.Param.ep_ir_0),Curr.Param.c));
	}else{
		result=Curr.Param.r1;
	}

	return(result);
}*/


double GTE::Ftheta(double ep_ir){ //Ftheta is a new function
	double result;

	if(ep_ir<Curr.Param.rf_strain){
		result=(Curr.Param.th2+Curr.Param.th1)/2.0+(Curr.Param.th2-Curr.Param.th1)/2.0*cos(___PI*pow((ep_ir/Curr.Param.rf_strain),Curr.Param.curve));
	}else{
		result=Curr.Param.th1;
	}

	return(result);
}


double GTE::DSigma_V(double ep_c,double ep_ir_c,double epd_ir_c){
	double result;

	if(Curr.Param.flg_freeze){
		result=0;
	}else{
		result=(Sigma_f0(ep_c,ep_ir_c)+Sigma_f0(Prev.ep,Prev.ep_ir))*(Fgv(epd_ir_c)-Fgv(Prev.epd_ir))/2
			+(Sigma_f0(ep_c,ep_ir_c)-Sigma_f0(Prev.ep,Prev.ep_ir))*(Fgv(epd_ir_c)+Fgv(Prev.epd_ir))/2;
	}

    return(result);
}


/*double GTE::DSigma_V(double ep_c,double ep_ir_c,double epd_ir_c){
	double Sigma_f0_c,Sigma_f0_p,Sigma_f0_0;
	double n=1;
	double result;

	Sigma_f0_c=Sigma_f0(ep_c,ep_ir_c);
	Sigma_f0_p=Sigma_f0(Prev.ep,Prev.ep_ir);

	if(Curr.Param.flg_freeze){
		result=0;
	}else{
		if(Curr.Param.flg_unload){
			//�???�?�???�?
			if(Curr.Param.DirectionX<0){
			//�???�?
				n=Curr.Param.ProportionY;
				Sigma_f0_c=n*Sigma_f0_c;
				Sigma_f0_p=n*Sigma_f0_p;
			}else{
			//�?�???�?1??23�?�?�?修�?
				Sigma_f0_0=Sigma_f0(Curr.Param.OriginX,Curr.Param.OriginX);
				n=Curr.Param.ProportionX*(Curr.Param.Sigma_Un-Curr.Param.IniSigma_f)
					/(Curr.Param.Sigma_Un-Curr.Param.Sigma_Re);
				Sigma_f0_c=n*(Sigma_f0_c-Sigma_f0_0)+Sigma_f0_0;
				Sigma_f0_p=n*(Sigma_f0_p-Sigma_f0_0)+Sigma_f0_0;
			}
		}

		result=(Sigma_f0_c+Sigma_f0_p)*(Fgv(epd_ir_c)-Fgv(Prev.epd_ir))/2
			+(Sigma_f0_c-Sigma_f0_p)*(Fgv(epd_ir_c)+Fgv(Prev.epd_ir))/2;

	}

    return(result);
}*/

/*
double GTE::Fgv(double epd_ir){
	double result;
		//(double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_0+1>0)
	//if((double)Curr.Param.DirectionX*epd_ir>=0){
	//	result=(double)Curr.Param.DirectionY*(Curr.Param.alpha*1-Curr.Param.alpha*exp(1-pow(((double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_0+1),Curr.Param.m)));
	//}else{
	//	result=-(double)Curr.Param.DirectionY*(Curr.Param.alpha*1-Curr.Param.alpha*exp(1-pow((-(double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_0+1),Curr.Param.m)));
	//}

	if((double)Curr.Param.DirectionX*epd_ir>=0){
		result=(double)Curr.Param.DirectionY*Curr.Param.alpha*pow((double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_0,Curr.Param.m);
	}else{
		result=-(double)Curr.Param.DirectionY*Curr.Param.alpha*pow(-(double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_0,Curr.Param.m);
	}
	

		return(result);
}; */


double GTE::Fgv(double epd_ir){
	double result;
	double gv1,gv2;
	
	if(Curr.Param.gvtype==0){//use gv1 only
		if((double)Curr.Param.DirectionX*epd_ir>=0){
			gv1=(double)Curr.Param.DirectionY*(Curr.Param.alpha*1-Curr.Param.alpha*exp(1-pow(((double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_0+1),Curr.Param.m)));
			gv2=(double)Curr.Param.DirectionY*Curr.Param.alpha2*pow((double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_2,Curr.Param.m2);
		}else{
			gv1=-(double)Curr.Param.DirectionY*(Curr.Param.alpha*1-Curr.Param.alpha*exp(1-pow((-(double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_0+1),Curr.Param.m)));
			gv2=-(double)Curr.Param.DirectionY*Curr.Param.alpha2*pow(-(double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_2,Curr.Param.m2);
		}
		result=gv1;
	}else if(Curr.Param.gvtype==1){//use gv2 only
		if((double)Curr.Param.DirectionX*epd_ir>=0){
			gv1=(double)Curr.Param.DirectionY*(Curr.Param.alpha*1-Curr.Param.alpha*exp(1-pow(((double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_0+1),Curr.Param.m)));
			gv2=(double)Curr.Param.DirectionY*Curr.Param.alpha2*pow((double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_2,Curr.Param.m2);
		}else{
			gv1=-(double)Curr.Param.DirectionY*(Curr.Param.alpha*1-Curr.Param.alpha*exp(1-pow((-(double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_0+1),Curr.Param.m)));
			gv2=-(double)Curr.Param.DirectionY*Curr.Param.alpha2*pow(-(double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_2,Curr.Param.m2);
		}
		result=gv2;
	}else if(Curr.Param.gvtype==2){//mix gv1 and gv2 by 1 log cycle
		if((double)Curr.Param.DirectionX*epd_ir>=0.0){
			gv1=(double)Curr.Param.DirectionY*(Curr.Param.alpha*1-Curr.Param.alpha*exp(1-pow(((double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_0+1),Curr.Param.m)));
			gv2=(double)Curr.Param.DirectionY*Curr.Param.alpha2*pow((double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_2,Curr.Param.m2);
			if((double)Curr.Param.DirectionX*epd_ir>=pow(10.0,0.5)*Curr.Param.epd_ir_int){
				result=gv1;
			}else if((double)Curr.Param.DirectionX*epd_ir<pow(10.0,0.5)*Curr.Param.epd_ir_int && (double)Curr.Param.DirectionX*epd_ir>=1.0/pow(10.0,0.5)*Curr.Param.epd_ir_int){
				result=(log10((double)Curr.Param.DirectionX*epd_ir)-log10(1.0/pow(10.0,0.5)*Curr.Param.epd_ir_int))/log10(10.0)*gv1+(1-(log10((double)Curr.Param.DirectionX*epd_ir)-log10(1.0/pow(10.0,0.5)*Curr.Param.epd_ir_int))/log10(10.0))*gv2;
			}else{
				result=gv2;
			}
		}else{
			gv1=-(double)Curr.Param.DirectionY*(Curr.Param.alpha*1-Curr.Param.alpha*exp(1-pow((-(double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_0+1),Curr.Param.m)));
			gv2=-(double)Curr.Param.DirectionY*Curr.Param.alpha2*pow(-(double)Curr.Param.DirectionX*epd_ir/Curr.Param.epd_ir_2,Curr.Param.m2);
			if(-(double)Curr.Param.DirectionX*epd_ir>=pow(10.0,0.5)*Curr.Param.epd_ir_int){
				result=gv1;
			}else if(-(double)Curr.Param.DirectionX*epd_ir<pow(10.0,0.5)*Curr.Param.epd_ir_int && -(double)Curr.Param.DirectionX*epd_ir>=1.0/pow(10.0,0.5)*Curr.Param.epd_ir_int){
				result=(log10(-(double)Curr.Param.DirectionX*epd_ir)-log10(1.0/pow(10.0,0.5)*Curr.Param.epd_ir_int))/log10(10.0)*gv1+(1-(log10(-(double)Curr.Param.DirectionX*epd_ir)-log10(1.0/pow(10.0,0.5)*Curr.Param.epd_ir_int))/log10(10.0))*gv2;
			}else{
				result=gv2;
			}
		}
	}

	return(result);
};





/*
double GTE::Fgdecay(double tau_c,double tau_i,double r){
    double result;
    
    result=pow(r,tau_c-tau_i);
    
    return(result);
}*/

double GTE::Fgdecay(double tau_c,double tau_i,double r){
    double result;
    
    result=pow(r,tau_c-tau_i);
    
    return(result);
}


//#######
//ELASTIC
//#######

//Elastic Strain(Stress)
double GTE::Sigma_to_ep_e(double Sigma){
	double result;

	if(Sigma<0){
		Sigma=0;
	}
	result=pow(Sigma,1-Curr.Param.b)/(1-Curr.Param.b)/Curr.Param.E_ini*pow(Curr.Param.Sigma_0,Curr.Param.b);

	return(result);
};

/*
//Elastic Strain(Stress)
double GTE::Sigma_to_ep_e(double Sigma){
	double result;

	if(Sigma>0){
		result=pow(Sigma,1-Curr.Param.b)/(1-Curr.Param.b)/Curr.Param.E_ini*pow(Curr.Param.Sigma_0,Curr.Param.b);
	}else{
		result=-pow(fabs(Sigma),1-Curr.Param.b)/(1-Curr.Param.b)/Curr.Param.E_ini*pow(Curr.Param.Sigma_0,Curr.Param.b);
	}
	
	return(result);
};*/


//Stress(Elastic Strain)
double GTE::ep_e_to_Sigma(double ep_e){
	double result;

	result=pow((1-Curr.Param.b)*Curr.Param.E_ini*pow(Curr.Param.Sigma_0,-Curr.Param.b)*ep_e,1/(1-Curr.Param.b));
	if(result<0){
		result=0;
	}	

	return(result);
};

/*
//Stress(Elastic Strain)
double GTE::ep_e_to_Sigma(double ep_e){
	double result;

	if(ep_e>0){
		result=pow((1-Curr.Param.b)*Curr.Param.E_ini*pow(Curr.Param.Sigma_0,-Curr.Param.b)*ep_e,1/(1-Curr.Param.b));
	}else{
		result=-pow((1-Curr.Param.b)*Curr.Param.E_ini*pow(Curr.Param.Sigma_0,-Curr.Param.b)*fabs(ep_e),1/(1-Curr.Param.b));
	}

	return(result);
};*/


//#############################
//Irreversible Strain Increment
//#############################
/*
double GTE::getDtau(double epd_prev, double epd_curr, double dt) {
	double result;
	
	if (fabs(epd_prev) < 1.e-8 && fabs(epd_curr)<1.e-8 ) {
		result = 0;
	}else if(epd_prev*epd_curr>0) {
		result = fabs(epd_prev+epd_curr)*dt/2;
	}else{
		result = (epd_prev*epd_prev+epd_curr*epd_curr)/(fabs(epd_prev)+fabs(epd_curr))*dt/2;
    }

	return(result);
}*/

double GTE::getDtau(double epd_prev, double epd_curr, double dt) {
	double result;
	
	if(epd_prev*epd_curr>=0) {
		result = fabs(epd_prev+epd_curr)*dt/2;
	}else if (fabs(epd_prev) < 1.e-15 && fabs(epd_curr)<1.e-15 ) {
		result = 0;
	}else{
		result = (epd_prev*epd_prev+epd_curr*epd_curr)/(fabs(epd_prev)+fabs(epd_curr))*dt/2;
    }

	return(result);
}
	
#endif