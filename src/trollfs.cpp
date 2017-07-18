#include <trollfs.hpp>

using std::string;
using std::fstream;
using std::get;
using std::endl;
using std::vector;
using std::map;
using std::move;
using std::copy;
using std::exception;
using std::exception_ptr;
using std::current_exception;
using std::numeric_limits;
using std::rethrow_exception;

using checkMagic::MagicFile;

namespace trollfs{
    const string DEFAULT_REPO                   = "./trollrepos"; 
    const string DEFAULT_EMPTY_CONT             = "empty"; 
    const string DEFAULT_DEBUG_FILE             = ".trollfs_debug.log"; 
    const string ROOT_DIR                       = "/"; 
    const string PATH_SEPARATOR                 = "/"; 

    enum  STDCONST { STRBUFF_LEN=1024 };

    bool                      Trollfs::trace         = false;
    fstream                   Trollfs::debugStream;
    Filesystem                Trollfs::fs;
    map<Extension, DummyFile> Trollfs::dummyFileLookup;

    MagicFile&                Trollfs::mf            = MagicFile::getInstance();
    string                    Trollfs::magicDb;
    size_t                    Trollfs::mountPoint;

    void genericExcPtrHdlr(exception_ptr exptr){
           try {
              if(exptr) rethrow_exception(exptr);
           }catch(const exception& e) {
		   Trollfs::debugStream << "Caught unexpected exception \"" << e.what() << "\"\n";
           }
    }

    int Trollfs::getattrCb(const char *path, Stat *stbuf) noexcept(true){
      exception_ptr exPtr; 

      try{
         *stbuf = {};
   
         string fullPath = path, 
                fileName = path; 

         if(fullPath.compare(ROOT_DIR) == 0) {
	       stbuf->st_mode  = S_IFDIR | 0755;
               stbuf->st_nlink = 2;
	       return 0;
         }
   
         Trollfs::extractIds(path, fullPath, fileName);
   
         if(Trollfs::trace) Trollfs::debugStream << "getattrCb - Path: <" << fullPath  << ">\n";
         if(Trollfs::trace) Trollfs::debugStream << "getattrCb - File Name<" << fileName  << ">\n";
   
         auto isDir = Trollfs::fs.find(fullPath);
         if(isDir != Trollfs::fs.end()){
             if(Trollfs::trace) Trollfs::debugStream << "getattrCb - <Found dir>\n";
             auto isFile = isDir->second.find(fileName);
             if(isFile != isDir->second.end()){
		      auto &stats     = get<FILESTAT>(isFile->second);
		      stbuf->st_uid   = stats.st_uid;
		      stbuf->st_gid   = stats.st_gid;
                      stbuf->st_nlink = stats.st_nlink;
                      stbuf->st_mode  = stats.st_mode;
   
                      stbuf->st_atim  = stats.st_atim;
                      stbuf->st_mtim  = stats.st_mtim;
                      stbuf->st_ctim  = stats.st_ctim;
   
                      if(Trollfs::trace) Trollfs::debugStream << "getattrCb - <Found file>\n";
		      switch(get<FILETYPE>(isFile->second)){
	                  case ISFILE:
                              stbuf->st_size    = get<FILECONT>(isFile->second)->size();
                              if(Trollfs::trace) Trollfs::debugStream << "getattrCb - <ISFILE>\n";
                              return 0;
		          case ISDIR:
                              stbuf->st_blocks  = stats.st_blocks;
                              stbuf->st_size    = stats.st_size;
                              if(Trollfs::trace) Trollfs::debugStream << "getattrCb - <ISDIR>\n";
                              return 0;
		          default:
                              return -ENOENT;
                      }
             }
         }
      }catch(...){
          exPtr = current_exception();
	  genericExcPtrHdlr(exPtr);
      } 
      return -ENOENT;
    }

    bool Trollfs::extractIds(const char* buff, string& path, string& file) noexcept(true){
	 exception_ptr exPtr;

	 try{
	     path           = buff;
	     file           = buff;
             size_t lastSep = path.find_last_of(PATH_SEPARATOR);
             if(lastSep != string::npos && lastSep != 0){
                  path.erase(path.find_last_of(PATH_SEPARATOR), path.size()-1);
                  file.erase(0, lastSep+1);
             }else if(lastSep == 0){
	          path = ROOT_DIR;
                  file.erase(0, 1);
             }
	  }catch(...){
		exPtr = current_exception();
		genericExcPtrHdlr(exPtr);
		return true;
          }

	  return false;
    }
    
