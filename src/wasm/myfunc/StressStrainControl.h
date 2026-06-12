//最終更新2000年12月7日
#if !defined ___StressStrainControl
#define ___StressStrainControl

#include "C3comp.h"

//Class to solve equation in stress strain rerationship constrol
class StressStrainControl:public C3comp{
public:
	
protected:

private:
	void Calc();//override
	double Equation(double ep_ir);//override

};

void StressStrainControl::Calc(){

	Curr.ep=Prev.ep+Prev.epd*CTR.dt;//temporal
	
	if(!SolveEq(Curr.ep,Prev.ep)){
		(*LOGFile) << Curr.time << "Cannot solve Equation\n";
		Curr.result=false;
	};
	Curr.Sigma=CTR.CTRValue*(Curr.ep-Prev.ep)+Prev.Sigma;
	Curr.epd=2*(Curr.ep-Prev.ep)/CTR.dt-Prev.epd;
	Curr.ep_ir=Curr.ep-Sigma_to_ep_e(Curr.Sigma);

	Curr.epd_ir=2*(Curr.ep_ir-Prev.ep_ir)/CTR.dt-Prev.epd_ir;
	Curr.Sigma_f=Sigma_f(Curr.ep,Curr.ep_ir);
	Curr.Sigma_f0=Sigma_f0(Curr.ep,Curr.ep_ir);
	Curr.Sigma_V=Sigma_V(getDtau(Prev.epd_ir,Curr.epd_ir,CTR.dt),Curr.ep,Curr.ep_ir,Curr.epd_ir);
	Curr.tau=Prev.tau+getDtau(Prev.epd_ir,Curr.epd_ir,CTR.dt);

}

double StressStrainControl::Equation(double ep){
    double Sigma;
	double ep_ir;
    double result;

    Sigma=CTR.CTRValue*(ep-Prev.ep)+Prev.Sigma;
    ep_ir=ep-Sigma_to_ep_e(Sigma);
    result=Curr.Sigma-Sigma_t(ep,ep_ir);

    return(result);
}

#endif