//最終更新2000年12月7日
#if !defined ___SetParam
#define ___SetParam

#include "GTE.h"

class SetParam:protected GTE{
public:
	class Value setParam(ifstream& PRMFile);
	class Value ChkDir(class Value CurrVal,class Value PrevVal,class Control CurrCTR);
	void RecParam(ofstream& LOGFile);
	
protected:
	class Value ChgDir(class Value CurrVal,class Value PrevVal);

private:

};

class Value SetParam::ChkDir(class Value CurrVal,class Value PrevVal,class Control CurrCTR){
	
	Curr=CurrVal;
	Prev=PrevVal;

//	if(Curr.epd_ir*Prev.epd_ir>0){
//		if(Curr.epd_ir*CurrCTR.CTRValue>0 && Curr.epd_ir*Curr.Param.DirectionX <0){
//			class SetParam setParam;
//			Curr=setParam.ChgDir(Curr,Prev);
//		}
//	}
	if(Curr.Sigma_f>Curr.Sigma_fy && Curr.Param.DirectionX >0){
		Curr.Param.OriginX=Curr.Param.OriginX0;
		Curr.Param.OriginY=Curr.Param.OriginY0;
		Curr.Param.flg_unload=0;
		Curr.Param.flg_freeze=0;
	}
	//temporal
	if(Curr.epd_ir*Prev.epd_ir>0 && Curr.Param.DirectionX >0){
		if(Curr.epd_ir<0){
			Curr.Param.flg_freeze=1;
		}
	}
	return(Curr);
}

void SetParam::RecParam(ofstream& LOGFile){

	LOGFile << "Initial Data\n"
		<< "time\t" << Curr.Param.Initime << "\n"
		<< "ep\t" << Curr.Param.Iniep << "\n"
		<< "epd\t" << Curr.Param.Iniepd << "\n"
		<< "ep_ir\t" << Curr.Param.Iniep_ir << "\n"
		<< "epd_ir\t" << Curr.Param.Iniepd_ir << "\n"
		<< "Sigma\t" << Curr.Param.IniSigma << "\n"
		<< "Sigma_f\t" << Curr.Param.IniSigma_f << "\n"
		<< "tau\t" << Curr.Param.Initau << "\n";

	LOGFile << "Elastic Component\n"
		<< "b\t" << Curr.Param.b << "\n"
		<< "E_ini\t" << Curr.Param.E_ini << "\n"
		<< "Sigma_0\t" << Curr.Param.Sigma_0 << "\n";

	LOGFile << "Viscous Component\n";
		
	LOGFile << "Decay paramters\n"
		<< "r1\t" << Curr.Param.r1 << "\n"
		<< "r2\t" << Curr.Param.r2 << "\n"
		<< "ep_ir_0\t" << Curr.Param.ep_ir_0 << "\n"
		<< "c\t" << Curr.Param.c << "\n";

	LOGFile << "Viscosity function\n"
		<< "alpha\t" << Curr.Param.alpha << "\n"
		<< "m\t" << Curr.Param.m << "\n"
		<< "epd_ir_0\t" << Curr.Param.epd_ir_0 << "\n"
		<< "alpha2\t" << Curr.Param.alpha2 << "\n"
		<< "m2\t" << Curr.Param.m2 << "\n"
		<< "epd_ir_2\t" << Curr.Param.epd_ir_2 << "\n"
		<< "epd_ir_int\t" << Curr.Param.epd_ir_int << "\n"
		<< "gvtype\t" << Curr.Param.gvtype << "\n";

	LOGFile << "Initial direction\n"
		<< "Direction X\t" << Curr.Param.DirectionX << "\n"
		<< "Direction Y\t" << Curr.Param.DirectionY << "\n";

//	LOGFile << "Initial viscous values\n"
//		<< "Ini last DSigma_V\t" << Curr.Param.Ini_last_DSigma_V << "\n"
//		<< "Ini last tau\t" << Curr.Param.Ini_last_tau << "\n";

	LOGFile << "Iteration\n" 
		<< "MaxIteration\t" << Curr.Param.MaxIteration << "\n"
		<< "Precision\t" << Curr.Param.Precision << "\n"
		<< "Integration\t" << Curr.Param.Integration << "\n";

	LOGFile << "Lambda_V\t" <<Curr.Param.lambda_V << "\n";

	LOGFile << "Proportion\n"
		<< "Proportion X\t" << Curr.Param.ProportionX << "\n"
		<< "Proportion Y\t" << Curr.Param.ProportionY << "\n";

	LOGFile << "Reference curve function parameters\n" << "RefParam\t" 
		<< Curr.Param.RefParam[0] << "\t"
		<< Curr.Param.RefParam[1] << "\t"
		<< Curr.Param.RefParam[2] << "\t"
		<< Curr.Param.RefParam[3] << "\t"
		<< Curr.Param.RefParam[4] << "\t"
		<< Curr.Param.RefParam[5] << "\t"
		<< Curr.Param.RefParam[6] << "\t"
		<< Curr.Param.RefParam[7] << "\t"
		<< Curr.Param.RefParam[8] << "\t"
		<< Curr.Param.RefParam[9] << "\n";

	LOGFile << "Theta\n"
		<< "theta1\t" << Curr.Param.th1 << "\n"
		<< "theta2\t" << Curr.Param.th2 << "\n"
		<< "rf_strain\t" << Curr.Param.rf_strain << "\n"
		<< "curve" << Curr.Param.curve << "\n";

	LOGFile << "Ageing\n" 
		<< "a_f\t" << Curr.Param.a_f << "\n"
		<< "b_f\t" << Curr.Param.b_f << "\n"
		<< "lambda_f\t"	<< Curr.Param.lambda_f << "\n"
		<< "r1_f\t" << Curr.Param.r1_f << "\n"
		<< "r2_f\t" << Curr.Param.r2_f << "\n"
		<< "ep_ir_0_f\t" << Curr.Param.ep_ir_0_f << "\n"
		<< "c_f\t" << Curr.Param.c_f << "\n";
	LOGFile << "A_f\t" << Curr.Param.A_f << "\n";

}

