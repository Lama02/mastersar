#
# Copyright (c) 2003 The University of Paris VI
# Authors:  P. ANELLI,
#	    F. HARIVELO
#
# All rights reserved.
# 
# Permission to use and copy this software in source and binary forms
# is hereby granted, provided that the above copyright notice, this
# paragraph and the following disclaimer are retained in any copies
# of any part of this software and that the University of Paris VI is
# acknowledged in all documentation pertaining to any such copy
# or derivative work. The name of the University of Paris VI may not
# be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
# EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF 
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL 
# THE UNIVERSITY OF PARIS VI BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
# SOFTWARE.
#
# Contributed by the RP Research Group, http://www.lip6.fr/rp
#


puts "#================================================ Test Suite"
#puts "#load test-suite-template.tcl"
source template.rc
source global.tcl
source test-topologies.tcl
source test-source.tcl
source test-error.tcl
source test-post-template.tcl

#=======================================
##                 Root Class         ##
#=======================================



Class TestSuite


TestSuite instproc init { args } {
	$self instvar ns_ net_ topo_ node_ flowactif_ 
	$self instvar test_ opt_ debug_ replication_ quiet_ verbose_
        $self instvar lastfid_ transfercnt_ traffic_

	global opt
	
	
	set lastfid_ 0
	set flowactif_ 0
        set transfercnt_ 0
	
        if {$opt(namgraph)} {
                set opt(nam) 1
         }
        
	array set opt_ [array get opt]
	# $self set-opt "[array get opt]"

	# create simulator instance
	set ns_ [new Simulator]
	if {[llength $args] > 0} {
	     if {[lindex $args 0] == "-multicast"} {
		$ns multicast
             }
	}
		
	# activate or not debug trace
	set debug_  [isdebug]
	
	# set others parameters
	if {![info exists test_]} {
		set name [$self info class]
        	set test_ [lindex [split $name /] 1]
	}
	
        set replication_ $opt(replication)
	global defaultRNG
	for {set i 1} {$i < $replication_} {incr i} {
              $defaultRNG next-substream
        }
	
        set quiet_ $opt(quiet)
        set verbose_ $opt(verbose)

	# Tune the simulation output
	$self SetOutput $opt(wd) $opt(nam)
        

	# Set an event to stop the simulation and call the post process 
	if {$opt(stoptime) == 0} {
                puts-error "Simulation duration is ZERO sec"
        } 
        $ns_ at $opt(stoptime) "$self finish"

	if {$net_ == ""} {
		global argv0
		puts-error "$argv0: no topology defined in test $test_"
	}

}


TestSuite instproc finish {} {
	$self instvar filenamenam_ filenamelog_
        $self instvar namout_ test_ opt_ verbose_
        global env
        

        puts "END Simulation is OVER\n   ----"
	# Close all the trace files
	$self cleanup
        if {$verbose_} {
                puts "LOG: \n[exec cat $filenamelog_ ]"
        }
	# Start network Animator
	if { $namout_ } {
               if {$opt_(namgraph)} {
                        puts "filtering..."
                        exec tclsh $env(NAM)/namfilter.tcl $filenamenam_
               }
  	       puts "running nam..."
	       exec nam $filenamenam_ &
	} else {
	   # Call post process then drawing function
	   setup_pprocess $test_ $opt_(quiet) $verbose_
        }
	
}



TestSuite instproc SetOutput {wd nam} {
	$self instvar ns_  test_   namout_
        $self instvar filenamenam_    namfilefd_   suffixnam_ 
	$self instvar filenamepkt_    pktfilefd_   suffixpkt_
        $self instvar filenameparam_  paramfilefd_ suffixparam_
        $self instvar filenamelog_    logfilefd_   suffixlog_
        
	# Color assigned to FID for nam
	# Define different colors for data flows
	# format: color <fid> <color>
	$ns_ color 0 red
	$ns_ color 1 green
	$ns_ color 2 blue
	$ns_ color 3 yellow
	$ns_ color 4 purple
	$ns_ color 5 orange
	$ns_ color 6 pink
	$ns_ color 7 cyan


        # create a new working directory
	# remove all old trace files
	if {[file exists $wd] && [file isdirectory $wd]} {
		set rootdir [pwd] 
		cd $wd
		if {![catch "glob *" todelete]} {
			eval exec rm -f $todelete
		}
		#puts [pwd]
		cd $rootdir 
		#exec rmdir $wd
	} else {
               exec mkdir $wd
	}
        #puts "[pwd] [file exists $wd]"
	# Change the working directory
	cd $wd

	# Find out suffix file name
	 set suffixpkt_ [GetSuffix pkt]
	 set suffixparam_ [GetSuffix param]
	 set suffixnam_ [GetSuffix nam]
         set suffixlog_ [GetSuffix log]

	# Set names for differents useful files
	$self set filenamepkt_ $test_.$suffixpkt_
	$self set filenameparam_ $test_.$suffixparam_
	$self set filenamenam_ $test_.$suffixnam_
        $self set filenamelog_ $test_.$suffixlog_
        
        set paramfilefd_ [open $filenameparam_ w]
	set logfilefd_ [open $filenamelog_ w]
        
	# activate trace for nam 
	set namout_ $nam
	if { $nam } {
	    set namfilefd_ [open $filenamenam_ w]
	    $ns_ namtrace-all $namfilefd_ 
	} else {
	# Open the trace files
        set pktfilefd_ [open $filenamepkt_ w]

        # activate trace packets activity for all the link
	$ns_ trace-all $pktfilefd_ 
       } 
	
} 