    int Trollfs::readdirCb(const char *path, void *buf, fuse_fill_dir_t filler,
        off_t offset, FileInfo *fi) noexcept(true){
    
      static_cast<void>(offset);
      static_cast<void>(fi);
      static_cast<void>(path);

      exception_ptr exPtr;

      try{
    
          filler(buf, ".", nullptr, 0);
          filler(buf, "..", nullptr, 0);
    
          string fpath = path; 
    
          if(Trollfs::trace) Trollfs::debugStream << "readdirCb - Path: <" << fpath << ">\n";
    
          auto dirCont = Trollfs::fs.find(fpath);
          if(dirCont != Trollfs::fs.end()){
              for(auto eit = dirCont->second.cbegin(); eit != dirCont->second.cend(); ++eit){
	            if(eit->first.compare(PATH_SEPARATOR) != 0){
		       	    filler(buf, eit->first.c_str(), nullptr, 0);
                            if(Trollfs::trace) Trollfs::debugStream << "readdirCb - Found: <" << eit->first  << ">\n";
		    }
              }
          }
      }catch(...){
          exPtr = current_exception();
	  genericExcPtrHdlr(exPtr);
	  return -EIO;
      }
      return 0;
    }

    int Trollfs::openCb(const char *path, FileInfo *fi) noexcept(true){
    
      static_cast<void>(path);
      static_cast<void>(fi);
    
      return 0;
    }
    
    int Trollfs::readCb(const char *path, char *buf, size_t size, off_t offset,
                             FileInfo *fi) noexcept(true){
    
      static_cast<void>(fi);

      exception_ptr exPtr; 

      try{
          string fullPath,
                 fileName; 
    
          Trollfs::extractIds(path, fullPath, fileName);
    
          if(Trollfs::trace) Trollfs::debugStream << "readCb - Path: <" << fullPath << ">\n";
          if(Trollfs::trace) Trollfs::debugStream << "readCb - File Name: <" << fileName << ">\n";
        
          auto filepath = Trollfs::fs.find(fullPath);
          if(filepath != Trollfs::fs.end() ){
	    auto file = filepath->second.find(fileName);
	    if(file != filepath->second.end() ){
                size_t len = get<FILECONT>(file->second)->size();
                if(Trollfs::trace) Trollfs::debugStream << "readCb - Size: <" << len << ">\n";
    
	        if(get<FILETYPE>(file->second) == ISFILE){
    
		    if(offset < 0)
                        return -ENOENT;
    
		    if(len > INT_MAX)
                        return -ENOENT;
        
                    if (static_cast<size_t>(offset) >= len) 
                      return 0;
                
                    if (offset + size > len) {
		      copy(get<FILECONT>(file->second)->data() + offset, get<FILECONT>(file->second)->data() + len, buf);
                      return len - offset;
                    }
            
		    copy(get<FILECONT>(file->second)->data() + offset, get<FILECONT>(file->second)->data() + offset + size, buf);

                    return size;
                  }
              }
          }
      }catch(...){
	  exPtr = current_exception(); 
	  genericExcPtrHdlr(exPtr);
      }     

      return -ENOENT;
    }

    Trollfs& Trollfs::getInstance(const string& pdir, const string& pmdb, const string& prepo) noexcept(true){
	  static Trollfs singleTroll(pdir, pmdb, prepo);

          return singleTroll;
    }
    
    Trollfs::Trollfs(const string& dir, const string& mdb, const string& repoPath) : dirPath(dir){
         fuse = {};
         fuse.getattr = Trollfs::getattrCb;
         fuse.open    = Trollfs::openCb;
         fuse.read    = Trollfs::readCb;
         fuse.readdir = Trollfs::readdirCb;
         mountPoint   = dirPath.find_last_of(PATH_SEPARATOR);
         magicDb      = mdb; 
         reposPath    = repoPath.size() == 0 ? DEFAULT_REPO : repoPath; 
    }
    
    void Trollfs::setDebug(bool on) noexcept(true){
	  if(on){
	      Trollfs::debugStream.open(DEFAULT_DEBUG_FILE, fstream::in | fstream::out | fstream::app);
	      Trollfs::trace   = true;
	  }else{
	      Trollfs::trace   = false;
              if(Trollfs::debugStream.is_open())
	           Trollfs::debugStream.close();
	  }
    }

    Trollfs::~Trollfs(void){
      if(Trollfs::debugStream.is_open())
	  Trollfs::debugStream.close();
    }
    
    int  Trollfs::initPayloads(void)  noexcept(true){
	    return traversalRepoDir(reposPath);
    }
    
