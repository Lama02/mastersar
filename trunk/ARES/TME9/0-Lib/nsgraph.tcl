set dir [pwd]
catch "cd 0-Lib"
source template.rc
source global.tcl
source test-grapher.tcl
catch "cd $dir"

#------------------------------------
proc usage { argv0 ltest}  {
        puts stderr "Usage:  $argv0  <metric> <filename> -wd <name>"
	exit 1
}
						
#-----------------------

global argc argv argv0


        if {$argc < 4} {
	                usage $argv0 
		  }
        if { [expr $argc % 2 ] != 0 } {
                usage $argv0
        }
        getopt $argc [lrange $argv 1 end]  

       cd $opt(wd)			
       set graph [new Grapher/$opt(grapher) [lindex $argv 1] [lindex $argv 0]] 
       $graph draw $opt(quiet)

