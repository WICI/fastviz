# path to include directory of Boost library (http://www.boost.org/)
# Boost version has to be 1.44 or higher
# e.g. BOOST_SRC=/usr/local/include/
export BOOST_SRC=/usr/local/include/

# path to directory with binaries of Boost library
# e.g. BOOST_BIN=/usr/local/lib
export BOOST_BIN=/usr/local/lib

# path to include directory of JsonCpp (http://jsoncpp.sourceforge.net/)
# JsonCpp version has to 0.5.0 or higher
# e.g. JSON_SRC=~/jsoncpp-src-0.5.0/include
export JSON_SRC=fill_here

# path to the compiled JsonCpp library
# if no prefix was given during the installation then
# the library is in ~/jsoncpp-src-0.5.0/libs/PLACEHOLDER/libjson_PLACEHOLDER_libmt.{a,so}
# where PLACEHOLDER depends on the compiler used and will vary
# e.g. JSON_LIBMT=~/jsoncpp-src-0.5.0/libs/linux-gcc-4.2.4/libjson_linux-gcc-4.2.4_libmt
export JSON_LIBMT=fill_here
