#if !defined ___control

#define ___control

enum TypeofEvent {
	End=0,
    StrainRateConstant=1,
    Relaxation=2,
    AccControl=3,
    StressRateConstant=4,
    Creep=5,
    StressStrainControl=6
};

class Control{
public:
	enum TypeofEvent TypeofEvent;
    double StartTime,EndTime;
	//CTRValue
	//epd for StrainRateConstant,Relaxation,AccControl
	//StressRate for StressRateConstant,Creep
	//alpha for StressStrainControl
	double CTRValue;
	double dt;

	void Load_Control(ifstream& CTRFile);

protected:

private:

};

void Control::Load_Control(ifstream& CTRFile){
	int temp;

    CTRFile >> temp;
	TypeofEvent = (enum TypeofEvent)temp;
	if(TypeofEvent==End){
		StartTime=0;
		EndTime=0;
		dt=0;
		CTRValue=0;
	}else{
	    CTRFile >> StartTime;
		CTRFile >> EndTime;
		CTRFile >> dt;
		CTRFile >> CTRValue;
	}
}

#endif