
MED Format C library and documentation

For more information about MED, see medformat.org

Edit targets.h to set the target operating system (and if appropriate, target application - only Matlab is supported as a target application currently)

MED2RAW.c is example code for converting MED files to a series of 4-byte integer files, one per channel.

Shell Scripts: (edit these for your target locations & compiler, specified at the top)
mklibs.sh is a shell script to create MED libraries in MacOS & Linux
mklibs.bat is a shell script to create MED libraries in Windows
compile.sh is a shell script to compile MED2RAW using MED libraries in MacOS & Linux
compile.bat is a shell script to compile MED2RAW using MED libraries in Windows
