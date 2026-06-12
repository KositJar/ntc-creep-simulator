//最終更新2000年12月7日
#if !defined ___StressControl
#define ___StressControl

#include "C3comp.h"

//Class to solve equation in stress rate constrol
class StressControl:public C3comp{
public:

protected:

private:
	void Calc();//override
	double Equation(double ep_ir);//override

};

void StressControl::Calc(){
	if(CTR.TypeofEvent==StressRateConstant){
		Curr.Sigma=Prev.Sigma+CTR.CTRValue*CTR.dt;
	}else{//if(CTR.TypeofEvent==Creep)
		Curr.Sigma=Prev.Sigma;
	}
            
	Curr.ep_ir=Prev.ep_ir+Prev.epd_ir*CTR.dt;//temporal

	if(!SolveEq(Curr.ep_ir,Prev.ep_ir)){
		(*LOGFile) << Curr.time << "\tCannot solve Equation\n";
		Curr.result=false;
	};
	Curr.ep=Curr.ep_ir+Sigma_to_ep_e(Curr.Sigma);
	Curr.epd=2*(Curr.ep-Prev.ep)/CTR.dt-Prev.epd;

	Curr.epd_ir=2*(Curr.ep_ir-Prev.ep_ir)/CTR.dt-Prev.epd_ir;
	Curr.Sigma_f=Sigma_f(Curr.ep,Curr.ep_ir);
	Curr.Sigma_f0=Sigma_f0(Curr.ep,Curr.ep_ir);
	Curr.Sigma_V=Sigma_V(getDtau(Prev.epd_ir,Curr.epd_ir,CTR.dt),Curr.ep,Curr.ep_ir,Curr.epd_ir);
	Curr.tau=Prev.tau+getDtau(Prev.epd_ir,Curr.epd_ir,CTR.dt);

}

double StressControl::Equation(double ep_ir){
    double ep;
    double result;

    ep=ep_ir+Sigma_to_ep_e(Curr.Sigma);
    result=Curr.Sigma-Sigma_t(ep,ep_ir);    

    return(result);
}

#endif