//最終更新2000年12月7日
#if !defined ___FreezingStressControl
#define ___FreezingStressControl

#include "C3comp.h"

//Class to solve equation in stress rate constrol
class FreezingStressControl:public C3comp{
public:

protected:

private:
	void Calc();//override
	double Equation(double ep_ir);//override

};

void FreezingStressControl::Calc(){
	if(CTR.TypeofEvent==StressRateConstant){
		Curr.Sigma=Prev.Sigma+CTR.CTRValue*CTR.dt;
	}else{//if(CTR.TypeofEvent==Creep)
		Curr.Sigma=Prev.Sigma;
	}
            
	Curr.ep_ir=Prev.ep_ir;
	Curr.ep=Curr.ep_ir+Sigma_to_ep_e(Curr.Sigma);
	Curr.epd=2*(Curr.ep-Prev.ep)/CTR.dt-Prev.epd;

	Curr.epd_ir=0;
	Curr.Sigma_f=Prev.Sigma_f+Curr.Sigma-Prev.Sigma;
	Curr.Sigma_f0=Sigma_f0(Curr.ep,Curr.ep_ir);
	Curr.Sigma_V=0;
	Curr.tau=Prev.tau+getDtau(Prev.epd_ir,Curr.epd_ir,CTR.dt);

}

//not use
double FreezingStressControl::Equation(double ep_ir){
    double result;

    result=0;    

    return(result);
}

#endif