#ifndef CRF_TRAINER_H_
#define CRF_TRAINER_H_

#include "../CRF.h"
#include "../CRF_Model.h"
#include "../io/CRF_FeatureStreamManager.h"
#include <string.h>

class CRF_Trainer
{
protected:
	CRF_Model* crf_ptr;
	CRF_FeatureStreamManager* ftr_strm_mgr;
	int maxIters;
	char* weight_fname;
	float lr;
	QNUInt32 uttRpt;
	int useLogspace;
	float gvar;
	bool useGvar;
	bool useLabelMask;
	objfunctype objective;
public:
	CRF_Trainer(CRF_Model* crf_in, CRF_FeatureStreamManager* ftr_str_mgr, char* wt_fname);
	virtual ~CRF_Trainer();
	void display_ftrstrm();
	virtual void train();
	virtual void setMaxIters(int);
	virtual void setLR(float lr_in);
	virtual void setUttRpt(QNUInt32 rpt_in);
	virtual void setLogSpace(int);
	virtual void setGaussVar(float gvar_in);
	virtual void setLabelMask(bool useMask);
	virtual void setObjectiveFunction(objfunctype ofunc);
};

#endif /*CRF_TRAINER_H_*/
