OPAQ consists of a number of components which have been implemented
in different (eclipse) C++ projects and which all have their own
SVN location.

OPAQ Plugin Framework
 -> The plugin framework that serves as a base for OPAQ
svn://sublime.vito.local/KWL/RMA/MAP/OPAQ/opaqPluginFramework
 
OPAQ common
 -> API and tools used by the other OPAQ components
svn://sublime.vito.local/KWL/RMA/MAP/OPAQ/opaqCommon

OPAQ Configuration Handler
 -> Configuration parser
svn://sublime.vito.local/KWL/RMA/MAP/OPAQ/opaqConfigurationHandler

OPAQ Engine
 -> OPAQ workflow implementation
svn://sublime.vito.local/KWL/RMA/MAP/OPAQ/opaqEngine

OPAQ Main
 -> The main OPAQ executable (this component)
svn://sublime.vito.local/KWL/RMA/MAP/OPAQ/opaqMain

For more information about the OPAQ architecture: see 
Y:\Unit_RMA\MAP\1310342 - AirINFORM (N78H9)\3 Werkdocumenten\OPAQ Technische analyse\Architectuur\

Each svn location has its own trunk/branches/tags

This example shows how to use the trunk version of all the components
and build the test/example program for OPAQ.

# create a temporary directory and cd into it
mkdir /tmp/opaq/
cd /tmp/opaq/

# each OPAQ component needs to checked out into the following
# exact directories, otherwise the Makefiles and generated scripts
# will not find the required includes/libs for the build.

# once you understand how the libraries depend on each other
# you can install them all in (for example) /usr/lib and
# /usr/include to make them accessible by default 
# eliminating the use of these exact directory names

mkdir OPAQ\ Plugin\ Framework
cd OPAQ\ Plugin\ Framework/
svn co svn://sublime.vito.local/KWL/RMA/MAP/OPAQ/opaqPluginFramework/trunk .
cd ..

mkdir OPAQ\ common
cd OPAQ\ common/
svn co svn://sublime.vito.local/KWL/RMA/MAP/OPAQ/opaqCommon/trunk .
cd ..

mkdir OPAQ\ Configuration\ Handler
cd OPAQ\ Configuration\ Handler/
svn co svn://sublime.vito.local/KWL/RMA/MAP/OPAQ/opaqConfigurationHandler/trunk .
cd ..

mkdir OPAQ\ Engine
cd OPAQ\ Engine/
svn co svn://sublime.vito.local/KWL/RMA/MAP/OPAQ/opaqEngine/trunk .
cd ..

mkdir OPAQ\ Main
cd OPAQ\ Main/
svn co svn://sublime.vito.local/KWL/RMA/MAP/OPAQ/opaqMain/trunk .
cd ..

# now we compile everything
cd OPAQ\ Plugin\ Framework/
make
cd ..
cd OPAQ\ common/
make
cd ..
cd OPAQ\ Configuration\ Handler/
make
cd ..
cd OPAQ\ Engine/
make
cd ..
cd OPAQ\ Main/
make

# now we can build the test/example plugins and run OPAQ 
# with the test/example configuration

cd test
make

cd run
./runme

# and now it is up to you.
# a good starting point is inspecting the generated runme
# script and the contents of the created run directory
# and its subdirectories.
