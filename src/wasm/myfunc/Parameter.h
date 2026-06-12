#if !defined ___Parameter

#define ___Parameter

class Parameter{
public:
	//for Initial Setting
	double Initime,Iniep,Iniepd,Iniep_ir,Iniepd_ir,IniSigma,IniSigma_f,Initau;
	//for Elastic
	double b,E_ini,Sigma_0;
	//for Reference
	double OriginX,OriginY,OriginXp,OriginYp,OriginX0,OriginY0;
	double DirectionX,DirectionY,DirectionXp,DirectionYp;
	double ProportionX,ProportionY,RefParam[10];
	int RefFunction;
	//decay Param
	double 	r1,r2,ep_ir_0,c;
	//gv(new ISOTACH)
	double alpha,m,epd_ir_0;
	double alpha2,m2,epd_ir_2,epd_ir_int;
	int	gvtype;
	//combination
	double lambda_V;
	double th1,th2,rf_strain,curve;
	//calculation
	int MaxIteration;
	double Precision;
	int Integration;

	class History history;
	int flg_unload;
	int flg_freeze;

	//for aging
	double a_f,b_f;
	double lambda_f;
	double 	r1_f,r2_f,ep_ir_0_f,c_f;
	double	A_f;	//temperature/ageing factor (was hard-coded 0.957 in GTE::A_f)

protected:	

private:

};

#endif

/*
	//for Initial Setting
	double Initime,Iniep,Iniepd,Iniep_ir,Iniepd_ir,IniSigma,IniSigma_f,Initau;
	//for Elastic
	double b,E_ini,Sigma_0;
	//for Reference
	double OriginX,OriginY,OriginXp,OriginYp,OriginX0,OriginY0;
	double DirectionX,DirectionY,DirectionXp,DirectionYp;
		// ProportionX レファレンスのプロポーション
		// ProportionY 粘性のプロポーション
	double ProportionX,ProportionY,RefParam[10],RefParamU[10];
	int RefFunction;
	//decay Param
	double 	r1,r2,ep_ir_0,c;
	//gv(new ISOTACH)
	double alpha,m,epd_ir_0;
	double alpha2,m2,epd_ir_2,epd_ir_int;
	int	gvtype;
	//combination
	double lambda_V;
	double th1,th2,rf_strain,curve;
	//calculation
	int MaxIteration;
	double Precision;
	int Integration;

	class History history;
	int flg_unload;
	int flg_freeze;
	double Sigma_Un,ep_Un,Sigma_Re,ep_Re;

*/