#include "stdafx.h"
#include "KronGen.h"
#include "GenPy.h"
#include "InOut.h"
#include "Stat.h"

ofstream TFile;

void CheckParams(const TStr& model_gen, const TStr& model_plt)
{
    try
    {
        if (model_gen != model_plt)
        {
            if (model_gen != "model+kron")
                throw 1;
            if (model_gen == "none" && model_plt != "none")
                throw 1;
            if (model_plt == "model+kron" && model_gen != "model+kron")
                throw 1;
        }
    }
    catch (int i){
        printf("Inconsistency in KRONTEST parameters\n");
        system("pause");
        exit(1);
    }
}

int BasicGraphGen(const TStr args, PNGraph &GD){
    Env = TEnv(args, TNotify::StdNotify);
    Env.PrepArgs(TStr::Fmt("Graph generation. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
    TExeTm ExeTm;
    Try
        const TStr OutFNm = Env.GetIfArgPrefixStr("-o:", "output.txt", "Output graph filename");
    const TStr Plot = Env.GetIfArgPrefixStr("-gt:", "e", "Which generator to use:"
        "\n\tf: Complete graph. Required parameters: n (number of nodes)"
        "\n\ts: Star graph. Required parameters: n (number of nodes)"
        "\n\t2: 2D Grid. Required parameters: n (number of rows), m (number of columns)"
        "\n\te: Erdos-Renyi (G_nm). Required parameters: n (number of nodes), m (number of edges)"
        "\n\tk: Random k-regular graph. Required parameters: n (number of nodes), k (degree of every node)"
        "\n\tb: Albert-Barabasi Preferential Attachment. Required parameters: n (number of nodes), k (edges created by each new node)"
        "\n\tp: Random Power-Law graph. Required parameters: n (number of nodes), p (power-law degree exponent)"
        "\n\tc: Copying model by Kleinberg et al. Required parameters: n (number of nodes), p (copying probability Beta)"
        "\n\tw: Small-world model. Required parameters: n (number of nodes), k (each node is connected to k nearest neighbors in ring topology), p (rewiring probability)\n"
        );
    const int N = Env.GetIfArgPrefixInt("-n:", 1000, "Number of nodes");
    const int M = Env.GetIfArgPrefixInt("-m:", 5000, "Number of edges");
    const double P = Env.GetIfArgPrefixFlt("-p:", 0.1, "Probability/Degree-exponent");
    const int K = Env.GetIfArgPrefixInt("-k:", 3, "Degree");

    if (Env.IsEndOfRun()) { return 0; }
    TInt::Rnd.PutSeed(0); // initialize random seed
    printf("Generating...\n");

    TStr DescStr;
    PUNGraph G;
    if (Plot == "f") {
        G = TSnap::GenFull<PUNGraph>(N);
        DescStr = TStr::Fmt("Undirected complete graph.");
    } else
        if (Plot == "s") {
            G = TSnap::GenStar<PUNGraph>(N, false);
            DescStr = TStr::Fmt("Undirected star graph (1 center node connected to all other nodes).");
        } else
            if (Plot == "2") {
                G = TSnap::GenGrid<PUNGraph>(N, M, false);
                DescStr = TStr::Fmt("Undirected 2D grid of %d rows and %d columns.", N, M);
            } else
                if (Plot == "e") {
                    G = TSnap::GenRndGnm<PUNGraph>(N, M, false);
                    DescStr = TStr::Fmt("Undirected Erdos-Renyi random graph.");
                } else
                    if (Plot == "k") {
                        G = TSnap::GenRndDegK(N, K);
                        DescStr = TStr::Fmt("Undirected k-regular random graph (every node has degree K).");
                    } else
                        if (Plot == "b") {
                            G = TSnap::GenPrefAttach(N, K);
                            DescStr = TStr::Fmt("Undirected Albert-Barabasi Preferential Attachment graph (each new node creades k preferentially attached edges).");
                        } else
                            if (Plot == "p") {
                                G = TSnap::GenRndPowerLaw(N, P, true);
                                DescStr = TStr::Fmt("Random Graph with Power-Law degree distribution with exponent P.");
                            } else
                                if (Plot == "c") {
                                    G = TSnap::ConvertGraph<PUNGraph>(TSnap::GenCopyModel(N, P));
                                    DescStr = TStr::Fmt("Copying model by Kleinberg et al. Node u comes, selects a random v, and with prob P it links to v, with 1-P links u links to neighbor of v. Power-law degree slope is 1/(1-P).");
                                } else
                                    if (Plot == "w") {
                                        G = TSnap::GenSmallWorld(N, K, P);
                                        DescStr = TStr::Fmt("Watts-Strogatz Small-world model. Every node links to K other nodes.");
                                    }
                                    printf("done.\n");
                                    // save graph
                                    //TSnap::SaveEdgeList(G, OutFNm, DescStr);
                                    GD = TSnap::ConvertGraph<PNGraph>(G);


                                    return 0;

                                    Catch
                                        printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
                                    return 0;
}


int InitKronecker(const TStr args, PNGraph &GD, TKronMtx& FitMtx){
	Env = TEnv(args, TNotify::StdNotify);
	Env.PrepArgs(TStr::Fmt("Kronecker graphs. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
	TExeTm ExeTm;
	Try
	Env = TEnv(args, TNotify::StdNotify);
	//const TStr InFNm = Env.GetIfArgPrefixStr("-i:", "../as20graph.txt", "Input graph file (single directed edge per line)");
	TStr OutFNm = Env.GetIfArgPrefixStr("-o:", "initMatrix.txt", "Output file prefix");
	const TInt NZero = Env.GetIfArgPrefixInt("-n0:", 2, "Innitiator matrix size");
	const TStr InitMtx = Env.GetIfArgPrefixStr("-m:", "0.9 0.7; 0.5 0.2", "Init Gradient Descent Matrix (R=random)").GetLc();
	const TStr Perm = Env.GetIfArgPrefixStr("-p:", "d", "Initial node permutation: d:Degree, r:Random, o:Order").GetLc();
	const TInt GradIter = Env.GetIfArgPrefixInt("-gi:", 10, "Gradient descent iterations");
	const TFlt LrnRate = Env.GetIfArgPrefixFlt("-l:", 1e-5, "Learning rate");
	const TFlt MnStep = Env.GetIfArgPrefixFlt("-mns:", 0.005, "Minimum gradient step");
	const TFlt MxStep = Env.GetIfArgPrefixFlt("-mxs:", 0.05, "Maximum gradient step");
	const TInt WarmUp =  Env.GetIfArgPrefixInt("-w:", 10000, "Samples to warm up");
	const TInt NSamples = Env.GetIfArgPrefixInt("-s:", 100000, "Samples per gradient estimation");
	//const TInt GradType = Env.GetIfArgPrefixInt("-gt:", 1, "1:Grad1, 2:Grad2");
	const bool ScaleInitMtx = Env.GetIfArgPrefixBool("-sim:", true, "Scale the initiator to match the number of edges");
	const TFlt PermSwapNodeProb = Env.GetIfArgPrefixFlt("-nsp:", 1.0, "Probability of using NodeSwap (vs. EdgeSwap) MCMC proposal distribution");
	//if (OutFNm.Empty()) { OutFNm = TStr::Fmt("%s-fit%d", InFNm.GetFMid().CStr(), NZero()); }
	printf("%s\n", OutFNm.CStr());
	// load graph
	cout << "n0 = " << NZero << endl;
	// fit
	FILE *F = fopen(OutFNm.CStr(), "w");
	TKronMtx InitKronMtx = InitMtx=="r" ? TKronMtx::GetRndMtx(NZero, 0.1) : TKronMtx::GetMtx(InitMtx);
	InitKronMtx.Dump("INIT PARAM", true);
	TKroneckerLL KronLL(GD, InitKronMtx, PermSwapNodeProb);
	fprintf(F, "INIT PARAM\t%s, MTX SUM %f\n", InitKronMtx.GetMtxStr().CStr(), InitKronMtx.GetMtxSum());
	if (ScaleInitMtx) {
		InitKronMtx.SetForEdges(GD->GetNodes(), GD->GetEdges()); }
	KronLL.InitLL(GD, InitKronMtx);
	InitKronMtx.Dump("SCALED PARAM", true);
	fprintf(F, "SCALED PARAM\t%s, MTX SUM %f\n", InitKronMtx.GetMtxStr().CStr(), InitKronMtx.GetMtxSum());
	KronLL.SetPerm(Perm.GetCh(0));
	double LogLike = 0;
	//if (GradType == 1) {
	LogLike = KronLL.GradDescent(GradIter, LrnRate, MnStep, MxStep, WarmUp, NSamples);
	//} else if (GradType == 2) {
	//  LogLike = KronLL.GradDescent2(GradIter, LrnRate, MnStep, MxStep, WarmUp, NSamples); }
	//else{ Fail; }
	//const TKronMtx& FitMtx = KronLL.GetProbMtx();
	FitMtx = KronLL.GetProbMtx();

//	fprintf(F, "Input\t%s\n", InFNm.CStr());
	TStrV ParamV; Env.GetCmLn().SplitOnAllCh(' ', ParamV);
	fprintf(F, "Command line options\n");
	for (int i = 0; i < ParamV.Len(); i++) {
		fprintf(F, "\t%s\n", ParamV[i].CStr()+(ParamV[i][0]=='-'?1:0)); }
	fprintf(F, "Loglikelihood\t%10.2f\n", LogLike);
	fprintf(F, "Absolute error (based on expected number of edges)\t%f\n", KronLL.GetAbsErr());
	fprintf(F, "RunTime\t%g\n", ExeTm.GetSecs());
	fprintf(F, "Estimated initiator\t%s, mtx sum %f\n", FitMtx.GetMtxStr().CStr(), FitMtx.GetMtxSum());
	/*if (ScaleInitMtx) {
		FitMtx.SetForEdgesNoCut(GD->GetNodes(), GD->GetEdges()); }
	fprintf(F, "Scaled initiator\t%s, mtx sum %f\n", FitMtx.GetMtxStr().CStr(), FitMtx.GetMtxSum());
	FitMtx.Normalize();
	fprintf(F, "Normalized initiator\t%s, mtx sum %f\n", FitMtx.GetMtxStr().CStr(), FitMtx.GetMtxSum());*/
	fclose(F);

	Catch
		printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
	return 0;
}

void GenRandomMtx(const int& MtxRndSize, TKronMtx& FitMtx){
    FitMtx.SetRndMtx(MtxRndSize);
}

void GenNewMtx(PNGraph& model, const TStr& args, TKronMtx& FitMtx){
    TExeTm execTime;
    InitKronecker(args, model, FitMtx);
    TFile << "Time of creation of init matrix: " <<  execTime.GetTmStr() << endl;
}


// read or get random mtx
bool GetMtx(const TStr& MtxArgs, TKronMtx& FitMtxModel){
    Env = TEnv(MtxArgs, TNotify::StdNotify);
    // how to generate initiator matrix
    const TStr Mtx = Env.GetIfArgPrefixStr("-m:", "random", "Init Kronecker matrix");
    // if matrix will be generated, its size is an argument of KRONFIT cmd line
    const TInt MtxSize = Env.GetIfArgPrefixInt("-rs:", 2, "Size of randomized Kronecker matrix");
    // get Kronecker init matrix
    if (Mtx == "create") return false;
    if (Mtx == "random")
        GenRandomMtx(MtxSize, FitMtxModel);
    else 
        ReadMtx(Mtx, MtxSize, FitMtxModel);
    return true;
}


// get model graph according to args
void GetModel(const TStr& Args, PNGraph& G){
	Env = TEnv(Args, TNotify::NullNotify);
	const TStr Gen = Env.GetIfArgPrefixStr("-g:", "gen", "How to get model graph: read, gen, deg, genpy");
	const TStr InFNm = Env.GetIfArgPrefixStr("-i:", "", "Input graph file (single directed edge per line)");
	
	TExeTm execTime;
	if (Gen == "gen")
		BasicGraphGen(Args, G);
	else if (Gen == "read")
		ReadPNGraphFromFile(InFNm, G);
	else if (Gen == "genpy")
	{
		PUNGraph GU;
		GenPy(GU, TFile, Args);	
		G = TSnap::ConvertGraph<PNGraph>(GU);
	}
	TFile << "Time of getting model: " <<  execTime.GetTmStr() << endl;
	/*TFile << "Model graph: " << G->GetNodes() << " nodes, " << G->GetEdges() << " edges\n";
	TIntV DegV;
	TSnap::GetDegSeqV(G, DegV);
	execTime.Tick();
	PUNGraph Conf = TSnap::GenConfModel(DegV);
	TFile << "Time of getting configuration model: " <<  execTime.GetTmStr() << endl;
	cout << "Undirected configuration model: " << Conf->GetNodes() << " nodes, " << Conf->GetEdges() << " edges\n";
	PNGraph ConfD = TSnap::ConvertGraph<PNGraph>(Conf);
	SaveAndPlot(ConfD, "conf", false);
	TFile << "Clustering coefficient of configuration model: " << TSnap::GetClustCf(ConfD) << endl;
	TSnap::PlotClustCf(ConfD,"conf");*/
}

void GenKron(const TStr& Args, TKronMtx& FitMtx, TFltPrV& KronDegAvgIn, TFltPrV& KronDegAvgOut){
	Env = TEnv(Args, TNotify::NullNotify);
	TExeTm ExecTime;
	// number of Kronecker graphs to generate
	const TInt NKron = Env.GetIfArgPrefixInt("-n:", 1, "Number of generated Kronecker graphs");
	// iterations of Kronecker product
	const TInt NIter = Env.GetIfArgPrefixInt("-i:", 10, "Iterations of Kronecker product");
	// is graph directed?
	TStr IsDir = Env.GetIfArgPrefixStr("-isdir:", "false", "Produce directed graph (true, false)");
	
	TFlt ExpectedNodes = FitMtx.GetNodes(NIter), ExpectedEdges = FitMtx.GetEdges(NIter);
	
	TFile << "Kronecker nodes: " << ExpectedNodes << ", expected Kronecker edges: " << ExpectedEdges << endl;
		
	double Sec = 0.0;
	int AvgMaxOutDeg = 0, AvgMaxInDeg = 0, MinMaxOutDeg = 0, MaxMaxOutDeg = 0, MinMaxInDeg = 0, MaxMaxInDeg = 0;
   bool Dir = IsDir == "true" ? true : false;

	for (int i = 0; i < NKron; i++){
		ExecTime.Tick();
      PNGraph Kron = TKronMtx::GenFastKronecker(FitMtx, NIter, Dir, 0);
		Sec += ExecTime.GetSecs();
		printf("Calculating maximum degree...\n");
		int MaxOutDeg = GetMaxMinDeg(Kron, IsDir, "false", "true"), MaxInDeg = GetMaxMinDeg(Kron, IsDir, "true", "true");
		CompareDeg(i, MaxOutDeg, MinMaxOutDeg, MaxMaxOutDeg, AvgMaxOutDeg);
		CompareDeg(i, MaxInDeg, MinMaxInDeg, MaxMaxInDeg, AvgMaxInDeg);

		//printf("Nodes count: %d, nodes with non-zero degree %d, edges count %d\n max deg = %d\n", kron->GetNodes(), TSnap::CntNonZNodes(kron), kron->GetEdges(), MaxDeg);
		if (i == NKron - 1){
			//TFile << "Clustering coefficient: " << TSnap::GetClustCf(kron) << endl;
			//TSnap::PlotClustCf(kron,"kronSingle");
			//TSnap::PlotHops(kron, "kronSingle");
			
			TFile << "Maximum output degree in kron graph: " << "from " << MinMaxOutDeg << " to " << MaxMaxOutDeg << " (average: " << (double)AvgMaxOutDeg / (double)NKron << ")" << endl;
			TFile << "Maximum input degree in kron graph: " << "from " << MinMaxInDeg << " to " << MaxMaxInDeg << " (average: " << (double)AvgMaxInDeg / (double)NKron << ")" << endl;
		}
		AddDegreesStat(KronDegAvgIn, Kron, true);
		AddDegreesStat(KronDegAvgOut, Kron, false);
	}
	Sec /= NKron;

	
    GetAvgDegreeStat(KronDegAvgIn, NKron);
    GetAvgDegreeStat(KronDegAvgOut, NKron);
	
	KronDegAvgIn.Sort();
	KronDegAvgOut.Sort();
	TFile << "Average time of generation of Kronecker product: " <<  Sec << endl;
}

void GetGraphs(const vector <TStr>& Parameters, const TStr& ModelGen, const TStr&ModelPlt)
{
    PNGraph G;
    size_t PSize = Parameters.size();
    if (GRAPHGEN >= PSize || MTXGEN >= PSize || KRONGEN >= PSize || KRONFIT >= PSize)
        Error("GetGraphs", "Wrong index in array of parameters");

    GetModel(Parameters[GRAPHGEN], G);

    if (G->GetNodes() == 0)
        Error("GetGraphs", "Empty graph");

    TFltPrV MDegIn, MDegOut;
    TSnap::GetInDegCnt(G, MDegIn);
    TSnap::GetOutDegCnt(G, MDegOut);

    PlotDegrees(Parameters, MDegIn, MDegOut, "model");
    TFile << "Model nodes: " << G->GetNodes() << ", model edges: " << G->GetEdges() << endl;
    TFile << "Maximum output degree in model graph: " << MDegOut[MDegOut.Len()-1].GetVal1() << endl;
    TFile << "Maximum input degree in model graph: " << MDegIn[MDegIn.Len()-1].GetVal1() << endl;


    if (ModelGen == "model+kron"){
        // generate (or read) Kronecker initiator matrix
        TKronMtx FitMtxM;
        if (!GetMtx(Parameters[MTXGEN], FitMtxM))
            GenNewMtx(G, Parameters[KRONFIT], FitMtxM);
        PrintMtx(FitMtxM, TFile);
        TFile << "Scaling for the number of edges... " << endl;
        FitMtxM.SetForEdges(G->GetNodes(), G->GetEdges());

        int ModelNodes = G->GetNodes(), ModelEdges = G->GetEdges();

        Env = TEnv(Parameters[KRONGEN], TNotify::NullNotify);
        TStr IsDir = Env.GetIfArgPrefixStr("-isdir:", "false", "Produce directed graph (true, false)");
        const TInt NIter = Env.GetIfArgPrefixInt("-i:", 1, "Number of iterations of Kronecker product");
        
        if (pow(FitMtxM.GetDim(), static_cast<double>(NIter)) != ModelNodes)
            Error("GetGraphs", "Inconsistent value of -i: parameter, KronNodes != ModelNodes");
              

        // in and out average degrees of Kronecker graphs
        TFltPrV KronDegAvgIn, KronDegAvgOut;

       
        GenKron(Parameters[KRONGEN], FitMtxM, KronDegAvgIn, KronDegAvgOut);

        PlotDegrees(Parameters, KronDegAvgIn, KronDegAvgOut, "kron");

    }
}



// generates Kronecker model using configuration model of small model network
// and compare it to big network
void KroneckerBySample(vector<TStr> CommandLineArgs){
    Try
        Env = TEnv(CommandLineArgs[KRONTEST], TNotify::NullNotify);
    // generation of big model and its Kronecker product is required
    const TStr Gen = Env.GetIfArgPrefixStr("-gen:", "model+kron", "Generation of sample or/and its Kronecker product (model, kron, model+kron)");
    // plot type
    const TStr Plt = Env.GetIfArgPrefixStr("-plt:", "all", "Type of plots (cum, noncum, all)");
    // time estimates file name
    const TStr StatFile = Env.GetIfArgPrefixStr("-ot:", "stat.tab", "Name of output file with statistics");

    TFile = OpenFile(StatFile.CStr());
    CheckParams(Gen, Plt);
    
    PyInit("PySettings.txt");
      

    if (Gen != "none")
    {
        vector <TStr> Parameters;
        GetParameters(CommandLineArgs, Parameters);
        // name is empty
        Parameters.push_back("");
        GetGraphs(Parameters, Gen, Plt);
    }
    

    Py_Finalize();

    Catch
}