TestSuite instproc nam? {} {
	$self instvar namout_
	
	return $namout_
}


TestSuite instproc paramfd? {} {
        $self instproc paramfilefd_

        return $paramfilefd_
}


TestSuite instproc cleanup { } {
	$self instvar ns_ pktfilefd_ namfilefd_ paramfilefd_ logfilefd_
	
	$ns_ flush-trace

	if { [info exists pktfilefd_] } {
		close $pktfilefd_
		if { ![catch "$ns_ set WirelessNewTrace_"] && [$ns_ set WirelessNewTrace_] } {
			$self use-newtrace
		}
	}
        foreach item "logfilefd_ namfilefd_ paramfilefd_" {
	        if { [info exists $item] } {
		           eval close \$$item
	        }
        }
}


TestSuite instproc puts-log {msg} {
        $self instvar $logfilefd_
        puts $logfilefd_ $msg
}


TestSuite instproc just-before-run {} {
}


TestSuite instproc run {} {
	$self instvar ns_ opt_ debug_
        $self instvar guide_
        
        if {[info exists guide_]} {
              puts "****\nTEST: $guide_\n****"
        }                               
        $self PrintTopology
        $self CommentAnimation
	$self just-before-run
	
	if { $debug_ } {
	        puts "DEBUG Section"
		parray opt_
	}
	puts "END script file\n   ----"
	puts "BEGIN simulation..."
	$ns_ run
}


TestSuite proc instance {} {
	set lclass [subclasslist TestSuite]
	foreach test $lclass {
		set testsuite [$test info instances]
		if { $testsuite != "" } {
			return $testsuite
		}
	}
	puts-error "no TestSuite instance found"
}

#
#  To add comment in a nam animation
TestSuite instproc putcomment { ttime msg } {
 $ns_ at $ttime "$ns_ trace_annotate \"$msg\""
}

# Specialize this procedure for a specific simulation scenario (adding in rc file)
TestSuite instproc CommentAnimation {} {
}




#-----------------------------------------------#
#        TOPOLOGY
#-----------------------------------------------#

TestSuite instproc get-edge {edgeid} {
	$self instvar edge_
	
	return $edge_($edgeid)
}



TestSuite instproc create-topology {} {
	$self instvar opt_ ns_ edge_ corenetwork_ net_ debug_

	if { $debug_ } {
		puts "** Create core"
	}
	set corenetwork_ [new Core/$net_ $ns_ $self $opt_(routelength) $opt_(corelink)]
	
     if {$net_ == "Diffserv"} {
	for {set i 0} { $i < [llength $opt_(edgelink)] } {incr i} {
		if {  $debug_ } {
			puts "** create edge: $i"
		}
		set edgelinkparam [value $i $opt_(edgelink)]
		set edge_($i) [new Edge/$net_ $ns_ $self $corenetwork_ $edgelinkparam]
	}
     }
}

#
# to check the network topology 
TestSuite instproc PrintTopology {} {
	$self instvar corenetwork_ pattern_ logfilefd_

puts $logfilefd_ " TOPOLOGY"
puts $logfilefd_ "-------------------x-------------------x--------------------"
 $corenetwork_ print $logfilefd_
 foreach i [array names pattern_] {
 	$pattern_($i) print $logfilefd_
 }
puts $logfilefd_ "-------------------x-------------------x--------------------"
}


#-----------------------------------------------#
#        SOURCE
#-----------------------------------------------#


