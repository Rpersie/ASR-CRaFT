#include "QN_config.h"
#include "fst/fstlib.h"
#include <vector>
#include <string>
#include <map>

#include "CRF.h"
#include "CRF_Model.h"
#include "io/CRF_FeatureStream.h"
#include "io/CRF_FeatureStreamManager.h"
#include "ftrmaps/CRF_StdFeatureMap.h"
#include "ftrmaps/CRF_StdSparseFeatureMap.h"
#include "io/CRF_MLFManager.h"
#include "decoders/CRF_ViterbiDecoder.h"

using namespace std;
typedef StdArc::StateId StateId;
typedef StdArc::Weight Weight;

static struct CRF_FeatureMap_config fmap_config;

static struct {
	char* ftr1_file;
	char* ftr1_format;
	int ftr1_width;
	int ftr1_ftr_start;
	int ftr1_ftr_count;
	int ftr1_window_offset;
	int ftr1_window_len;
	int ftr1_delta_order;
	int ftr1_delta_win;
	char* ftr2_file;
	char* ftr2_format;
	int ftr2_width;
	int ftr2_ftr_start;
	int ftr2_ftr_count;
	int ftr2_window_offset;
	int ftr2_window_len;
	int ftr2_delta_order;
	int ftr2_delta_win;
	char* ftr3_file;
	char* ftr3_format;
	int ftr3_width;
	int ftr3_ftr_start;
	int ftr3_ftr_count;
	int ftr3_window_offset;
	int ftr3_window_len;
	int ftr3_delta_order;
	int ftr3_delta_win;
	int train_sent_start;
	int train_sent_count;
	int window_extent;
	char* hardtarget_file;
	int hardtarget_window_offset;
	char* weight_file;
	int crf_label_size;
	int crf_bunch_size;
	int crf_epochs;
	char* crf_output_format;
	char* crf_output_labelfile;
	char* crf_output_mlffile;
	int crf_mlf_output_frames;
	int crf_mlf_output_states;
	char* crf_align_mlffile;
	char* crf_eval_range;
	char* crf_decode_mode;
	char* crf_featuremap;
	char* crf_featuremap_file;
	int crf_stateftr_start;
	int crf_stateftr_end;
	int crf_transftr_start;
	int crf_transftr_end;
	int crf_states;
	int crf_use_state_bias;
	int crf_use_trans_bias;
	float crf_state_bias_value;
	float crf_trans_bias_value;
	char* crf_lm_bin;
	char* crf_lm_arpa;
	char* crf_isymbols;
	char* crf_osymbols;
	char* crf_olist;
	char* crf_lat_outdir;
	float crf_decode_beam;
	int crf_decode_max_hyp;
	int crf_decode_min_hyp;
	float crf_decode_hyp_inc;
	int verbose;
	int dummy;
} config;

