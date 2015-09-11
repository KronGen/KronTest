#include "stdafx.h"
#include "InOut.h"

void Error(const TStr& FuncName, const TStr& ErrorMsg){
    printf("Error in %s: %s\n", FuncName.CStr(), ErrorMsg.CStr());
    system("pause");
    exit(1);
}


void ReadParameters(TStr settingsFN, vector<TStr>& out){
    ifstream f;
    f.open(settingsFN.CStr());
    if (!f.is_open())
        Error("ReadParameters", "File with settings (parameters.txt by default) is not found");
    string insteadOfName = "1 ";
    for (int i = 0; i < NFUNC; i++){
        string s;
        getline(f,s);
        if ( s != FUNCNAMES[i])
            Error("ReadParameters", "Wrong syntax in file of settings");
        bool isComment = true;
        while (isComment){
            getline(f,s);
            if (s.find_first_of("//")!=0)
                isComment = false;
        }

        if (s == "default")
            s = insteadOfName;
        else s = insteadOfName + s;
        TStr ts(s.c_str());
        out.push_back(ts);
    }
    f.close();
}

void GetNEFromAccDistr(const TFltPrV& deg, int& nodes, int& edges){
    double nodesD = deg[0].Val2.Val, edgesD = 0;
    for (int i = 0; i < deg.Len(); i++){
        if (i == deg.Len()-1)
            edgesD += deg[i].Val1.Val * deg[i].Val2.Val;
        else edgesD += deg[i].Val1.Val * (deg[i].Val2.Val - deg[i+1].Val2.Val);
    }
    nodes = static_cast<int>(nodesD);
    edges = static_cast<int>(edgesD);
    //	edges /= 2; as Deg = inDeg + outDeg
}

void SaveDegree(const TFltPrV& deg, const TStr& n, bool isIn, bool isCum){
    TFltPrV d(deg);
    d.Sort();
    int nodes, edges;
    GetNEFromAccDistr(d, nodes, edges);
    if (isCum){
        d = TGUtil::GetCCdf(d);
    }
    TSnap::PlotDegDistr(d, nodes, edges, n, n, isCum, false, isIn);
}

// plot all points without binning
void PlotPoints(const TFltPrV& in, const TFltPrV& out, const TStr& name, const TStr& Plt){
    if (Plt == "noncum" || Plt == "all"){
        SaveDegree(in, name, true, false);
        SaveDegree(out, name, false, false);
    }
    if (Plt == "cum" || Plt == "all"){
        SaveDegree(in, name, true, true);
        SaveDegree(out, name, false, true);
    }
}

void PlotDegrees(const vector <TStr>& Parameters, const TFltPrV& In, const TFltPrV& Out, const TStr& Type){
    const TStr& Name = Parameters[NAME];
    const TStr& Plt = Parameters[PLT];
   
    PlotPoints(In, Out, Type + Name, Plt);
   
}

void PrintMtx(const TKronMtx& FitMtxM, ofstream& TFile){
    TFile << "Initiator matrix: ";
    size_t Dim = FitMtxM.GetDim();
    for (size_t i = 0; i < Dim; ++i)
        for (size_t j = 0; j < Dim; ++j){
            TFile << FitMtxM.At(i,j);
            if (!(i == Dim-1 && j == Dim-1))
                TFile << ";";
        }
    TFile << endl;
}

void GetParameters(const vector<TStr>& CommandLineArgs, vector<TStr>& Parameters){
    Env = TEnv(CommandLineArgs[KRONTEST], TNotify::NullNotify);
    // to plot
    const TStr Plt = Env.GetIfArgPrefixStr("-plttype:", "model+kron", "Plotting of big model and/or its Kronecker product (model, kron, model+kron)");
    // type of plots
    const TStr PltType = Env.GetIfArgPrefixStr("-plt:", "all", "Type of plots (cum, noncum, all)");
    
    for (size_t i = 0; i < CommandLineArgs.size(); i++)
        Parameters.push_back(CommandLineArgs[i]);
    Parameters.push_back(Plt); Parameters.push_back(PltType); 
}

void ReadPNGraphFromFile(const TStr args, PNGraph& G){
    Try
        Env = TEnv(args, TNotify::StdNotify);
    const TStr InFNm = Env.GetIfArgPrefixStr("-i:", "../as20graph.txt", "Input graph file (single directed edge per line)");
    // load graph
    G = TSnap::LoadEdgeList<PNGraph>(InFNm, 0, 1);
    Catch
}

ofstream OpenFile(const TStr& fileName)
{
    Try
        ofstream f(fileName.CStr());
    if (f.is_open())
        return f;		
    IAssert(1);
    Catch
}

void GetMtxFromSepLine(const TStr& line, const TStr& separator, TFltV& matrix){
    TStrV strVals;
    line.SplitOnAllAnyCh(separator, strVals);
    for (int i = 0; i < strVals.Len(); i++) 
        matrix.Add(strVals[i].GetFlt());
}

void ReadMtx(const TStr& Mtx, const TInt& MtxSize, TKronMtx& FitMtx){
    TFltV matrix;
    GetMtxFromSepLine(Mtx, ";", matrix);
    FitMtx.GenMtx(matrix.Len() / MtxSize);
    FitMtx.SetMtx(matrix);
}