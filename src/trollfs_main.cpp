#include <trollfs.hpp>

using namespace std;
using namespace trollfs;

void paramError(const char* progname, const char* err=nullptr){
    if(err != nullptr) cerr << err << endl << endl;

    cerr << "trollfs - A high tech prank :-). GBonacini - (C) 2017   " << endl;
    cerr << "Version: " << VERSION << endl;
    cerr << "Syntax: " << endl;
    cerr << "       " << progname << " [-d mountpoint] [-f payload_repository] [-m magicdb] [-D] | [-h]" << endl;
    cerr << "       " << "-d specifies the mountpoint." << endl;
    cerr << "       " << "-f specifies the directory with the fake files." << endl;
    cerr << "       " << "   ( the default is: ./trollrepos)" << endl;
    cerr << "       " << "-m specifies a different 'magic db' " << endl;
    cerr << "       " << "   ( the default is: /usr/share/file/magic.mgc)" << endl;
    cerr << "       " << "-D sets the debug mode." << endl;
    cerr << "       " << "-h print this help message." << endl;

    exit(1);
}
    
int main(int argc, char *argv[]) {
    int fuseErr   = 0;
    exception_ptr exPtr;

    try{
        string         directory = "",
                       magicdb   = "",
                       filerepo  = "";
        const char     flags[]   = "d:m:f:hD";
        
                       opterr    = 0;
        int            c         = 0;
        bool           debug     = false;
    
        vector<string> parVals;
    
        parVals.push_back(argv[0]); 
        parVals.push_back("-o"); 
        parVals.push_back("nonempty"); 
    
        while ((c = getopt(argc, argv, flags)) != -1){
                switch (c){
                    case 'd':
                             directory = optarg;
                    break;
                    case 'm':
                             magicdb   = optarg;
                    break;
                    case 'f':
                             filerepo  = optarg;
                    break;
                    case 'D':
		             debug     = true;
                    break;
                    case 'h':
                             paramError(argv[0]);
                }
        }

        if(directory.size() == 0 || optind != argc)
               paramError(argv[0], "Invalid parameter(s).");
    
        parVals.push_back(directory); 
    
        int    paramsc  = parVals.size();
        char** paramsv  = new char*[paramsc+1]();
    
        for(size_t i=0; i<parVals.size(); i++){
            paramsv[i] = strdup(parVals[i].c_str());
         cerr << paramsv[i] <<  endl;
        }
    
        Trollfs& trfs  = Trollfs::getInstance(directory, magicdb, filerepo);

        trfs.setDebug(debug);
        if(trfs.initPayloads() != 0){
	   cerr << "Init Error: Payload." << endl;
           throw(string("Init Error: Payload."));	   
	};
        trfs.printPayloads();
        if(trfs.initFileSystem() != 0){
	   cerr << "Init Error: File System." << endl;
           throw(string("Init Error: File System."));	   
	};
        trfs.printFileSystem();

        fuseErr =  fuse_main(paramsc, paramsv, &(trfs.fuse), nullptr);
        delete[] paramsv;
    }catch(...){
	exPtr = current_exception();
	genericExcPtrHdlr(exPtr);
	fuseErr = EXIT_FAILURE;
    }

    return fuseErr;
}