QN_ArgEntry argtab[] =
{
	{ NULL, "ASR CRaFT CRF decoding program version " CRF_VERSION, QN_ARG_DESC },
	{ "ftr1_file", "Input feature file", QN_ARG_STR, &(config.ftr1_file), QN_ARG_REQ },
	{ "ftr1_format", "Input feature file format [pfile]", QN_ARG_STR, &(config.ftr1_format) },
	{ "ftr1_width", "Input feature file columns", QN_ARG_INT, &(config.ftr1_width) },
	{ "ftr1_ftr_start", "First feature used", QN_ARG_INT, &(config.ftr1_ftr_start) },
	{ "ftr1_ftr_count", "Number of features used", QN_ARG_INT, &(config.ftr1_ftr_count) },
	{ "ftr1_window_offset", "Offset of window on ftr1_file (frames)", QN_ARG_INT, &(config.ftr1_window_offset) },
	{ "ftr1_window_len", "Length of window on ftr1_file (frames)", QN_ARG_INT, &(config.ftr1_window_len) },
	{ "ftr1_delta_order", "Delta order", QN_ARG_INT, &(config.ftr1_delta_order) },
	{ "ftr1_delta_win", "Delta window", QN_ARG_INT, &(config.ftr1_delta_win) },
	{ "ftr2_file", "Input feature file", QN_ARG_STR, &(config.ftr2_file) },
	{ "ftr2_format", "Input feature file format [pfile]", QN_ARG_STR, &(config.ftr2_format) },
	{ "ftr2_width", "Input feature file columns", QN_ARG_INT, &(config.ftr2_width) },
	{ "ftr2_ftr_start", "First feature used", QN_ARG_INT, &(config.ftr2_ftr_start) },
	{ "ftr2_ftr_count", "Number of features used", QN_ARG_INT, &(config.ftr2_ftr_count) },
	{ "ftr2_window_offset", "Offset of window on ftr1_file (frames)", QN_ARG_INT, &(config.ftr2_window_offset) },
	{ "ftr2_window_len", "Length of window on ftr1_file (frames)", QN_ARG_INT, &(config.ftr2_window_len) },
	{ "ftr2_delta_order", "Delta order", QN_ARG_INT, &(config.ftr2_delta_order) },
	{ "ftr2_delta_win", "Delta window", QN_ARG_INT, &(config.ftr2_delta_win) },
	{ "ftr3_file", "Input feature file", QN_ARG_STR, &(config.ftr3_file) },
	{ "ftr3_format", "Input feature file format [pfile]", QN_ARG_STR, &(config.ftr3_format) },
	{ "ftr3_width", "Input feature file columns", QN_ARG_INT, &(config.ftr3_width) },
	{ "ftr3_ftr_start", "First feature used", QN_ARG_INT, &(config.ftr3_ftr_start) },
	{ "ftr3_ftr_count", "Number of features used", QN_ARG_INT, &(config.ftr3_ftr_count) },
	{ "ftr3_window_offset", "Offset of window on ftr1_file (frames)", QN_ARG_INT, &(config.ftr3_window_offset) },
	{ "ftr3_window_len", "Length of window on ftr1_file (frames)", QN_ARG_INT, &(config.ftr3_window_len) },
	{ "ftr3_delta_order", "Delta order", QN_ARG_INT, &(config.ftr3_delta_order) },
	{ "ftr3_delta_win", "Delta window", QN_ARG_INT, &(config.ftr3_delta_win) },
	{ "window_extent", "Extent of all windows (frames)", QN_ARG_INT, &(config.window_extent) },
	{ "hardtarget_file", "Target Label File", QN_ARG_STR, &(config.hardtarget_file) },
	{ "hardtarget_window_offset", "Offset of hardtarget file (frames)", QN_ARG_INT, &(config.hardtarget_window_offset) },
	{ "weight_file", "Input Weight File", QN_ARG_STR, &(config.weight_file), QN_ARG_REQ },
	{ "crf_label_size", "Number of CRF output labels", QN_ARG_INT, &(config.crf_label_size), QN_ARG_REQ },
	{ "crf_bunch_size", "Bunch size for CRF processing", QN_ARG_INT, &(config.crf_bunch_size) },
	{ "crf_epochs", "Maximum number of epochs", QN_ARG_INT, &(config.crf_epochs) },
	{ "crf_output_format", "Output format type (ilab|mlf)", QN_ARG_STR, &(config.crf_output_format) },
	{ "crf_output_labelfile", "Output label file name", QN_ARG_STR, &(config.crf_output_labelfile) },
	{ "crf_output_mlffile", "Output MLF file name", QN_ARG_STR, &(config.crf_output_mlffile) },
	{ "crf_align_mlffile", "Target MLF filename for word alignment", QN_ARG_STR, &(config.crf_align_mlffile) },
	{ "crf_mlf_output_frames", "Output frame timings to MLF", QN_ARG_BOOL, &(config.crf_mlf_output_frames) },
	{ "crf_mlf_output_states", "Output state labels to MLF", QN_ARG_BOOL, &(config.crf_mlf_output_states) },
	{ "crf_eval_range", "Range of utterances to evaluate", QN_ARG_STR, &(config.crf_eval_range), QN_ARG_REQ },
	{ "crf_decode_mode", "Mode of decode (decode|align) ", QN_ARG_STR, &(config.crf_decode_mode) },
	{ "crf_featuremap", "Association of inputs to feature functions (stdstate|stdtrans|stdparse|stdparsetrans|file)", QN_ARG_STR, &(config.crf_featuremap) },
	{ "crf_featuremap_file", "File containing map of inputs to feature functions", QN_ARG_STR, &(config.crf_featuremap_file) },
	{ "crf_stateftr_start", "Feature index to start computing state feature funcs", QN_ARG_INT, &(config.crf_stateftr_start) },
	{ "crf_stateftr_end", "Feature index to end computing state feature funcs", QN_ARG_INT, &(config.crf_stateftr_end) },
	{ "crf_transftr_start", "Feature index to start computing trans feature funcs", QN_ARG_INT, &(config.crf_transftr_start) },
	{ "crf_transftr_end", "Feature index to end computing trans feature funcs", QN_ARG_INT, &(config.crf_transftr_end) },
	{ "crf_states", "Number of states per label", QN_ARG_INT, &(config.crf_states) },
	{ "crf_use_state_bias", "Use state bias functions", QN_ARG_BOOL, &(config.crf_use_state_bias) },
	{ "crf_use_trans_bias", "Use transition bias functions", QN_ARG_BOOL, &(config.crf_use_trans_bias) },
	{ "crf_state_bias_value", "Function value for state bias functions", QN_ARG_FLOAT, &(config.crf_state_bias_value) },
	{ "crf_trans_bias_value", "Function value for transition bias functions", QN_ARG_FLOAT, &(config.crf_trans_bias_value) },
	{ "crf_lm_bin", "Language model file name (in OpenFST binary format)", QN_ARG_STR, &(config.crf_lm_bin) },
	{ "crf_lm_arpa", "Language model file name (in ARPA text format)", QN_ARG_STR, &(config.crf_lm_arpa) },
	{ "crf_isymbols", "Input symbols file name (in OpenFST format)", QN_ARG_STR, &(config.crf_isymbols) },
	{ "crf_osymbols", "Output symbols file name (in OpenFST format)", QN_ARG_STR, &(config.crf_osymbols) },
    { "crf_olist", "Ordered list of output labels (for MLF)", QN_ARG_STR, &(config.crf_olist) },
	{ "crf_lat_outdir", "Output directory for lattice files (in OpenFST binary format)", QN_ARG_STR, &(config.crf_lat_outdir) },
	{ "crf_decode_beam", "Beam width for pruning", QN_ARG_FLOAT, &(config.crf_decode_beam) },
	{ "crf_decode_max_hyp", "Maximum hypotheses to keep in beam", QN_ARG_INT, &(config.crf_decode_max_hyp) },
	{ "crf_decode_min_hyp", "Minimum hypotheses to keep in beam", QN_ARG_INT, &(config.crf_decode_min_hyp) },
	{ "crf_decode_hyp_inc", "Increment for hypothesis beam pruning", QN_ARG_FLOAT, &(config.crf_decode_hyp_inc) },
	{ "verbose", "Output status messages", QN_ARG_INT, &(config.verbose) },
	{ NULL, NULL, QN_ARG_NOMOREARGS }
};

