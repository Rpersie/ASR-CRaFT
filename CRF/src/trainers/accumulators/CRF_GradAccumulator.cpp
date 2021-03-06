/*
 * CRF_GradAccumulator.cpp
 *
 *  Created on: Jul 7, 2009
 *      Author: fosler
 */

#include "CRF_GradAccumulator.h"
#include "../gradbuilders/CRF_GradBuilder.h"

CRF_GradAccumulator::CRF_GradAccumulator(CRF_Model *myCrf,
										bool myLogspace,
										int myNStates)
	: crf(myCrf), useLogspace(myLogspace), nStates(myNStates) {

	uttReport=0;
	objective=EXPF;

	// Added by Ryan
	minibatch = QN_UINT32_MAX;
}

CRF_GradAccumulator::~CRF_GradAccumulator() {

}

double CRF_GradAccumulator::accumulateGradient(CRF_FeatureStreamManager* ftr_str_mgr,
												int nStreams,
												double* grad,
												QNUInt32 *uttCount) {

	//CRF_GradBuilder *gbuild=CRF_GradBuilder::create(crf,useLogspace,nStates);

	//cerr << "Creating gradbuilder" << endl;
	CRF_GradBuilder *gbuild=CRF_GradBuilder::create(crf,this->objective);
	if (nStreams!=1) {
		cerr << "CRF_GradAccumulator can only handle one stream" << endl;
		exit(1);
	}

	// Added by Ryan
	if (minibatch != 1) {
		cerr << "CRF_GradAccumulator::accumulateGradient can only handle minibatch size 1." << endl;
		exit(1);
	}

	CRF_FeatureStream *ftr_str=ftr_str_mgr->trn_stream;

	// Rewind the stream
	ftr_str->rewind();
	QN_SegID segid = ftr_str->nextseg();

	// SHOULD CODE BETTER TO THROW EXCEPTION
	if (segid == QN_SEGID_BAD) {
		cerr << "Feature stream contains no utterances!" << endl;
		exit(1);
	}

	time_t rawtime;
	double totLogLi=0.0;
	unsigned int uCounter=0;
	double tmp_Zx;
	QNUInt32 nlambda=this->crf->getLambdaLen();

	for (QNUInt32 i=0;i<nlambda;i++) {
		grad[i]=0.0;
	}

	// now loop over all utterances
	do {
		// each call to buildGradient updates grad
		double tmpLogLi=gbuild->buildGradient(ftr_str,grad,&tmp_Zx);
		double logLi=tmpLogLi - tmp_Zx;
		totLogLi += logLi;

		if (this->uttReport>0 && (uCounter % this->uttReport == 0)) {
			time(&rawtime);
			char* time = ctime(&rawtime);
			time[strlen(time)-1]='\0';
			cout << time << " Finished Utt: " << uCounter << " logLi: " << logLi;
			cout << " Avg LogLi: " << totLogLi/(uCounter+1) << " Zx: " << tmp_Zx;
			cout << " Numerator: " << tmpLogLi << endl;
		}
		uCounter++;

		segid = ftr_str->nextseg();
	} while (segid != QN_SEGID_BAD);

	cerr << "Deleting gbuild" << endl;
	delete gbuild;
	cerr << "gbuild deleted" << endl;
	    // clean up stream
	ftr_str->rewind();
	ftr_str->nextseg();
	*uttCount=uCounter;
	return totLogLi;
}

// Added by Ryan
double CRF_GradAccumulator::accumulateGradientMinibatch(CRF_FeatureStreamManager* ftr_str_mgr,
												int nStreams,
												double* grad,
												double* Zx_out,
												QNUInt32 *uttCount,
												bool *endOfIter) {

	if (nStreams == 1) {
		cerr << "Error in CRF_GradAccumulator::accumulateGradientMinibatch(): "
				<< "CRF_GradAccumulator::accumulateGradient() should be called when the number of threads is 1." << endl
				<< "Do you forget to set the number of threads?" << endl
				<< "If so, set it to a number greater than 1 and use CRF_Pthread_GradAccumulator::accumulateGradientMinibatch() instead."
				<< endl;
		exit(1);
	} else {
		cerr << "Error in CRF_GradAccumulator::accumulateGradientMinibatch(): "
				<< "Use CRF_Pthread_GradAccumulator::accumulateGradientMinibatch() for multithreading."
				<< endl;
		exit(1);
	}
}
