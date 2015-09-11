// KronTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "InOut.h"
#include "KronGen.h"

int _tmain(int argc, _TCHAR* argv[])
{
    Env = TEnv(argc, argv, TNotify::StdNotify);
    Env.PrepArgs(TStr::Fmt("Two stage generation. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
    const TStr SettingsFNm = Env.GetIfArgPrefixStr("-s:", "parameters.txt", "Settings filename");
    std::vector<TStr> commandLineArgs;
    // read command line arguments for all generators
    ReadParameters(SettingsFNm, commandLineArgs);
    KroneckerBySample(commandLineArgs);
       
    return 0;
}