static void set_defaults(void) {
	config.ftr1_file="";
	config.ftr1_format="pfile";
	config.ftr1_width=0;
	config.ftr1_ftr_start=0;
	config.ftr1_ftr_count=0;
	config.ftr1_window_offset=0;
	config.ftr1_window_len=1;
	config.ftr1_delta_order=0;
	config.ftr1_delta_win=0;
	config.ftr2_file="";
	config.ftr2_format="pfile";
	config.ftr2_width=0;
	config.ftr2_ftr_start=0;
	config.ftr2_ftr_count=0;
	config.ftr2_window_offset=0;
	config.ftr2_window_len=1;
	config.ftr2_delta_order=0;
	config.ftr2_delta_win=0;
	config.ftr3_file="";
	config.ftr3_format="pfile";
	config.ftr3_width=0;
	config.ftr3_ftr_start=0;
	config.ftr3_ftr_count=0;
	config.ftr3_window_offset=0;
	config.ftr3_window_len=1;
	config.ftr3_delta_order=0;
	config.ftr3_delta_win=0;
	config.window_extent=1;
	config.hardtarget_file="";
	config.hardtarget_window_offset=0;
	config.crf_bunch_size=1;
	config.crf_epochs=10;
	config.crf_output_format="ilab";
	config.crf_output_labelfile=NULL;
	config.crf_output_mlffile=NULL;
	config.crf_align_mlffile=NULL;
	config.crf_mlf_output_frames=0;
	config.crf_mlf_output_states=0;
	config.crf_eval_range=NULL;
	config.crf_decode_mode="decode";
	config.crf_featuremap="stdstate";
	config.crf_featuremap_file=NULL;
	config.crf_stateftr_start=0;
	config.crf_stateftr_end=-1;
	config.crf_transftr_start=0;
	config.crf_transftr_end=-1;
	config.crf_states=1;
	config.crf_use_state_bias=1;
	config.crf_use_trans_bias=1;
	config.crf_state_bias_value=1.0;
	config.crf_trans_bias_value=1.0;
	config.crf_lm_bin=NULL;
	config.crf_lm_arpa=NULL;
	config.crf_isymbols=NULL;
	config.crf_osymbols=NULL;
	config.crf_olist=NULL;
	config.crf_lat_outdir=NULL;
	config.crf_decode_beam=0.0;
	config.crf_decode_max_hyp=0;
	config.crf_decode_min_hyp=0;
	config.crf_decode_hyp_inc=0.05;
	config.verbose=0;
};

