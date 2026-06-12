#if !defined ___Value

#define ___Value

class Value{
public:
	double time,ep,epd,ep_ir,epd_ir,Sigma,Sigma_f0,Sigma_f,Sigma_fy,Sigma_V,tau;
	double Sigma_V_Iso;//without decay
	bool result = true;   // initialized (orig. relied on uninitialized memory)
	Parameter Param;
	void Output_Header(ofstream& DATAFILE);
	void Output_Value(ofstream& DATAFILE);	

protected:

private:

};

void Value::Output_Header(ofstream& DATFile){

	DATFile
		<< "time (s)" << "\t"
		<< "ep" << "\t"
		<< "epd" << "\t"
		<< "ep_ir" << "\t"
		<< "epd_ir" << "\t"
		<< "Sigma" << "\t"
		<< "Sigma_f" << "\t"
		<< "Sigma_fy" << "\t"
		<< "tau" << "\n";
};

void Value::Output_Value(ofstream& DATFile){

	DATFile
		<< time << "\t" // Export time in second
//		<< setprecision(12) << time/60.0/60.0 << "\t" // Export time in hour
		<< ep << "\t"
		<< epd << "\t"
		<< ep_ir << "\t"
		<< epd_ir << "\t"
		<< Sigma << "\t"
		<< Sigma_f << "\t"
		<< Sigma_fy << "\t"
		<< tau << "\n";
};

#endif