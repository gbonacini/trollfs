#ifndef  TROLL__FS
#define  TROLL__FS

#define FUSE_USE_VERSION 26
#define _XOPEN_SOURCE    700

#ifndef USE_FDS
#define USE_FDS 15
#endif

#include <config.h>

#include <fuse.h>
#include <errno.h>
#include <ftw.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <checkMagic.hpp> 

#include <exception>
#include <stdexcept>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <tuple>
#include <utility>
#include <iostream>
#include <fstream>
#include <limits>

namespace trollfs{

    enum Filetype { ISFILE,   ISLINK,   ISDIR    };
    enum VarDescr { FILECONT, FILETYPE, FILESTAT };

    typedef struct stat                            Stat;
    typedef struct fuse_operations                 Fuse;
    typedef std::string                            Filename;
    typedef std::vector<unsigned char>             Filedata;
    typedef std::string                            Path;
    typedef std::tuple<Filedata*, Filetype, Stat>  FileAttr;
    typedef std::map<  Filename, FileAttr>         Directory;
    typedef std::map<  Path,     Directory>        Filesystem;
    typedef struct fuse_file_info                  FileInfo;
    typedef struct FTW                             Ftw;
    typedef std::string                            Extension;
    typedef std::vector<unsigned char>             DummyFile;
    typedef std::map<Extension, DummyFile>         DummyFileMap;

    void genericExcPtrHdlr(std::exception_ptr exptr);

    class Trollfs{
         public:
                        Fuse fuse;

                          
             static Trollfs& getInstance(     const std::string& dir, 
                                              const std::string& mdb,
                                              const std::string& repoPath)           noexcept(true);
           
                             ~Trollfs(        void);
             void            setDebug(        bool on)                               noexcept(true);  
             int             initFileSystem(  void)                                  noexcept(true);
             int             initPayloads(    void)                                  noexcept(true);
             void            printFileSystem( void)                          const   noexcept(true);
             void            printPayloads(   void)                          const   noexcept(true);
             bool            loadDummyFile(   unsigned char*      buff,
                                              const std::string   ext)       const   noexcept(true);
           
             static bool extractIds(          const char*         buff,
                                              std::string&        path, 
                                              std::string&        file)               noexcept(true);
             static int  readdirCb(           const char          *path, 
                                              void                *buf, 
                                              fuse_fill_dir_t     filler, 
                                              off_t               offset, 
                                              FileInfo            *fi)                noexcept(true);
             static int  openCb(              const char          *path, 
                                              FileInfo            *fi)                noexcept(true);
             static int  readCb(              const char          *path, 
                                              char                *buf,
                                              size_t              size, 
                                              off_t               offset,
                                              FileInfo            *fi)                noexcept(true); 
             static int  getattrCb(           const char          *path,
                                              Stat                *stbuf)             noexcept(true);
               
         private:
                         Trollfs(         const std::string& dir, 
                                          const std::string& mdb,
                                          const std::string& repoPath);
                         Trollfs(         Trollfs const&);
                    void operator=(       Trollfs const&);

                    static checkMagic::MagicFile&  mf;
                    static Filesystem              fs;
                    static bool                    trace;
                    static std::fstream            debugStream;
                           std::string             reposPath,
                                                   dirPath;
                    static std::string             magicDb;
                    static size_t                  mountPoint;
                                  
                    static DummyFileMap            dummyFileLookup;
                
                    static int  loadDirTree(       const  char        *filepath, 
                                                   const  struct stat *info, 
                                                   const  int         typeflag, 
                                                   struct FTW         *pathinfo)        noexcept(true);
                    static int  loadRepoDir(       const  char        *filepath, 
                                                   const  struct stat *info, 
                                                   const  int         typeflag, 
                                                   struct FTW         *pathinfo)        noexcept(true);
                           int  traversalDirTree(  const  std::string path)    const    noexcept(true);
                           int  traversalRepoDir(  const  std::string path)    const    noexcept(true);
    };

} // namespace trollfs

#endif
