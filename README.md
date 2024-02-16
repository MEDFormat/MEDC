
MED Format C library and documentation

For more information about MED, see medformat.org

Edit targets_m12.h to set the target operating system, and if appropriate, target application.
Currently, Postgres & Matlab are the only supported target applications.

MED2RAW.c is example code for converting MED files to a series of 4-byte integer files, one per channel.
These can be read into Matlab as follows:
fp = fopen('<channel_name>.raw', 'r');
<channel_name> = fread(fp, 'int32=>double');
fclose(fp);

Shell Scripts: (edit these for your target locations & compiler, specified at the top). 
mklib.sh is a shell script to a create MED library in MacOS & Linux. 
mklib.bat is a shell script to a create MED library in Windows. 
compile.sh is a shell script to compile MED2RAW using the MED library in MacOS & Linux. 
compile.bat is a shell script to compile MED2RAW using the MED library in Windows. 

The API documentation is currently being updated for m12 libraries.  
New record types may be added to the medrec_m12 prior to their appearance in the MED Records documentation.