TestSuite instproc create-pattern {pattern replication} {
	$self instvar ns_ pattern_ debug_ patterntype_ test_

	for {set i 0} { $i < [llength $pattern] } {incr i} { 
		if { $debug_ } {
			puts "** Create pattern: $i"
		}

		# create pattern
	#	set code [catch {new SourcePattern/$patterntype_ $self [lindex $pattern $i]} pat]
	#	if {$code != 0 } {
		set pat [new SourcePattern/$patterntype_ $self [lindex $pattern $i] $replication]
	#	}
		set pattern_($i) $pat
		# connect sources and destination nodes of pattern to edge network
		#  or to the corenetwork 
		$pattern_($i) connect-pattern  DropTail
	}
}


TestSuite instproc get-pattern {type} {
	$self instvar pattern_ 

	set temp ""
	foreach i [array names pattern_] {
		if {$type == [$i set type_]} {
		      set temp [concat $temp $pattern_($i)]
		}
	}
	return $temp
}

# create counter for traffic class
TestSuite instproc flow-new {} {
        $self instvar lastfid_
        
        set fid $lastfid_
        incr lastfid_
        
        return $fid
}

TestSuite instproc get-traffic {id} {
        $self instvar traffic_

        if {![array exists $id]} {
                set traffic_($id) [new Traffic $id]
        }
        return $traffic_($id)
}



#----------------------------------------------------------#
#   POST PROCESS 
#----------------------------------------------------------#

TestSuite instproc post-process {paramlist} {
	$self instvar param_ 
	
	set param_ ""
	if { [llength $paramlist] == 0 } {
	                    return
	}
 
	set param_ $paramlist

}


TestSuite instproc tracevar {} {
	$self instvar param_ pattern_
	
	if { [llength $param_] == 0 } {
		return
	}
	# process  parameters by object types
	foreach item  $param_ {
		set typeobj [lindex $item 0]
		set totrace [lindex $item 1]
	
		# check if typeobj is an agent type then on= true
		set on [expr [lsearch -regexp [Agent info subclass] $typeobj] != -1]
                $self processtracevar $typeobj $totrace $on
	}
}


TestSuite instproc processtracevar {type ltodo where} {
	$self instvar pattern_
	
	#puts "Parameter scope $type \n\t\tTrace: $ltodo"
	foreach pat [array names pattern_] {
		if {$where} {;# on
			 if {![$pattern_($pat) agent? $type]} {
			   continue
	                 }
		} else { 
			# in
			if {![$pattern_($pat) contain? $type]} {
				continue
			}
		}
		foreach item $ltodo {
			$pattern_($pat) tracevar $item $where
		}
	}
}


TestSuite instproc QueueMonitor {} {
	$self instvar corenetwork_ pattern_ param_  paramfilefd_

	foreach scopeitem $param_ {
           if {[lsearch -regexp $scopeitem "RED"] != "-1"} {
                 $corenetwork_ enable_traceRED  $scopeitem $paramfilefd_
           }
        }
	# call  enable_traceQueue, monitor-flow

}


 
# TO DO : Tgoodput
# No TCP flows:	
#		set goodput [expr [$opt_(destination) set bytes_]*8/[expr [$opt_(destination) set lastPktTime_] - $opt_(starttime)] ]
#	puts "Total recv thoughput from source: [$opt_(agent) set agent_addr_] fid  [$opt_(agent) set fid_] [format "%9.0f" $goodput ]bits/s\n"



TestSuite instproc Tgoodput {} {
	$self instvar ns_ 

	set now [$ns_ now]
	set lpattern [$self get-pattern TCP]
	
	foreach p $lpattern {
                               
		foreach a [$p get-agents TCP] {
		  set finishtime [$a set finistime_]
		  if {$finishtime == -1 } {
			set finistime $now
		  }
        	  set starttime [$a set starttime_]
		  set bytes [$a set ack_]
		
        	  set rate [expr $bytes*8*[$a set packetSize_]/($finishtime - $starttime)]

        	  puts  "tcp_goodput: agent [$a set agent_addr_ ] fid [$a set fid_] [format "%9.0f" $rate] bits/s $bytes"
		}
	}	
}



# Add a monitor on a specific link
TestSuite instproc enable_traceQueue {fdtracefile} {
	$self instvar opt_
	
	if {$fdtracefile == 0} {
		return
	}
	$opt_(ns) monitor-queue $opt_(n1) $opt_(n2) $fdtracefile
	$opt_(ns) at [$opt_(ns) now] "$opt_(bottleneck_link) start-tracing"
}