class Value SetParam::setParam(ifstream& PRMFile){
		
	//Initial Value
	PRMFile >> Curr.Param.Initime;
	PRMFile >> Curr.Param.Iniep;
	PRMFile >> Curr.Param.Iniepd;
	PRMFile >> Curr.Param.Iniep_ir;
	PRMFile >> Curr.Param.Iniepd_ir;
	PRMFile >> Curr.Param.IniSigma;
	PRMFile >> Curr.Param.IniSigma_f;
	PRMFile >> Curr.Param.Initau;

	Curr.time=Curr.Param.Initime ;
	Curr.ep=Curr.Param.Iniep;
	Curr.epd=Curr.Param.Iniepd;
	Curr.ep_ir=Curr.Param.Iniep_ir;
	Curr.epd_ir=Curr.Param.Iniepd_ir;
	Curr.Sigma=Curr.Param.IniSigma;
	Curr.Sigma_f=Curr.Param.IniSigma_f;
	Curr.Sigma_fy=Curr.Param.IniSigma_f;
	Curr.Sigma_V=Curr.Sigma-Curr.Sigma_f;
	Curr.tau=Curr.Param.Initau;

	//Elastic
	PRMFile >> Curr.Param.b;
	PRMFile >> Curr.Param.E_ini;
	PRMFile >> Curr.Param.Sigma_0;

	// decay Curr.Parameter
	PRMFile >> Curr.Param.r1;
    PRMFile >> Curr.Param.r2;
	PRMFile >> Curr.Param.ep_ir_0;
	PRMFile >> Curr.Param.c;

	//gv
	PRMFile >> Curr.Param.alpha;
    PRMFile >> Curr.Param.m;
	PRMFile >> Curr.Param.epd_ir_0;
	PRMFile >> Curr.Param.alpha2;
	PRMFile >> Curr.Param.m2;
	PRMFile >> Curr.Param.epd_ir_2;
	PRMFile >> Curr.Param.epd_ir_int;
	PRMFile >> Curr.Param.gvtype;