    int  Trollfs::initFileSystem(void)  noexcept(true){
	    return traversalDirTree(dirPath.c_str());
    }
    
    void Trollfs::printFileSystem(void) const noexcept(true){
            Trollfs::debugStream << " ---------------  FS  ----------------------- \n" ;
	    for(auto eit = Trollfs::fs.cbegin(); eit != Trollfs::fs.cend(); ++eit){
		Trollfs::debugStream << "Path: <" << eit->first << ">\n" ;
	        for(auto fit = eit->second.cbegin(); fit != eit->second.cend(); ++fit){
		        Trollfs::debugStream << "     -----> Name: <" 
			            << fit->first << "> - Type: <" 
				    << get<FILETYPE>(fit->second) << ">\n";
	        }
	    }
            Trollfs::debugStream << " ------------- END FS ----------------------- \n" ;
    }

    void Trollfs::printPayloads(void) const noexcept(true){ 
            Trollfs::debugStream << " ------------- PAYLOADS --------------------- \n" ;
	    for(auto eit = Trollfs::dummyFileLookup.cbegin(); eit != Trollfs::dummyFileLookup.cend(); ++eit){
		Trollfs::debugStream << "Key: <" << eit->first << "> - Payload: <" ;
	         for(auto eeit = eit->second.cbegin(); eeit != eit->second.cend(); ++eeit)
		     Trollfs::debugStream << char(((*eeit >= 32 && *eeit <= 126) ? *eeit : 46 ));
	         Trollfs::debugStream << ">\n";
	    }
            Trollfs::debugStream << " ----------- END PAYLOADS ------------------- \n" ;
    }

    int Trollfs::loadDirTree(const char *filepath, const Stat *info, 
		             const int typeflag, Ftw *pathinfo) noexcept(true){

       static_cast<void>(pathinfo);	    
       exception_ptr exPtr;

       try{

           char    *target    = nullptr;
           string  absPath    = filepath + Trollfs::mountPoint,
	           fileName   = absPath,
	           mime       = "",
	           fileExt    = "",
	           trueExt    = "",
	           ext        = "";
           ssize_t len;
           size_t  maxlen     = STRBUFF_LEN,
                   lastSep    = absPath.find_last_of(PATH_SEPARATOR),
	           firstSep   = absPath.find_first_of(PATH_SEPARATOR),
	           extSep     = 0;
           Stat    st;
           bool    again      = true;
    
           switch(typeflag){ 
	      case FTW_SL:
                   while (again) {
                       target = new char[maxlen]();
                       if (target == nullptr)
                           return ENOMEM;
                       
                       len = readlink(filepath, target, maxlen-1);
                       if (len == (ssize_t)-1) {
                           const int saved_errno = errno;
                           delete[] target;
		           target   = nullptr;
                           return saved_errno;
                       }
                       if (len >= (ssize_t)maxlen-1) {
                            delete[] target;
		            target   = nullptr;
                            maxlen += STRBUFF_LEN;
                            continue;
                       }
                
                       again   = false;
                 }
           
	         Trollfs::debugStream << "loadDirTree - Filepath: " << filepath << " - Target: " << target << endl;
                 delete[] target;
	     break;
	     case FTW_SLN:
	         Trollfs::debugStream << "loadDirTree - Dangling symlink: " << filepath << endl;
	     break;
	     case FTW_F:
	         if(lastSep != string::npos && lastSep != 0 && lastSep != firstSep){
		      absPath.erase(lastSep, absPath.size()-1);
		      absPath.erase(0, firstSep);
	         }else{
		         absPath  = ROOT_DIR;
	         }
	         fileName.erase(0, lastSep+1);
	         fileExt = fileName;
	         extSep = fileExt.find_last_of(".");
	         if(extSep != string::npos){
                      fileExt.erase(0, extSep+1);
	         }

	         if(magicDb.size() != 0)
                     mf.setMagicDb(magicDb);
    
	         if(mf.magicIdentifier(filepath, mime)){
                     trueExt = mf.magicMimeLookup(mime);
	         }
    
	         if(trueExt.size() > 0)
		      ext = trueExt;
	         else if(fileExt.size() > 0)
		      ext = fileExt;
	         else
		      ext = MagicFile::getDefaultExt();
    
	         Trollfs::debugStream << "loadDirTree - FilePath: " << filepath << " - FileExt: " << fileExt 
		                      << " - Mime: " << mime <<" - TrueExt: " << trueExt << " - Assigned: " 
				      << ext << endl;
    
	         Trollfs::fs[absPath][fileName] = make_tuple(&dummyFileLookup[ext], ISFILE, st);
	         get<FILESTAT>(Trollfs::fs[absPath][fileName]) = *info;
	         
	         Trollfs::debugStream << "loadDirTree - File:  AbsPath: " << absPath << " Name: " 
		                      << fileName << " Payload Type : " << ext << " Type: FILE " 
				      << ISFILE << " BUFF: " << filepath << endl;
	     break;
	     case FTW_D:
	     case FTW_DP:
	         if(lastSep != string::npos && (lastSep != firstSep)){
		         absPath.erase(0, lastSep+1);
	         }else{
		         absPath  = ROOT_DIR;
		         if(lastSep != string::npos )
	                         fileName.erase(0, lastSep+1);
		         else
			         fileName = "";
	         }
    
	         if(fileName.size() > 0 ){
		         Trollfs::fs[absPath][fileName] = make_tuple(&dummyFileLookup[DEFAULT_EMPTY_CONT], ISDIR, st);
	                 get<FILESTAT>(Trollfs::fs[absPath][fileName]) = *info;
	         }else{
		         Trollfs::fs[absPath];
	         }
	         Trollfs::debugStream << "loadDirTree - Directory - AbsPath: " << absPath << " Name: " << fileName << " Cont : " 
		      << "" << " Type: DIR " << ISDIR << " LASTSEP: " << lastSep << " BUFF: " << filepath << endl;
	     break;
	     case FTW_DNR:
	         Trollfs::debugStream << "loadDirTree - Unreadable: " << filepath << endl;
	     break;
	     default:
	         Trollfs::debugStream << "loadDirTree - Unknown: " << filepath << endl;
           }
       }catch(...){
          exPtr = current_exception();
	  genericExcPtrHdlr(exPtr);
          return EIO;
       }
        
       return 0;
    }

