#
# Copyright (c) 2006 University of La Reunion - France
# Authors:   Pascal ANELLI
#            Fanilo HARIVELO
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

#puts "================================================ Post Suite"

#puts "load test-post-template.tcl"
source test-metric.tcl
source test-grapher.tcl

#========================================
Class PostSuite
#========================================
PostSuite instproc init {test verbose} {
	$self instvar conf_ test_ verbose_
	$self instvar plotid_ graphsuite_
	
	set plotid_ 0
	set verbose_ $verbose
	# Post process configuration 
	global conf opt
        array set conf_ [array get conf]
	
	set test_ $test
	
	# Process the asking analysis (post process option)	
	$self SetMetric

	#new GraphSuite
	set graphsuite_ [new GraphSuite $opt(grapher) $conf(plot) $verbose]
}



PostSuite instproc SetMetric {} {
    $self instvar post_ test_ conf_ plotid_ verbose_


	foreach scope $conf_(pkt) {
		set scopename [lindex $scope 0]
		set scopelist [lindex $scope 1]
		
	        if {[llength $scopelist] == 0} {
			continue
		}
	       foreach item $scopelist {
                        if {$verbose_} {
		   	  puts "Create post id $plotid_ for $scopename with {$item}"
			}
                        set post_($plotid_) [new Metric/$scopename $test_ $conf_(parameter) $item $verbose_]
	         	incr plotid_
	  	}
	}
	
	# tracedvar list in the format : {who what}
	foreach scope $conf_(param) {
		set tracedobject [lindex $scope 0]
		set ltvar [lindex $scope 1]
		
	        if {[llength $ltvar] == 0 } {
			return
		}
		# do a unique list of tracedvar
		set tmp {}
	        foreach item $ltvar {
		    foreach i [lindex $item 1] {
		    	lappend tmp $i
		    }
	        }

	       set lsorted [lsort $tmp]

	       set metrics {}
	       set pred ""
 	       foreach i $lsorted {
		    if {$i == $pred} {
			continue
		    }
		    lappend metrics $i
		    set pred $i
	       }
	      if {$verbose_} {
	        puts "Create post id $plotid_ for $tracedobject with $metrics"
	      }
              set post_($plotid_) [new Metric/$tracedobject $test_ $conf_(parameter) $metrics $verbose_]
	      incr plotid_
	}			
	puts "   ---- "
}


# Process the tracefiles in order to get data to plot
PostSuite instproc ComputeData {} {
	$self instvar post_ graphsuite_ verbose_

        foreach i [array names post_ ] {
                if {$verbose_} {
	        	puts "Call post id $i"
		}
                set ldatafile [$post_($i) MakeData]
		if {[llength $ldatafile] >0} {
	        	$graphsuite_ CreatePlot $ldatafile
        	}
	}
	puts "   ---- "
}


PostSuite instproc DrawGraph {quiet} {
	$self instvar graphsuite_

	$graphsuite_ DrawGraph $quiet
}


#=============================================================================
Class GraphSuite
#=============================================================================

GraphSuite instproc init {grapher plotlist verbose} {
	$self instvar grapherid_ 
	$self instvar plot_ verbose_
	$self instvar grapher_ suffixdata_

	set grapherid_ 0
        set verbose_ $verbose

	set grapher_ $grapher
	set plot_ $plotlist

	set suffixdata_ [GetSuffix data]
	
}



GraphSuite instproc CreatePlot {ldatafile} {
	$self instvar graph_ plot_ suffixdata_ grapher_
	$self instvar grapherid_ verbose_
	
	# Merge files according plot_
	foreach operation $plot_ {
		set operationname [lindex $operation 0]
                set index -1
                set i 0
                foreach item $ldatafile {
	                set code [lsearch -exact $item $operationname]
                        if {$code >=0} {
                                set index $i
                                break
                        }
                        incr i
                }
		#puts "CreatePlot:  $operationname index: $index"
                if {$index == "-1" } {
			#puts "ERROR in CreatePlot: $operationname not found"
			continue
		}
		if {"[lindex [lindex $ldatafile $index] 0]" != "$operationname" } {
		      continue
		}
		set operationfile [lindex [lindex $ldatafile $index] 1]
		set resultfile [file rootname $operationfile]
		append resultfile "comp." $suffixdata_
		if {$verbose_} {
                        puts -nonewline "     plot: $operationname in "
		        puts "$resultfile"
                }
		catch "exec rm -f $resultfile"
		# Copy the base file in resultfile
		exec cat $operationfile > $resultfile
		set ldatafile [lreplace $ldatafile $index $index]
		foreach merged [lrange $operation 1 end] {
			set index [lsearch -regexp $ldatafile $merged]
			if {$index >=0} {
				set mergedfile [lindex [lindex $ldatafile $index] 1]
				exec cat $mergedfile >>$resultfile
				set ldatafile [lreplace $ldatafile $index $index]
			}
		}
		lappend ldatafile [list $operationname $resultfile]
	}

	# create a grapher per plot
	foreach i $ldatafile {
		set metric [lindex $i 0]
		set datafile [lindex $i 1]
		
		set graph_($grapherid_) [new Grapher/$grapher_ $metric $datafile $verbose_]
		incr grapherid_
		}
}


# Draw graphes according to data file
GraphSuite instproc DrawGraph {quiet} {
	$self instvar graph_

	foreach i [array names graph_ ] {
                $graph_($i) draw $quiet
        }
}


#====================================================
#              Procedure definition
#====================================================      

proc setup_pprocess {test quiet verbose} {
        #if {$verbose} {
                puts "BEGIN post process"
        #}
        set post_suite [new PostSuite $test $verbose]
	$post_suite ComputeData
	$post_suite DrawGraph $quiet
        #if {$verbose} {
                puts "END post process"
        #}
}


