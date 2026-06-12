//最終更新2000年12月7日
#if !defined ___FreezingStrainControl
#define ___FreezingStrainControl

#include "C3comp.h"

//Class to solve equation in strain rate control condition
class FreezingStrainControl:public C3comp{
public:

protected:

private:
	void Calc();//override
	double Equation(double ep_ir);//override

};

void FreezingStrainControl::Calc(){
	if(CTR.TypeofEvent==StrainRateConstant){
		Curr.epd=CTR.CTRValue;
	}else if(CTR.TypeofEvent==Relaxation){
		Curr.epd=0;
	}else{//if(CTR.TypeofEvent==AccControl)
		Curr.epd=(CTR.CTRValue-Prev.epd)/(CTR.EndTime-Prev.time)*CTR.dt+Prev.epd;
	}
	Curr.ep=Prev.ep+(Prev.epd+Curr.epd)/2.*CTR.dt;

	Curr.ep_ir=Prev.ep_ir;//temporal

	Curr.Sigma=ep_e_to_Sigma(Curr.ep-Curr.ep_ir);

	Curr.epd_ir=0;
	Curr.Sigma_f=Prev.Sigma_f+Curr.Sigma-Prev.Sigma;
	Curr.Sigma_f0=Sigma_f0(Curr.ep,Curr.ep_ir);
	Curr.Sigma_V=0;
	Curr.tau=Prev.tau+getDtau(Prev.epd_ir,Curr.epd_ir,CTR.dt);

}

//not use
double FreezingStrainControl::Equation(double ep_ir){

	double result;

	result=0;  

    return(result);
}

#endif