# 	x axis: time
#       y axis: queue size
#  Perform post-process from trace made by enable_traceQueue
TestSuite instproc length {} {
	$self instvar opt_
	
	# input format:
	set awk_queue {
		BEGIN {
		}
		/^\q/ {
			printf "%g %g\n", $3, $11;
		}
		END {
			printf "\n\n";
		}
	}


set awk_code {
        BEGIN{
	ave = 0.0;
	}
        /^\q/ { 
	  ave *= (1-q_weight);
	  ave +=  q_weight*$11;
          printf "%g %g\n", $3, $11;
        }
        END{ printf "\n\n"; }
}
#      exec awk $awk_code q_weight=$opt(q_weight) $test_.$opt(param) > $nameoutfile

	exec awk $awk_queue $tracefile_ 
}




#  perform post-process from trace made by enable_trace
# input format:
TestSuite instproc monitor-flow { fdtracefile sampleInterval } {
	$self instvar opt_
	
	if {$fdtracefile == 0} {
                return
        }
        set l01 [$opt_(ns) link $opt_(n0) $opt_(n1)]
	set fmon [$opt_(ns) makeflowmon Fid]
	$opt_(ns) attach-fmon $l01 $fmon
        $fmon attach $fdtracefile
	
	$opt_(ns) at $sampleInterval "$self DumpMonitor $fmon $fdtracefile $sampleInterval"
}




# Prints "time: $time class: $class bytes: $bytes" for the link.
TestSuite instproc DumpMonitor { fmon fdtracefile sampleInterval } {
	$self instvar opt_
	
	foreach flow [$fmon flows] {
		set bytes [$flow set bdepartures_]
		if {$bytes > 0} {
			puts $fdtracefile "fmon [$opt_(ns) now] x x bytes: $bytes $sampleInterval [$flow set flowid_]"  
			$flow reset
		}
        }
	#$fmon dump
        $opt_(ns) at [expr [$opt_(ns) now] + $sampleInterval] "$self DumpMonitor $fmon $fdtracefile $sampleInterval"
}



#----------------------------------------------------------------------
#           TRACING
#----------------------------------------------------------------------
TestSuite instproc use-newtrace {} {
	$self instvar opt_  namepktfile_

	
	set awk {
		function getValue(parameterName) {
			for(i=1;i<=NF;i++) {
				if($i == parameterName) {
					j = i + 1
					return $j
				}
			}
			return FFLAG
		}
		
		BEGIN {
			FEVENT=1 ; FTIME="-t"
			FINGRESS="-Hs" ; FEGRESS="-Hd"
			FPTYPE="-It" ; FPLEN="-Il"
			FFLAG="-------" ; FFID="-If"
			FSA="-Is" ; FDA="-Id" ;
			FSEQTCP="-Ps" ; FSEQACK="-Pa" ; FSEQUDP="-Pi" ; FUID="-Ii"
			FDROPREA="-Nw"
		}
		
		{
			event=$FEVENT
			if(event != "s" && event != "r" && event != "d" && event != "f")
				next       
			
			protocole=getValue(FPTYPE)
			if(protocole == "tcp")
				FSEQ=FSEQTCP
			else {
				if(protocole == "ack")
					FSEQ=FSEQACK
				else
					FSEQ=FSEQUDP
			}
			
			if(event=="s")
				event="+"
				
			if(event=="d" && getValue(FDROPREA) == "COL")
				event="c"
			
			if(event != "c" && (getValue(FTIME) == FFLAG || getValue(FINGRESS) == FFLAG || getValue(FEGRESS) == FFLAG || getValue(FPTYPE) == FFLAG || getValue(FPLEN) == FFLAG || getValue(FFID) == FFLAG || getValue(FSA) == FFLAG || getValue(FDA) == FFLAG || getValue(FSEQ) == FFLAG || getValue(FUID) == FFLAG))
				next
				
			#print event, getValue(FTIME), getValue(FINGRESS), getValue(FEGRESS), getValue(FPTYPE), getValue(FPLEN), FFLAG, getValue(FFID), getValue(FSA), getValue(FDA), getValue(FSEQ), getValue(FUID)
			# A ENLEVER, temporaire pour un reseau sans fils sans routage avec l'utilisation de DumpAgent
			fegress = getValue(FEGRESS)
			if(event=="r" && fegress < 0)
				fegress="0"
			print event, getValue(FTIME), getValue(FINGRESS), fegress, getValue(FPTYPE), getValue(FPLEN), FFLAG, getValue(FFID), getValue(FSA), getValue(FDA), getValue(FSEQ), getValue(FUID)
		}
		#END {
		#	split(args, tab, " ")          
		#	for (elt in tab)
		#	     print tab[elt]
		#}
	}
	
	exec $opt_(AWK) $awk $namepkfile > .$namepkfile
	exec mv $namepkfile $namepkfile.tmp
	exec mv .$namepkfile $namepkfile
}