    int Trollfs::traversalDirTree(const string path) const noexcept(true){
	    int res = nftw(path.c_str(), loadDirTree, USE_FDS, FTW_PHYS);
	    if(res >= 0) return errno;
            return 0;
    }

    int Trollfs::loadRepoDir(const char *filepath, const Stat *info, 
		             const int typeflag, Ftw *pathinfo) noexcept(true){

       static_cast<void>(pathinfo);	    
       static_cast<void>(info);	    
       exception_ptr exPtr;

       try{

           string                absPath    = filepath,
	                         fileName   = absPath,
	                         ext        = "";
           size_t                lastSep    = fileName.find_last_of(PATH_SEPARATOR),
	                         extSep     = 0,
	                         parRead    = 0;
           int                   fd;
           vector<unsigned char> buffer;
    
           switch(typeflag){ 
	     case FTW_F:
	         fileName.erase(0, lastSep+1);
	         ext = fileName;
	         extSep = ext.find_last_of(".");
	         ext.erase(0, extSep+1);
    
                 fd = open(absPath.c_str(), O_RDONLY);
	         if(fd != -1){
                      buffer.resize(info->st_size);          
		      if(info->st_size < 0 || static_cast<size_t>(info->st_size) > numeric_limits<size_t>::max())
			      throw(string("Invalid nfo->st_size."));
		      while(parRead != static_cast<size_t>(info->st_size))
                          parRead += read(fd, buffer.data() + parRead, info->st_size-parRead);
    
	              Trollfs::dummyFileLookup[ext] = move(buffer);
		      
	         }
	     
	         Trollfs::debugStream << "loadRepoDir - Loading extension: " << ext << " Size: " << parRead << endl;
	     break;
	     case FTW_SL:
	     case FTW_SLN:
	         Trollfs::debugStream << "loadRepoDir - Found link in repo, ignored: " << filepath << endl;
	     break;
	     case FTW_D:
	     case FTW_DP:
	         Trollfs::debugStream << "loadRepoDir - Found subdirectory in repo: ignored." << filepath << endl;
	     break;
	     case FTW_DNR:
	         Trollfs::debugStream << "loadRepoDir - Unreadable: " << filepath << endl;
	     break;
	     default:
	         Trollfs::debugStream << "loadRepoDir - Unknown: " << filepath << endl;
           }
       }catch(...){
          exPtr = current_exception();
	  genericExcPtrHdlr(exPtr);
          return EIO;
       }
    
       return 0;
    }

    int Trollfs::traversalRepoDir(const string path) const noexcept(true){
	    int res = nftw(path.c_str(), loadRepoDir, USE_FDS, FTW_PHYS);
	    if(res >= 0) return errno;
            return 0;
    }

    
} // namespace trollfs