static void set_fmap_config(QNUInt32 nfeas) {
	fmap_config.map_type=STDSTATE;
	if (strcmp(config.crf_featuremap,"stdtrans")==0) { fmap_config.map_type=STDTRANS;}
	if (strcmp(config.crf_featuremap,"stdsparse")==0) { fmap_config.map_type=STDSPARSE;}
	if (strcmp(config.crf_featuremap,"stdsparsetrans")==0) { fmap_config.map_type=STDSPARSETRANS;}
	if (strcmp(config.crf_featuremap,"file")==0) { fmap_config.map_type=INFILE;}
	fmap_config.numLabs=config.crf_label_size;
	fmap_config.numFeas=nfeas;
	fmap_config.numStates=config.crf_states;
	fmap_config.useStateFtrs=true;
	fmap_config.stateFidxStart=config.crf_stateftr_start;
	if (config.crf_stateftr_end>=0) {
		fmap_config.stateFidxEnd=config.crf_stateftr_end;
	}
	else {
		fmap_config.stateFidxEnd=nfeas-1;
	}
	if (fmap_config.map_type==STDTRANS || fmap_config.map_type==STDSPARSETRANS) {
		fmap_config.useTransFtrs=true;
		fmap_config.transFidxStart=config.crf_transftr_start;
		if (config.crf_transftr_end>=0) {
			fmap_config.transFidxEnd=config.crf_transftr_end;
		}
		else {
			fmap_config.transFidxEnd=nfeas-1;
		}
	}
	else {
		fmap_config.useTransFtrs=false;
	}
	if (config.crf_use_state_bias == 0) {
		fmap_config.useStateBias=false;
	}
	else {
		fmap_config.useStateBias=true;
	}
	if (config.crf_use_trans_bias == 0) {
		fmap_config.useTransBias=false;
	}
	else {
		fmap_config.useTransBias=true;
	}
	fmap_config.stateBiasVal=config.crf_state_bias_value;
	fmap_config.transBiasVal=config.crf_trans_bias_value;
};


void log_msg(string outstr) {
	if (config.verbose) {
		cout << outstr << endl;
	}
}

void log_errmsg(string errstr) {
	if (config.verbose) {
		cerr << errstr << endl;
	}
}


