//最終更新2000年12月7日
#if !defined ___History
#define ___History

//あるステップから次のステップにいたる間に生じた応力変化
//あるステップから次のステップにいたる間の平均的tau(不可逆ひずみ増分の絶対値の和)
/*StateVal[i-1].tau->Data_v[i].tau->StateVal[i].tau*/
class History{
public:
	History(void){before=NULL;DSigma_V=0;DSigma_f=0;tau=0;};
	~History(void){};

	void IniHistory(double DSigma_V_n,double DSigma_f_n,double tau_n);
	void RecHistory(double DSigma_V,double DSigma_f,double tau);
	History *before;
	double DSigma_V,DSigma_f,tau;

protected:

private:

};

void History::RecHistory(double DSigma_V_n,double DSigma_f_n,double tau_n){
	class History *temp_History;

	temp_History=new History;
	temp_History->before=before;
	temp_History->DSigma_V=DSigma_V;
	temp_History->DSigma_f=DSigma_f;
	temp_History->tau=tau;

	before=temp_History;
	DSigma_V=DSigma_V_n;
	DSigma_f=DSigma_f_n;
	tau=tau_n;

}

void History::IniHistory(double DSigma_V_n,double DSigma_f_n,double tau_n){

	before=NULL;
	DSigma_V=DSigma_V_n;
	DSigma_f=DSigma_f_n;
	tau=tau_n;

}

#endif