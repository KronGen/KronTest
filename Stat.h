int GetMaxMinDeg(const PNGraph& G, const TStr& IsDir, const TStr& IsIn, const TStr& IsMax);
void CompareDeg(const int i, const int MaxDeg, int& MinMaxDeg, int& MaxMaxDeg, int& AvgMaxDeg);
void AddDegreesStat(TFltPrV& deg, const PNGraph& G, bool isIn);
void GetAvgDegreeStat (TFltPrV& deg, const TInt& NKron);