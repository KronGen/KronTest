#include "stdafx.h"
#include "GenPy.h"


inline TStrV createTStrV(TStr s)
{
	TStrV vec;
	s.SplitOnAllCh(' ', vec);
	return vec;
}

class cmp_str
{
public:
   bool operator()(TStr const&  a, TStr const&  b) const
   {
	int res = strcmp(a.CStr(), b.CStr());
	if (res == -1 || res == 0)
		return true;
	else return false;
   }
};
// [funcName] -> argPrefixes, argTypes, required arguments count, default values
map<TStr, tuple<TStrV,TStrV, int, TStrV>,cmp_str> funcInfo;

void AddFuncInfo()
{
	funcInfo.clear();
	funcInfo["fast_gnp_random_graph"] = make_tuple(createTStrV("n p seed directed"), createTStrV("int double int int"), 2, createTStrV("1024 0.003 0 0"));
	funcInfo["random_powerlaw_tree"] = make_tuple(createTStrV("n gamma seed tries"), createTStrV("int double int int"), 2, createTStrV("1024 2 0 10000"));
	funcInfo["barabasi_albert_graph"] = make_tuple(createTStrV("n m seed"), createTStrV("int int int"), 2, createTStrV("1024 3 0"));
	funcInfo["gnm_random_graph"] = make_tuple(createTStrV("n m seed directed"), createTStrV("int int int int"), 2, createTStrV("1024 2048 0 0"));
	funcInfo["newman_watts_strogatz_graph"] = make_tuple(createTStrV("n k p seed"), createTStrV("int int double int"), 3, createTStrV("1024 2 0.3 0"));
	funcInfo["powerlaw_cluster_graph"] = make_tuple(createTStrV("n m p seed"), createTStrV("int int double int"), 3, createTStrV("1024 3 0.2 0"));
	funcInfo["random_lobster"] = make_tuple(createTStrV("n p1 p2 seed"), createTStrV("int double double int"), 3, createTStrV("1024 0.1 0.1 0"));
	funcInfo["path_graph"] = make_tuple(createTStrV("n"), createTStrV("int"), 1, createTStrV("1024"));
	//funcInfo["grid_2d_graph"] = make_tuple(createTStrV("n m"), createTStrV("int int"), 2, createTStrV("1024 1024"));
	//funcInfo["grid_graph"] = make_tuple(createTStrV("n"), createTStrV("int"), 1, createTStrV("1024"));
}



void AddPath(const char * path)
{
	PyObject * pName = PyUnicode_FromString(path), *syspath;
	// reference to Python search path
	syspath = PySys_GetObject("path");
	// add path to syspath
	if (PyList_Insert(syspath, 0, pName))
		printf("Error inserting extra path into sys.path list\n");
	// reset sys.path object
	if (PySys_SetObject("path", syspath))
		printf("Error setting sys.path object\n");
}

void AddPath(const std::string& path)
{
	AddPath(path.c_str());
}

void PyInit(const TStr& PySettings)
{
	Try
	ifstream f(PySettings.CStr());
	if (f.is_open())
	{
		std::string s;
		std::getline(f, s);
		Py_Initialize(); // инициализация интерпретатора  */
		AddPath(s);
		AddPath(s+"\\\\networkx\\\\generators");
		AddPath(s+"\\\\networkx\\\\readwrite");
		AddPath(s+"\\\\networkx\\\\classes");
	}
	return;		
	IAssert(1);
	Catch
	
}

int ParseArgument(const TStr& arg, const TStr& argType, PyObject** argPy)
{
	if (argType == "int")
	{
		*argPy = PyLong_FromLong(arg.GetInt());
	}
	else if (argType == "double")
	{
		*argPy = PyFloat_FromDouble(arg.GetFlt());
	}
	else if (argType == "string")
	{
		*argPy = PyUnicode_FromString(arg.CStr());
	}
	if (!*argPy) {
        fprintf(stderr, "Cannot convert argument\n");
        return 0;
    }
	return 1;
}

int CallPyFunction(const char *moduleName, const char *funcName, const TStrV& args, const TStrV& argTypes,  PyObject** res, PyObject*** pyObjects = nullptr)
{
	PyObject *pName, *pModule, *pFunc = nullptr, *pArgs;
	bool err = false;
	// get PyObject representation of moduleName
	pName = PyUnicode_FromString(moduleName);
	TExeTm execTime;
	// import module
	pModule = PyImport_Import(pName);
	cout << "Time of importing module " << moduleName << ": " << execTime.GetTmStr() << endl;
	// we don't need pName anymore
	Py_DECREF(pName);
	// if module was loaded 
	if (pModule != nullptr) {
		// get pointer to function
		pFunc = PyObject_GetAttrString(pModule, funcName);
		// check function for existence
		if (pFunc && PyCallable_Check(pFunc)) {
			// a number of arguments
			int argc = argTypes.Len();
			// tuple of arguments
			pArgs = PyTuple_New(argc);
			// index of current PyObject in array of PyObjects
			int argPyObjIndex = 0, argStrIndex = 0;
			// parsing arguments
			for (size_t i = 0; i < argc; i++)
			{
				PyObject **arg = new PyObject*[1];
				//printf("argtypes[%d] = %s\n", i, argTypes[i].CStr());
				if (argTypes[i] != "pyobject"){
					if (!ParseArgument(args[argStrIndex++], argTypes[i], arg))
						err = true;
				}
				else
				{
					// non-safe
					*arg = pyObjects[argPyObjIndex++][0];
				}
				PyTuple_SetItem(pArgs, i, *arg);
				Py_DECREF(arg);
			}
			//TExeTm execTime;
			*res = PyObject_CallObject(pFunc, pArgs);
			//cout << "Time of execution of function " << funcName << ": " << execTime.GetTmStr() << endl; 
			if (PyErr_Occurred()){
                PyErr_Print();
				err = true;
			}
			if (res == nullptr)
			{
				PyErr_Print();
                fprintf(stderr,"Call failed\n");
                err = true;
			}
			//Py_DECREF(pArgs);
		}
		else {
            if (PyErr_Occurred())
                PyErr_Print();
            fprintf(stderr, "Cannot find function \"%s\" in module \"%s\"\n", funcName, moduleName);
			err = true;
        }
	}
	else {
		PyErr_Print();
		fprintf(stderr, "Failed to load \"%s\"\n", moduleName);
		err = true;
    }
	if (err == true) return 0;
	//Py_DECREF(pArgs);
	Py_DECREF(pFunc);
	Py_DECREF(pModule);
	
	return 1;
}


