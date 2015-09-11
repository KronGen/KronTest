#include "stdafx.h"
#include "Stat.h"

int GetMaxMinDeg(const PNGraph& G, const TStr& IsDir, const TStr& IsIn, const TStr& IsMax){
    TIntPrV DegCnt;
    if (IsDir == "false"){
        PUNGraph U = TSnap::ConvertGraph<PUNGraph>(G);
        if (IsIn == "true")
            TSnap::GetInDegCnt(U, DegCnt);
        else 
            TSnap::GetOutDegCnt(U, DegCnt);
    }
    else{
        if (IsIn == "true")
            TSnap::GetInDegCnt(G, DegCnt);
        else
            TSnap::GetOutDegCnt(G, DegCnt);
    }
    // sort in descending order
    DegCnt.Sort(false);
    if (IsMax == "true") return DegCnt[0].Val1;
    else return DegCnt[DegCnt.Len()-1].Val1;
}

void CompareDeg(const int i, const int MaxDeg, int& MinMaxDeg, int& MaxMaxDeg, int& AvgMaxDeg){
    if (i == 0) { 
        MinMaxDeg = MaxDeg;
    }
    else if (MaxDeg < MinMaxDeg){
        MinMaxDeg = MaxDeg;
    }
    if (MaxDeg > MaxMaxDeg) {
        MaxMaxDeg = MaxDeg;
    }
    AvgMaxDeg += MaxDeg;
}

// get degrees from current and add it to degrees
void AddDegreeStat(const TFltPrV& current, TFltPrV& degrees){
    for (int j = 0; j < current.Len(); j++){
        const TFltPr& elem = current[j];
        const double& deg = elem.Val1.Val, &nodesCount = elem.Val2.Val;
        bool wasFound = false;
        // silly search
        for (int k = 0; k < degrees.Len(); k++){
            if (degrees[k].Val1.Val == deg){
                degrees[k].Val2.Val += nodesCount;
                wasFound = true; break;
            }
        }
        if (!wasFound){
            TFlt d(deg), n(nodesCount);
            TFltPr val(d,n);
            degrees.Add(val);
         }
    }
}

// get graph and add in and out degrees to cumulative vectors
void AddDegreesStat(TFltPrV& deg, const PNGraph& G, bool isIn){
    TFltPrV current;
    if (isIn)
        TSnap::GetInDegCnt(G, current);
    else 
        TSnap::GetOutDegCnt(G, current);
    AddDegreeStat(current, deg);
}

void GetAvgDegreeStat (TFltPrV& deg, const TInt& NKron){
    for (int i = 0; i < deg.Len(); i++)
        deg[i].Val2.Val /= NKron;
}
