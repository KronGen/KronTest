void Error(const TStr& FuncName, const TStr& ErrorMsg);
void ReadParameters(TStr settingsFN, vector<TStr>& out);
void PlotDegrees(const vector <TStr>& Parameters, const TFltPrV& In, const TFltPrV& Out, const TStr& Type);
void GetParameters(const vector<TStr>& CommandLineArgs, vector<TStr>& Parameters);
void PrintMtx(const TKronMtx& FitMtxM, ofstream& TFile);
void ReadPNGraphFromFile(const TStr args, PNGraph& G);
ofstream OpenFile(const TStr& fileName);
void ReadMtx(const TStr& Mtx, const TInt& MtxSize, TKronMtx& FitMtx);