void Get(PyObject **G, const char* funcName, PyObject***list)
{
	PyObject *** pyObjects = new PyObject**[1];
	pyObjects[0] = G;
	*list = new PyObject*[1];
	char * moduleName = "function";
	int argc = 1;
	TStrV args(0); TStrV argTypes(argc);
	argTypes[0] = "pyobject";
	CallPyFunction(moduleName, funcName, args, argTypes, *list, pyObjects);
	Py_DECREF(pyObjects);	
}

void GetNodes(PyObject **G, PyObject***list)
{
	return Get(G, "nodes", list);
}

void GetEdges(PyObject **G, PyObject***list)
{
	return Get(G, "edges", list);
}

int ParseArgs(const char* funcname, const TStr& parameters, TStrV& args, TStrV& argTypes)
{
	Env = TEnv(parameters, TNotify::NullNotify);
	map<TStr,tuple<TStrV,TStrV, int, TStrV>,cmp_str>::const_iterator i;
	for (i = funcInfo.begin(); i!= funcInfo.end(); ++i)
	{
		if (i->first == funcname)
		{
			size_t argCount = get<1>(i->second).Len();
			if (get<1>(i->second).Len() != argCount)
			{
				printf("ParseArgs error\n");
				return 0;
			}
			int reqArgs = get<2>(i->second);
			int argRead = 0;
			for (size_t j = 0; j < argCount; j++)
			{
				TStr arg = Env.GetIfArgPrefixStr("-" + get<0>(i->second)[j] + ":", "", get<0>(i->second)[j]);
				TStr argType = get<1>(i->second)[j];
				if (arg != ""){
					args.Add(arg);
					argRead++;
				}
				else
				{
					arg = get<3>(i->second)[j];
					args.Add(arg);
				}
				argTypes.Add(argType);
				//printf("%d %s %s\n", j, arg.CStr(), argType.CStr());
			}
			if (argRead < reqArgs)
				return 0;
		}
	}
	return 1;
}

int GenPy(PUNGraph &res, ofstream& TFile, const TStr& parameters)
{
	Env = TEnv(parameters, TNotify::StdNotify);
	TStr mN = Env.GetIfArgPrefixStr("-module:", "random_graphs", "Module name");
	TStr fN = Env.GetIfArgPrefixStr("-func:", "fast_gnp_random_graph", "Function name");
	
	PyObject **G = new PyObject*[1];
		
	char *moduleName = mN.CStr();
	char *funcName = fN.CStr();
	AddFuncInfo();
	TStrV args, argTypes;
	if (!ParseArgs(funcName, parameters, args, argTypes))
	{
		printf("Fail to parse arguments for NetworkX generation...\n");
		return 0;
	};
	TExeTm execTime;
	if (!CallPyFunction(moduleName, funcName, args, argTypes, G))
	{
		cout << "CallPyFunction() raised error. Execution terminated.\n";
		system("pause");
		exit(1);
	};
	
	TFile << "Time of generation of graph by NetworkX: " << execTime.GetTmStr() << endl; 

	execTime.Tick();
	PyObject*** nodes = new PyObject**[1];
	GetNodes(G, nodes);
	int nodesCount = PyList_Size(*(nodes[0]));
	//printf("nodesCount = %d, ", nodesCount);
	res = PUNGraph::TObj::New();
    res->Reserve(nodesCount, nodesCount*nodesCount);
	for (size_t i = 0; i < nodesCount; i++)
		res->AddNode(i);
	Py_DECREF(nodes);

	PyObject*** edges = new PyObject**[1];
	GetEdges(G, edges);
	int edgesCount = PyList_Size(*(edges[0]));
	//printf("edgesCount = %d\n", edgesCount);
	for (size_t i = 0; i < edgesCount; i++)
	{
		PyObject* item = PySequence_Fast_GET_ITEM(*(edges[0]), i);
		int v1, v2;
		PyObject* node = PySequence_Fast_GET_ITEM(item,0);
		v1 = PyLong_AsLong(node);
		node = PySequence_Fast_GET_ITEM(item,1);
		v2 = PyLong_AsLong(node);
		res->AddEdge(v1,v2);
	}
	TFile << "Time of copying of graph from NetworkX representation: " << execTime.GetTmStr() << endl; 
	Py_DECREF(G);
	Py_DECREF(edges);
	//Py_Finalize(); // очищение памяти, отданной интерпретатору
	
	return 0;
}