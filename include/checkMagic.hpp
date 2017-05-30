#ifndef CHECK__MAGIC 
#define CHECK__MAGIC

#include <magic.h>

#include <map>
#include <string>
#include <iostream>

namespace checkMagic{

    class MagicFile{
        public:    
           static       MagicFile&   getInstance(); 
	   static const std::string& getDefaultExt(void)                        noexcept(true);

           void setMagicDb(      std::string mdb)                               noexcept(true);
           bool magicIdentifier( const char*     file)                 const    noexcept(true);
           bool magicIdentifier( const char*     file,
                                 std::string&    dest)                 const    noexcept(true);
           const std::string& magicExtLookup(const std::string& ext)   const    noexcept(true);
           const std::string& magicMimeLookup(const std::string& mime) const    noexcept(true);
        private:
           std::string                        mimeDb;
           std::map<std::string, std::string> extLookup; 
           std::map<std::string, std::string> typeLookup; 

	        MagicFile();
                MagicFile(MagicFile const&);
           void operator=(MagicFile const&);
           void initDictionary(void)                                           noexcept(true); 
    };

} // namespace checkMagic

#endif
