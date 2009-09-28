#
# Copyright (c) 2002-2005 University of La Reunion - France
# Authors:   Pascal ANELLI
#	     F HARIVELO
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
# THE UNIVERSITY OF PARIS LA REUNION BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
# SOFTWARE.
#
#

#puts "#load test-main.tcl"


#=======================================
##                 MAIN               ##
#=======================================

#====================================================
#              Procedures Definition
#====================================================

proc usage { argv0 ltest}  {
        puts stderr "Usage: ns $argv0  <tests>  \[DEBUG\] \[-f <RC File>\]  \[\{simulation parameters\}\]"
        puts stderr "Valid <tests> : $ltest" 
	puts stderr "\t\-f Ressource file name\n"
        puts stderr "\tsimulation parameters: "
        puts stderr "\t\t format: -<parameter>  <value>"
	puts stderr "\t\t for the parameters list, see ressource array often called opt"
exit 1
}


#====================================================
#              Main script
#====================================================


global argv argv0               
	
puts "   ----"
puts "BEGIN script file $argv0"
	set lclass [subclasslist TestSuite]
	set lcl ""
	foreach i $lclass {
		set lcl	[concat $lcl [lindex [split $i "/"] end]]	
	}
	set ltest $lcl
 
	set argc [llength $argv]   
	if {$argc < 1} {
		usage $argv0 $ltest
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
       		} else {
                        usage $argv0 $ltest
                }
        }
	
	# Ressource file name option
	set opt(f) "";

	# process simulation parameters in command line
        getopt opt [llength $argv] $argv

        if {$opt(f) !=""} {
                #puts "     Read  ressources file : $opt(f)"
    		source $opt(f)
	}

       
	if { [lsearch -exact $ltest $test] >= 0 } {
                set ts [new Test/$test]
        } else {
                 puts stderr "Unknown test $test"
                 usage $argv0 $ltest
        }

        $ts run

