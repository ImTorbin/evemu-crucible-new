# - Build config of UTF8-CPP
# Defines several variables needed for automatic
# download and build of source archive.
#

# Search for the package
FIND_PACKAGE( "UTF8CPP" )
SET( PKG_FOUND "${UTF8CPP_FOUND}" )

IF( PKG_FOUND )
  RETURN()
ENDIF( PKG_FOUND )

# Cache entries (GitHub: SourceForge mirror often returns HTTP errors from automated downloads.)
SET( UTF8CPP_URL
  "https://github.com/nemtrif/utfcpp/archive/refs/tags/v2.3.4.tar.gz"
  CACHE STRING "URL of the UTF8-CPP source archive" )
MARK_AS_ADVANCED( UTF8CPP_URL )

# Setup the needed variables
SET( PKG_URL "${UTF8CPP_URL}" )
SET( PKG_MD5 "106f0763397de3c53d7ee289dd5f15e5" )
SET( PKG_EXTRACT_PREFIX "" )
SET( PKG_ARCHIVE_PREFIX "/utfcpp-2.3.4" )

SET( PKG_CMAKELISTS "
SET( UTF8CPP_INCLUDE_DIRS
  \"\${CMAKE_CURRENT_SOURCE_DIR}/source\"
  PARENT_SCOPE )
" )