	//Initial direction
	PRMFile >> Curr.Param.DirectionX;
	PRMFile >> Curr.Param.DirectionY;

	//Initial History
	double IniDSigma_V,IniDSigma_f,Initau;
	PRMFile >> IniDSigma_V;
//temporal
	IniDSigma_f=0;
	PRMFile >> Initau;
	Curr.Param.history.IniHistory(IniDSigma_V,IniDSigma_f,Initau);

	//Calculation
	PRMFile >> Curr.Param.MaxIteration;
	PRMFile >> Curr.Param.Precision;
	Curr.Param.Precision=pow(10.,-Curr.Param.Precision);
	PRMFile >> Curr.Param.Integration;
	
	//Lambda
	PRMFile >> Curr.Param.lambda_V;

	PRMFile >> Curr.Param.ProportionX;
	PRMFile >> Curr.Param.ProportionY;

	//Reference curve function parameters
	PRMFile >> Curr.Param.RefParam[0];
	PRMFile >> Curr.Param.RefParam[1];
	PRMFile >> Curr.Param.RefParam[2];
	PRMFile >> Curr.Param.RefParam[3];
	PRMFile >> Curr.Param.RefParam[4];
	PRMFile >> Curr.Param.RefParam[5];
	PRMFile >> Curr.Param.RefParam[6];
	PRMFile >> Curr.Param.RefParam[7];
	PRMFile >> Curr.Param.RefParam[8];
	PRMFile >> Curr.Param.RefParam[9];

	//Reference curve function type
	PRMFile >> Curr.Param.RefFunction;

	//Theta
	PRMFile >> Curr.Param.th1;
	PRMFile >> Curr.Param.th2;
	PRMFile >> Curr.Param.rf_strain;
	PRMFile >> Curr.Param.curve;

	//Ageing
	PRMFile >> 	Curr.Param.a_f;
	PRMFile >> 	Curr.Param.b_f;
	PRMFile >> 	Curr.Param.lambda_f;
	PRMFile >> 	Curr.Param.r1_f;
	PRMFile >> 	Curr.Param.r2_f;
	PRMFile >> 	Curr.Param.ep_ir_0_f;
	PRMFile >> 	Curr.Param.c_f;
	PRMFile >> Curr.Param.A_f;
	if(Curr.Param.A_f==0) Curr.Param.A_f=1.0;	//default A_f (1.0 = no time effect); also back-compat for files without A_f

	Curr.Param.OriginX=Curr.Param.Iniep_ir;//レファレンスカーブが不可逆ひずみの関数ならCurr.ep_irのほうがよい
	Curr.Param.OriginY=Curr.Param.IniSigma_f-Sigma_f00((double)Curr.Param.DirectionX*(Curr.Param.Iniep_ir-Curr.Param.OriginX));
	Curr.Param.OriginXp=Curr.Param.OriginX;
	Curr.Param.OriginYp=Curr.Param.OriginY;
	Curr.Param.OriginX0=Curr.Param.OriginX;
	Curr.Param.OriginY0=Curr.Param.OriginY;
	Curr.Param.DirectionXp=Curr.Param.DirectionX;//新しいステップで不可逆ひずみ速度負をうけて	
	Curr.Param.DirectionYp=Curr.Param.DirectionY;//方向を変える場合は不要

	Curr.Param.flg_freeze=0;

	return(Curr);
};

/*not use	
class Value SetParam::ChgDir(class Value CurrVal,class Value PrevVal){
	
	Curr=CurrVal;
	Prev=PrevVal;

	Curr.Param.DirectionX=-Curr.Param.DirectionX;
	Curr.Param.DirectionY=-Curr.Param.DirectionY;
	
	Curr.Param.OriginY=Prev.Sigma_f-Curr.Param.DirectionY*Sigma_f0(Curr.Param.OriginX,Curr.Param.OriginX);
	Curr.Param.OriginX=Prev.ep;

	Curr.Param.flg_unload=1;

	return(Curr);
}
*/
#endif