int main(int argc, const char* argv[]) {
	char* progname;

	bool alignMode=false;
	bool mlfMode=false;
	SymbolTable* oSymTab = NULL;
	SymbolTable* iSymTab = NULL;
	VectorFst<StdArc>* lm_fst = NULL;

	set_defaults();
	QN_initargs(&argtab[0], &argc, &argv, &progname);
	QN_printargs(NULL, progname, &argtab[0]);
	//QN_logger = new QN_Logger_Simple(stdout,stderr,progname);

	cout << "IN LABELS: " << config.crf_label_size << endl;

	if (strcmp(config.crf_decode_mode,"align")==0) { alignMode=true; }

    if (strcmp(config.hardtarget_file,"")==0) { config.hardtarget_file=NULL; }

    if ((config.hardtarget_file==NULL) && (alignMode)) {
    	cerr << "hardtarget_file required when crf_decode_mode=align" << endl;
    	exit(-1);
    }

    if ((config.crf_output_labelfile==NULL) && (config.crf_output_mlffile==NULL)) {
    	cerr << "At least one of crf_output_labelfile or crf_output_mlffile must be assigned" << endl;
    	exit(-1);
    }


	FILE* outl=NULL;
	QN_OutLabStream* labout=NULL;
	if (config.crf_output_labelfile != NULL ) {
		outl=fopen(config.crf_output_labelfile,"w+");
		labout = new QN_OutLabStream_ILab(1, "out_labfile", outl, 255, 1);
	}

	CRF_MLFManager* mlfManager = NULL;
    ofstream mlfstream;
    vector<string> olist;
    if (config.crf_output_mlffile != NULL) {
    	mlfMode=true;
    	if (config.crf_olist == NULL) {
    		cerr << "crf_olist required when crf_output mlffile defined" << endl;
    		exit(-1);
    	}
    	ifstream olistfile;
    	olistfile.open(config.crf_olist);
    	if (olistfile.is_open()) {
    		while (!olistfile.eof()) {
    			string s;
    			getline(olistfile,s);
    			if (s!="") {
    			olist.push_back(s);
    			}}
    	}
    	olistfile.close();
    	if (config.crf_osymbols==NULL) {
    		cerr << "crf_osymbols required when crf_output_mlffile defined" << endl;
    		exit(-1);
    	}
    	oSymTab = SymbolTable::ReadText(config.crf_osymbols);
    	if (config.crf_isymbols==NULL ) {
    		cerr << "crf_osymbols required when crf_output_mlffile defined" << endl;
    		exit(-1);
    	}
    	iSymTab = SymbolTable::ReadText(config.crf_isymbols);
    	mlfstream.open(config.crf_output_mlffile);
    	mlfstream << "#!MLF!#" << endl;

        if (config.crf_align_mlffile != NULL) {
        	try {
        	mlfManager=new CRF_MLFManager(config.crf_align_mlffile, config.crf_olist, oSymTab);
        	}
    		catch (exception &e) {
    			cerr << "Exception: " << e.what() << endl;
    		}
        }
    }


	if (config.crf_lm_bin != NULL) {
		cout << "Reading in LM fst from file: " << config.crf_lm_bin << endl;
		VectorFst<LogArc>* log_fst=VectorFst<LogArc>::Read(config.crf_lm_bin);
		if (log_fst == NULL) {
			cerr << "ERROR: Failed opening file: " << config.crf_lm_bin << endl;
			exit(-1);
		}

		lm_fst=new VectorFst<StdArc>();
		log_msg("Mapping LM to Tropical Ring...");
		Map(*log_fst,lm_fst,LogToStdMapper());
		delete log_fst;

		log_msg("Removing EOW tokens from final PhoneMap/LM/Dict lattice");
		vector<pair<StdArc::Label, StdArc::Label> >* ipairs;
		vector<pair<StdArc::Label, StdArc::Label> >* opairs;
		ipairs = new vector<pair<StdArc::Label, StdArc::Label> >;
		opairs = new vector<pair<StdArc::Label, StdArc::Label> >;
		pair<StdArc::Label,StdArc::Label> mypair1(55,0);
		pair<StdArc::Label,StdArc::Label> mypair2(56,0);
		pair<StdArc::Label,StdArc::Label> mypair3(57,0);
		pair<StdArc::Label,StdArc::Label> mypair4(58,0);
		(*ipairs).push_back(mypair1);
		(*ipairs).push_back(mypair2);
		(*ipairs).push_back(mypair3);
		(*ipairs).push_back(mypair4);
		Relabel(lm_fst,*ipairs,*opairs);
	}

    ftrmaptype trn_ftrmap = STDSTATE;

	if (strcmp(config.crf_featuremap,"stdtrans")==0) { trn_ftrmap=STDTRANS;}
	if (strcmp(config.crf_featuremap,"stdsparse")==0) { trn_ftrmap=STDSPARSE;}
	if (strcmp(config.crf_featuremap,"stdsparsetrans")==0) { trn_ftrmap=STDSPARSETRANS;}
	if (strcmp(config.crf_featuremap,"file")==0) { trn_ftrmap=INFILE;}

	if ((trn_ftrmap == INFILE) && (config.crf_featuremap_file==NULL)) {
		cerr << "ERROR: crf_featuremap_file must be non-NULL when crf_featuremap set to 'file'" << endl;
		exit(-1);
	}

	CRF_FeatureStreamManager* str2=NULL;
	CRF_FeatureStreamManager* str3=NULL;


    CRF_FeatureStreamManager str1(1,"ftr1_file",config.ftr1_file,config.ftr1_format,config.hardtarget_file,config.hardtarget_window_offset,
							(size_t) config.ftr1_width, (size_t) config.ftr1_ftr_start, (size_t) config.ftr1_ftr_count,
							config.window_extent, config.ftr1_window_offset, config.ftr1_window_len,
							config.ftr1_delta_order, config.ftr1_delta_win,
							config.crf_eval_range, 0,
							NULL,0,0,0,SEQUENTIAL);
	if (strcmp(config.ftr2_file,"") != 0) {

		str2=new CRF_FeatureStreamManager(1,"ftr2_file",config.ftr2_file,config.ftr2_format,config.hardtarget_file,config.hardtarget_window_offset,
							(size_t) config.ftr2_width, (size_t) config.ftr2_ftr_start, (size_t) config.ftr2_ftr_count,
							config.window_extent, config.ftr2_window_offset, config.ftr2_window_len,
							config.ftr2_delta_order, config.ftr2_delta_win,
							config.crf_eval_range, 0,
							NULL,0,0,0,SEQUENTIAL);
		str1.join(str2);
	}
	if (strcmp(config.ftr3_file,"") != 0) {

		str3=new CRF_FeatureStreamManager(1,"ftr3_file",config.ftr3_file,config.ftr3_format,config.hardtarget_file,config.hardtarget_window_offset,
							(size_t) config.ftr3_width, (size_t) config.ftr3_ftr_start, (size_t) config.ftr3_ftr_count,
							config.window_extent, config.ftr3_window_offset, config.ftr3_window_len,
							config.ftr3_delta_order, config.ftr3_delta_win,
							config.crf_eval_range, 0,
							NULL,0,0,0,SEQUENTIAL);
		str1.join(str3);

	}
    cout << "Feature File created" << endl;


	CRF_FeatureStream* crf_ftr_str = str1.trn_stream;
	CRF_Model my_crf(config.crf_label_size);
	cout << "LABELS: " << my_crf.getNLabs() << endl;


	set_fmap_config(str1.getNumFtrs());
	my_crf.setFeatureMap(CRF_FeatureMap::createFeatureMap(&fmap_config));

	bool openchk=my_crf.readFromFile(config.weight_file);
	if (!openchk) {
		cerr << "ERROR: Failed opening file: " << config.weight_file << endl;
		exit(-1);
	}

	//CRF_ViterbiDecoder vd(crf_ftr_str,&my_crf);
	crf_ftr_str->rewind();
	QN_SegID segid = crf_ftr_str->nextseg();
	int count=0;
	while (segid != QN_SEGID_BAD) {
		if (count %100 == 0) {
			cout << "Processing segment " << count << endl;
		}
		if (config.crf_olist != NULL) {
			cout << "Processing file: " << olist.at(count);
			cout << " (" << count << ")" << endl;
		}
		try {
			time_t rawtime;
			time(&rawtime);
			char* time1 = ctime(&rawtime);
			time1[strlen(time1)-1]='\0';
			log_msg(time1);
			CRF_ViterbiDecoder vd(crf_ftr_str,&my_crf);
			VectorFst<StdArc>* best_lat=new VectorFst<StdArc>();
			int nodeCnt=vd.nStateDecode(best_lat,lm_fst,config.crf_decode_beam,config.crf_decode_min_hyp,
					config.crf_decode_max_hyp,config.crf_decode_hyp_inc);
			if (config.crf_lat_outdir != NULL) {
				// Put code here to dump the lattice file out to the latdir in
				// OpenFst format
				string fst_fname=string(config.crf_lat_outdir)+"/fst."+stringify(count)+".final.fst";
				log_msg("*	*Writing lattice to "+fst_fname);
				best_lat->Write(fst_fname);
			}

			if (config.crf_output_mlffile != NULL) {
				VectorFst<StdArc>* shortest_fst = new VectorFst<StdArc>();
				log_msg("Finding Shortest Path");
				ShortestPath(*best_lat,shortest_fst,1);
				//log_msg("Projecting to Output Symbols");
				//Project(shortest_fst,PROJECT_OUTPUT);
				log_msg("Removing epsilons");
				RmEpsilon(shortest_fst);
				log_msg("Top sorting");
				TopSort(shortest_fst);

				//	Iterates over the FSTs states.
				if (oSymTab != NULL && mlfstream.is_open()) {
					 mlfstream << "\"" << olist.at(count) << "\"" << endl;
					 cout << "\"" << olist.at(count) << "\" : ";
				}
				int start=0;
				int current=0;
				int lastId=-1;
				int lastPhn=-1;
				int lastWrd=-1;
				for (StateIterator<StdFst> siter(*shortest_fst); !siter.Done(); siter.Next())
				{
  					StateId state_id = siter.Value();
  					//Iterates over state state_id's arcs.
  					for (ArcIterator<StdFst> aiter(*shortest_fst, state_id); !aiter.Done(); aiter.Next())
					{
  						const StdArc &arc = aiter.Value();
  						if ((arc.olabel ==0) && (arc.ilabel != 0)) {
  							// In here we process an epsilon on the word path
  							if (config.crf_mlf_output_states) {
  								if (arc.ilabel != lastPhn) {
  									if (lastPhn != -1) {
  										if (config.crf_mlf_output_frames) {
  											mlfstream << start << "\t" << (current-1) << "\t";
  										}
  										mlfstream << iSymTab->Find(lastPhn) << endl;
  									}
  									lastPhn=arc.ilabel;
  									start=current;
  								}
  							}
  							current++;
  						}
  						else if (arc.olabel !=0 && arc.ilabel==0) {
  							// Here we process an epsilon on the phone path and a word
  							if (oSymTab != NULL && mlfstream.is_open()) {
  								if (config.crf_mlf_output_frames) {
  									mlfstream << start << "\t" << (current-1) << "\t";
  								}
  								if (config.crf_mlf_output_states) {
  									mlfstream << iSymTab->Find(lastPhn) << "\t";
  								}
  								mlfstream  << oSymTab->Find(arc.olabel) << endl;
  								cout << oSymTab->Find(arc.olabel) << " ";
  								start=current;
  								lastPhn=-1;
  							}
  						}
  						else if (arc.olabel !=0 && arc.ilabel!=0) {
  							// And finally we process no epsilons on any line
							if (config.crf_mlf_output_states) {
  								if (arc.ilabel != lastPhn) {
  									if (lastPhn != -1) {
  										if (config.crf_mlf_output_frames) {
  											mlfstream << start << "\t" << (current-1) << "\t";
  										}
  										mlfstream << iSymTab->Find(lastPhn) << endl;
  									}
  									lastPhn=arc.ilabel;
  									start=current;
  								}
  							}
							current++;
  							if (oSymTab != NULL && mlfstream.is_open()) {
  								if (config.crf_mlf_output_frames) {
  									mlfstream << start << "\t" << (current-1) << "\t";
  								}
  								if (config.crf_mlf_output_states) {
  									mlfstream << iSymTab->Find(lastPhn) << "\t";
  								}
  								mlfstream  << oSymTab->Find(arc.olabel) << endl;
  								cout << oSymTab->Find(arc.olabel) << " ";
  								start=current;
  								lastPhn=-1;
  							}
  							//current++;
  						}

  						/*
  						if (arc.olabel == 0) {
  							if (config.crf_mlf_output_states) {
  							if (arc.ilabel != lastId) {
  								if (lastId != -1) {
  									if (config.crf_mlf_output_frames) {
  										mlfstream << start << "\t" << (current-1) << "\t";
  									}
  									mlfstream << iSymTab->Find(lastId) << endl;
  								}
  								lastId=arc.ilabel;
  								start=current;
  							}
  							}
  							current++;
  						}
  						else {
  							if (oSymTab != NULL && mlfstream.is_open()) {
  								if (config.crf_mlf_output_frames) {
  									mlfstream << start << "\t" << (current-1) << "\t";
  								}
  								if (config.crf_mlf_output_states) {
  									mlfstream << iSymTab->Find(lastId) << "\t";
  								}
  								mlfstream  << oSymTab->Find(arc.olabel) << endl;
  								cout << oSymTab->Find(arc.olabel) << " ";
  								start=current;
  								lastId=-1;
  							}
  						}
  						*/
					}
				}
				if (oSymTab != NULL && mlfstream.is_open()) {
					if (lastPhn != -1) {
							if (config.crf_mlf_output_states) {

									if (lastPhn != -1) {
										if (config.crf_mlf_output_frames) {
											mlfstream << start << "\t" << (current-1) << "\t";
										}
										mlfstream << iSymTab->Find(lastPhn) << endl;
									}

							}
					}
					mlfstream << "." << endl;
					cout << "." << endl;
				}
				delete shortest_fst;
			}
			time(&rawtime);
			char* time2 = ctime(&rawtime);
			time2[strlen(time2)-1]='\0';
			log_msg(time2);

			delete best_lat;
		}
		catch (exception &e) {
			cerr << "Exception: " << e.what() << endl;
			exit(0);
		}
		segid=crf_ftr_str->nextseg();
		count++;
	}
	delete lm_fst;
}
