//最終更新2000年12月7日
#if !defined ___StrainControl
#define ___StrainControl

#include "C3comp.h"

//Class to solve equation in strain rate control condition
class StrainControl:public C3comp{
public:

protected:

private:
	void Calc();//override
	double Equation(double ep_ir);//override

};

void StrainControl::Calc(){
	if(CTR.TypeofEvent==StrainRateConstant){
		Curr.epd=CTR.CTRValue;
	}else if(CTR.TypeofEvent==Relaxation){
		Curr.epd=0;
	}else{//if(CTR.TypeofEvent==AccControl)
		Curr.epd=(CTR.CTRValue-Prev.epd)/(CTR.EndTime-Prev.time)*CTR.dt+Prev.epd;
	}
	Curr.ep=Prev.ep+(Prev.epd+Curr.epd)/2.*CTR.dt;

	Curr.ep_ir=Prev.ep_ir+Prev.epd_ir*CTR.dt;//temporal

	if(!SolveEq(Curr.ep_ir,Prev.ep_ir)){
			(*LOGFile) << Curr.time << "Cannot solve Equation\n";
			Curr.result=false;
	}
	Curr.Sigma=ep_e_to_Sigma(Curr.ep-Curr.ep_ir);

	Curr.epd_ir=2*(Curr.ep_ir-Prev.ep_ir)/CTR.dt-Prev.epd_ir;
	Curr.Sigma_f=Sigma_f(Curr.ep,Curr.ep_ir);
	Curr.Sigma_f0=Sigma_f0(Curr.ep,Curr.ep_ir);
	Curr.Sigma_V=Sigma_V(getDtau(Prev.epd_ir,Curr.epd_ir,CTR.dt),Curr.ep,Curr.ep_ir,Curr.epd_ir);
	Curr.tau=Prev.tau+getDtau(Prev.epd_ir,Curr.epd_ir,CTR.dt);

}

double StrainControl::Equation(double ep_ir){
	double ep;
	double result;

	ep=Prev.ep+(Prev.epd+Curr.epd)/2*CTR.dt;
	result=ep-ep_ir-Sigma_to_ep_e(Sigma_t(ep,ep_ir));  

    return(result);
}

#endif

