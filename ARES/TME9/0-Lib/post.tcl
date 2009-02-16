#
# Copyright (c) 2008 University of La Reunion - France
# Authors:   Pascal ANELLI
# http://personnel.univ-reunion.fr/panelli
# All rights reserved.
# 
# Permission to use and copy this software in source and binary forms
# is hereby granted, provided that the above copyright notice, this
# paragraph and the following disclaimer are retained in any copies
# of any part of this software and that the University of La Reunion is
# acknowledged in all documentation pertaining to any such copy
# or derivative work. The name of the University of La Reunion may not
# be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
# EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF 
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL 
# THE UNIVERSITY OF LA REUNION BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
# SOFTWARE.
#
#
# load common library
set dir [pwd]
catch "cd $env(LIBSUITE)"
source template.rc
source global.tcl
source test-post-template.tcl
catch "cd $dir"

#====================================================
#              Procedures Definition
#====================================================

proc usage { argv0 }  {
        puts stderr "Usage: otclsh $argv0  <testname>  \[DEBUG\] \[-f <RC File>\] \[\{parameters\}\]"
        puts stderr "\t\-f Ressource file name\n"
        puts stderr "\tparameters: "
        puts stderr "\t\t format: -<parameter>  <value>"
        puts stderr "\t\t for the parameters list, see ressource array often called conf"
        exit 1
}


#====================================================
#                       Main
#====================================================

global argc argv argv0

puts "   ----"

        if {$argc < 1} {
                usage $argv0
        }

        set test [lindex $argv 0]
        set argv [lrange $argv 1 end]
        set argc [llength $argv]

        if { [expr $argc % 2] == 1 } {
                set q [lindex $argv 0]
                if { $q == "DEBUG" } {
                        set debug 1
                        set argv [lrange $argv 1 end]
                        set argc [expr $argc - 1]
                }
        }

        # Ressource file name option
        set opt(f) "";

        # process simulation parameters in command line
        getopt opt [llength $argv] $argv

        if {$opt(f) !=""} {
                puts "  Read  ressources file : $opt(f)"
                source $opt(f)
        }

cd $opt(wd)

set except 0
foreach i [glob -nocomplain "${test}*"] {
        if {$except < 3 } {
                incr except
                continue
        }
        exec rm -f  $i
}

        
setup_pprocess $test $opt(quiet) $opt(verbose)
#====================================#

