#
# Copyright (c) 2008 University of La Reunion - France
# Author:   Pascal ANELLI
# http://www.univ-reunion.fr/~panelli
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



#puts "#load test-source.tcl"

source test-RV.tcl

# Traffic: a set of pattern
#           counter for a traffic class
Class Traffic

Traffic instproc init {idclass} {
        $self instvar ns_ logfile_ testsuite_
        $self instvar class_ flowinprogress_ tid_ lastarrival_

        set ns_ [Simulator instance]
        set testsuite_ [TestSuite instance]
        set logfilefd_ [$testsuite_ set logfilefd_]

        set class_ $idclass
        set flowinprogress_ 0
        set tid_          0
        set lastarrival_ 0.0
}

Traffic instproc class? {} {
        $self instvar class_

        return $class_
}

Traffic instproc start {} {
        $self instvar ns_ flowinprogress_ tid_ lastarrival_
        $self instvar intera_

        set now [$ns_ now]
        if {$lastarrival_ == 0.0} {
                set intera_ 0.0
         } else {
                set intera_ [expr $now - $lastarrival_]
         }
         set lastarrival_ $now
        incr flowinprogress_
        incr tid_

        return [list $flowinprogress_ $intera_]
}

Traffic instproc finish {} {
        $self instvar flowinprogress_ logfilefd_

        incr flowinprogress_ -1
        return $flowinprogress_
}


# Pattern: a set of source-destination node with a same traffic pattern
Class SourcePattern

SourcePattern instproc init {testsuite pattern replication} {
	$self instvar ns_ edge_ testsuite_ edgeid_ replication_ verbose_
	$self instvar  core_ debug_ connection_ logfilefd_ class_ traffic_
        $self instvar RVsize_ RVdelay_ transfersize_ starttime_ idle_flow_

	set debug_ [isdebug]
	set ns_ [Simulator instance]
	set testsuite_ $testsuite 
        set replication_ $replication 
        set logfilefd_ [$testsuite set logfilefd_]
        set idle_flow_ {}
       
        set verbose_ [$testsuite set verbose_]

        #puts "new pattern:\n $pattern"
	#set testsuite_ [TestSuite instance]

	# process pattern parameter
	instance-parameter $self $pattern

        if {![info exists class_]} {
                set class_ be
        }
        set traffic_ [$testsuite get-traffic $class_]

	if {[info exists edgeid_]} {
		set edge_ [$testsuite get-edge $edgeid_]
	} else {
		set core_ [$testsuite set corenetwork_]
        }
	# create nodes (source and destination) of the pattern
	$self create-node
	# Assign edge node of the pattern
	$self assign-xgressnode
        # Start the flows in the pattern
        if {[info exists connection_] && ($connection_ == "Short")} {
               # puts "-->>$connection_"
                $self ScheduleTransfer 
        } else {
                #puts "-->>Long"
                set connection_ "Long"
                if {[info exists transfersize_]} {
                        $self StartTraffic
                } else {
                        $self Finishtime
                }
       }
}


# create source and destination nodes (according nbnode_) 
SourcePattern instproc create-node {} {
	$self instvar flows_ node_ sourceparam_ nbnode_ nbflow_ type_
	$self instvar  ns_ nodetype_ testsuite_ traffic_

	for {set i 0} {$i < $nbnode_} {incr i} {
		if { [isdebug] } {
			puts "New source node: $i $nodetype_"
		}
		set node_($i,s) [$ns_ $nodetype_]
		set node_($i,d) [$ns_ node]     
		
		# create flows in each node
		set nbflow  [value $i $nbflow_] 

		for {set j 0} {$j < $nbflow} {incr j} {
                     set flows_($i,$j) [new Flow/$type_ $self $node_($i,s) $node_($i,d)  $sourceparam_ [$testsuite_ flow-new] $i $traffic_]
		}
	}
}


# indicate the (i,e)gress node. gress node is node to be connected to the network
# by default, the source-destination is (i,e)gress node 
SourcePattern instproc assign-xgressnode  {} {
	$self instvar node_ nbnode_

	for {set i 0} {$i < $nbnode_} { incr i} {
		set node_($i,igress) $node_($i,s)
		set node_($i,egress) $node_($i,d)
	}
}

# For dynamic flow
SourcePattern instproc ScheduleTransfer {} {
        $self instvar RVsize_ RVdelay_ nbnode_ nbflow_ flows_ transfersize_
        $self instvar starttime_ laststime_ idle_flow_ replication_
        
        set RVsize_ [new RV $transfersize_ $replication_]
        set RVdelay_ [new RV $starttime_ $replication_]
        
        for {set i 0} {$i <$nbnode_} {incr i} {
                set laststime_($i) 0.0
                set nbflow  [value $i $nbflow_] 
                for {set j 0} {$j < $nbflow} {incr j} {
                        set pos [expr ($i+1)*$j]
                        set idle_flow_ [linsert $idle_flow_ $pos $flows_($i,$j)] 
                        $flows_($i,$j) conf-short $RVdelay_ $RVsize_
                 }
                 $self StartTransfer $i
        }
}

SourcePattern instproc StartTransfer {nodeidx} {        
        $self instvar idle_flow_ ns_ nbnode_
       # Have we still available flow ?
        #puts $tracefilefd_ "t: [format %.3f $now ] client list cl: $traffic_class list: $idle_clients_($traffic_class)"
        if {[llength $idle_flow_] == 0} {
                #set idle_flow_ [linsert $idle_flow_ end [$self create_flow] ]
                puts-error "[format %8.5f [$ns_ now]] Missing flows in the pool for node idx $nodeidx on $nbnode_ nodes "
        }
        set flow [lindex $idle_flow_ 0]
        set idle_flow_ [lrange $idle_flow_ 1 end]
        
        #puts "[$ns_ now] starting node index $nodeidx"
        $flow reschedule
}


SourcePattern instproc FinishTransfer {flow} {
        $self instvar testsuite_ idle_flow_ traffic_

        set idle_flow_ [linsert $idle_flow_ end $flow]
        # return the number of active flows
        return [$traffic_ finish]
}


# For Long Flow
SourcePattern instproc StartTraffic {} {
        $self instvar starttime_ nbflow_ transfersize_ flows_ nbnode_
        
        for {set i 0} {$i <$nbnode_} {incr i} {
            set nbflow  [value $i $nbflow_]                
            set flowstart [value $i $starttime_]
            set flowsize [value $i $transfersize_]
            for {set j 0} {$j < $nbflow} {incr j} {
               $flows_($i,$j) schedule-app [value $j $flowstart] [value $j $flowsize]
            }
        }
}

# for UDP
SourcePattern instproc Finishtime {} {
        $self instvar nbnode_ finishtime_ starttime_ nbflow_ flows_
         
         for {set i 0} {$i <$nbnode_} {incr i} {
            set nbflow  [value $i $nbflow_]
            set flowstart [value $i $starttime_]
            #set flowstart $starttime_
            for {set j 0} {$j < $nbflow} {incr j} {
                set starttime [value $j $flowstart]
                if {[info exists finishtime_]} {
                        set ftime [value $j [value $i $finishtime_]]
                        #set ftime [value $i $finishtime_]
                } else {
                        set ftime -1
                }
                $flows_($i,$j) schedule-app $starttime $ftime
            }
        }
}


# connect all node of the pattern to the network
SourcePattern instproc connect-pattern {queue} {
	$self instvar nbnode_ node_ edge_ bw_ tp_  core_ hook_

	for {set i 0} {$i < $nbnode_} {incr i} {
		if { [isdebug] } {
			puts "Connect source node: $i; value bw & tp"
		}
		set linkbw [lvalue $i $bw_]
		set linktp [lvalue $i $tp_]
		
		if {[info exists edge_]} {
		# give edge node of the pattern
		$edge_ connect-flow  $node_($i,igress) $node_($i,egress) $linkbw $linktp $queue
		} elseif {[info exists core_]} {
			$core_ connect-flow  $node_($i,igress) $node_($i,egress) $hook_ $linkbw $linktp $queue
		} else { puts-error "Where does pattern connect?"}
	}
}


# give (r,b) parameter for the token bucket policer
SourcePattern instproc ConfigurePolicer {} {
	$self instvar nbnode_ edge_ node_ cir_ cbs_ testsuite_
	
	set policy [value 0 [$edge_ set policy_]]
	if {$policy == "ANY"} {
		return
	}
	
	for {set i 0} {$i < $nbnode_} {incr i} {
		if { [isdebug] } {
			puts "Configure edge with source node: $i; value cir & cbs"
		}
		set flowcir [bw_parse [value $i $cir_]]
		set flowcbs [value $i $cbs_]
		
		$edge_ AddPolicy  $node_($i,s) $node_($i,d) $flowcir $flowcbs
	}
}
SourcePattern instproc agent? {type} {
	$self instvar type_	

	return [string match $type $type_]
}


SourcePattern instproc contain? {type} {
	$self instvar nodetype_ node_

	if {$nodetype_ != "ConditionerNode"} {
		return 0
	}
	set box [$node_(0,igress) entry]
	return [string match $type [$box info class]]
}

# return a list according to a simple regexp 
SourcePattern instproc cast {casting} {
	$self instvar nbnode_

	# Interpret the casting field
	set lcasting ""
	foreach element $casting {
		    if {$element == "*"} {
		        set lcasting ""   
		    	for {set i 0} {$i < $nbnode_} {incr i} {
				lappend lcasting $i
			}
		        break
		    }
		    if {[lsearch -regexp $element "-"] != -1} {
		    	set range [split $element "-" ]
			set max [lindex $range 1]
			if {$max == "\$"} { set max $nbnode_}
			for {set i [lindex $range 0]} {$i <= $max} {incr i} {
				lappend lcasting $i
			}
		     } else {
				lappend lcasting $element
		    }
	}
	return $lcasting
}


 # TCP PARAM 
        # enable source tracing
        #       enable TCP tracing, using to following some internal parameters 
        #
        # Use generic tracer
        #       set tracer [new Trace/Var]
        #       $tracer attach $pfd
        #       example:        $tcp(0) trace "ack_" $tracer
        # Or use the TCP tracer   like here     

#listvar format: { {{who} {what}}* }
#          who is the index or the index range of the object to trace
#              in the pattern. By default' who' is the node index in the pattern
#	   nota: Index start from 0
SourcePattern instproc tracevar {varlist on} {
	$self instvar testsuite_ node_ flows_ ns_ type_ nbflow_

	   set paramfilefd [$testsuite_ set paramfilefd_]
	   if {[llength $varlist] !=2} {
		puts-error "in [info level]: Not enough element \n\t$varlist"
 	   }
	   set casting [lindex $varlist 0]
	   set tvars [lindex $varlist 1]
	   set lcasting [$self cast  $casting]
	   # Activate tracevar in Tracedobject 
	   foreach i $lcasting {
	    	        # is it an agent ?
		if {$on} {
                        set lflow [$self get-flows $i]
                        set lobject {}
                        foreach flow $lflow {
                                set lobject [concat $lobject [$flow get-agent]]
                        }
                        if {[$testsuite_ nam?]} {
                                foreach item $lobject {
                                   $ns_ add-agent-trace $item [format "%s%02d" $type_ [$item set fid_]]
                                   $item set nam_tracevar_ true
                                   $ns_ monitor-agent-trace $item
                                        foreach j $tvars {
                                                $item tracevar ${j}_
                                        }
                                }
                                continue
                        }
		} else {
			# is it a box ?
			set lobject [$node_($i,igress) entry]
		}
		foreach item $lobject {
			$item attach $paramfilefd
			# loop on list of tracevar
			foreach j $tvars {
                         #	set tracer [new Trace/Var]
                         #      $tqracer attach  $paramfilefd
			     $item trace ${j}_
			}
		}
	   }
}

SourcePattern instproc get-node {nodeindex} {
        $self instvar node_ nbnode_

        if {$nodeindex >= $nbnode_} {
                puts-error  "get-node : node index $nodeindex over nbnode $nbnode_"
        }
        return [list $node_($nodeindex,s) $node_($nodeindex,d)]
}

# Return the list of flows in node nodeindex of pattern
SourcePattern instproc get-flows {nodeindex} {
        $self instvar nbflow_ flows_

       set nbflow  [value $nodeindex $nbflow_]
       set lflow {}
       for {set j 0} {$j < $nbflow} {incr j} {
                lappend lflow $flows_($nodeindex,$j)
       }
       return $lflow
}


# return the value of the parameter
# The name of the parameter is given without trailing character i.e. "_"
SourcePattern instproc param-value {type paramname} {
	$self instvar type_
		
	if  {$type != $type_} {
		return ""
	}
	append paramname "_"
	return [$self set $paramname]
}


#TO DO
SourcePattern instproc debug {listdebug} {
}


SourcePattern instproc rttmin {} {
        $self instvar nbnode_ flows_ node_

        set lrtt {}
      # Compute RTT for each node of the pattern
        for {set i 0} {$i < $nbnode_} {incr i} {
  		set agent [$flows_($i,0)  get-agent]
  		set packetSize [$agent set packetSize_]
  		lappend lrtt [format "%-8.5f" [rtt $node_($i,s) $node_($i,d) $packetSize]]
	}
        return $lrtt
}


SourcePattern instproc print {fd} {
	$self instvar flows_ type_ hook_ edgeid_ node_ nbnode_ nbflow_ bw_ tp_

	puts $fd "Source Pattern: $type_"
	for {set i 0} {$i < $nbnode_} {incr i} {
		puts $fd "\t source node id: [$node_($i,s) id] index: $i"
		puts $fd "\t destin node id: [$node_($i,d) id] index: $i"
		if {[info exists edgeid_]} {
		puts $fd "\t	 connected to edge index $edgeid_"
		} else {
		puts $fd "\t     connected to network node id $hook_"
		}
                set tp [lvalue $i $tp_]
                set bw [lvalue $i $bw_]
                puts $fd "\t        access tp [value 0 $tp]  bw [value 0 $bw]"
                puts $fd "\t        exit   tp [value 1 $tp]  bw [value 1 $bw]"
  	        set nbflow  [value $i $nbflow_] 
	        for {set j 0} {$j < $nbflow} {incr j} {
		    $flows_($i,$j) print $fd $j
	        }
	}
}



# add the error (a list of errors)  applied to all the flows of the node
SourcePattern instproc add-error {{returnpath 0} {front 1}} {
        $self instvar error_ replication_ re_ nbnode_ flows_ testsuite_ hook_

        if {![info exists error_]} {
                puts "No Error Model in pattern hook: $hook_ with $nbnode_ nodes"
                return
        }
        for {set j 0} { $j < $nbnode_} {incr j} { 
                set re_($j) {}
                set errorflow $error_
                if {$errorflow == {}} {
                        continue
                }
                set ecn [[$flows_($j,0) set agent_] set ecn_]
                set em [new ErrorModule Fid 29]
                
                $self insert-errormodule $j $em $returnpath $front
                
                set prev {}
                set lre {}
        	for {set i 0} {$i < [llength $errorflow]} {incr i} {
                   set re [new Error [lindex $errorflow $i] $replication_]
        	   lappend lre $re; # list of Error objects for this node
                   # Get Error model  (rmod)
   	           set rmod [$re get-emod]
                   $rmod set markecn_ $ecn
        	   $rmod drop-target [$em drop-target]
                   if {$prev == {}} {
                           # bind rmod to fid 0
                           $em bind $rmod 0
                           # default traffic passed to rmod
                           $em default $rmod
                   } else {
	                   set pmod [$prev get-emod]
        		   $pmod target $rmod
                   }
	           set prev $re
	        }
                # link rmod with the target of em
                $em insert $rmod
                lappend re_($j)  $lre
        }
}

# Add error module on the first flow  of the first pattern
# in the acces link (after the queue of access link)
# return the lossy link
SourcePattern instproc insert-errormodule {index error returnpath front } {
        $self instvar node_ core_ ns_ hook_ verbose_
        
        set nodes [$self localize-link $index $returnpath $front]
        set nodei [lindex $nodes 0]
        set nodee [lindex $nodes 1]
        if {$verbose_} {
                puts "Install error model ratio from node [$nodei id] to [$nodee id]"
        }
        # call SimpleLink errormodule
        $ns_ lossmodel $error $nodei $nodee
         #  return [$ns_ link $nodei $nodee]
}   


SourcePattern instproc localize-link {index returnpath front} {
       $self instvar node_ core_ ns_ hook_
       
    set entrynode [lindex $hook_ 0]
    set exitnode  [lindex $hook_ 1]
    if {$front} {
        if {$returnpath} {
                set nodei  $node_($index,egress)
                set nodee  [$core_ node? $exitnode]
        } else {
                set nodei  $node_($index,igress)
                set nodee  [$core_ node? $entrynode]
        }
     } else { 
        # Behind the corenetwork
        if {$returnpath} {
                set nodei [$core_ node? $entrynode]
                set nodee $node_($index,igress)
        } else {
                set nodei  [$core_ node? $exitnode]
                set nodee  $node_($index,egress)
        }
     }
     return [list $nodei $nodee]
}


SourcePattern instproc ErrorTrace {paramfilefd} {
        $self instvar re_ nbnode_
        if {![info exists re_]} {
                return
        }
        for {set j 0} { $j < $nbnode_} {incr j} { 
                foreach re $re_($j) {
			$re tracevar $paramfilefd                        
                }
        }
}


SourcePattern instproc  select-ca {} {
        $self instvar flows_ ns_ nbnode_ type_ nbflow_
        
        if {type_ != "TCP"} {
                return
        }
        for {set i 0} {$i < $nbnode_} {incr i} {
                set nbflow [value $i $nbflow_]
                for {set j 0} {$j < $nbflow} {incr j} {
                        flows($i,$j) select-ca
                }
        }
}


SourcePattern instproc add-tbf {shaper {returnpath 0} {front 1}} {
        $self instvar shaper_ nbnode_ ns_ policer_ verbose_

        if { $shaper && ![info exists shaper_]} {
		puts "NO shaper defined"
                return
	}
        if { !$shaper && ![info exists policer_]} {
                puts "NO policer defined"
                return
        }
        
        if {$shaper} {
                set ltbf $shaper_
        } else {
                set ltbf $policer_
        }
        for {set i 0} {$i < $nbnode_} {incr i} {
           set prev {}
           for {set j 0} {$j < [llength $ltbf]} {incr j} {
                set param [value $j $ltbf]
                set tbf [new TBF]
                instance-parameter $tbf $param
                $tbf set rate_ [$ns_ bw_parse [$tbf set rate_]]

                set nodes [$self localize-link $i $returnpath $front]
                set nodei [lindex $nodes 0]
                set nodee [lindex $nodes 1]
                if {$verbose_} {
                        puts "Install tbf between node [$nodei id] - [$nodee id]"
                }
                set link [$ns_ link $nodei $nodee]
                $tbf drop-target [$link set drophead_]
                if {$prev == {}} {
                        #$link add-to-head $tbf
                        set delay [$link set link_]
                        $tbf target [$delay target]
                        $delay target $tbf
                 } else {
                        $tbf target [$prev target]
                        $prev target $tbf
                 }
                 set prev $tbf
           }
        }
}


Class SourcePattern/Classic -superclass SourcePattern

SourcePattern/Classic instproc init {testsuite pattern replication} {
	$self instvar nodetype_
	
	set nodetype_ node
	$self next $testsuite $pattern	$replication
}


# A pair of node: may have several flows
#=====================================================
Class Flow

Flow instproc init {pattern sourcenode destnode sourceparam fid nodeidx traffic} {
	$self instvar ns_ node_source_ node_dest_ traffic_
	$self instvar agentparam_ transfercnt_ nodeidx_ inter_ class_ fid_
        $self instvar pattern_ logfilefd_ testsuite_ tracefilefd_

	set ns_ [Simulator instance]
	set node_source_ $sourcenode
	set node_dest_ $destnode
	set pattern_ $pattern
        set traffic_ $traffic
        set class_ [$traffic class?]
        set transfercnt_ 0; # number of object to transfer by this flow
        set nodeidx_ $nodeidx
        set logfilefd_ [$pattern_ set logfilefd_]
        set testsuite_ [TestSuite instance]
        set tracefilefd_ [$testsuite_ set paramfilefd_]
        set inter_ 0.0
        set fid_ $fid
        
	instance-parameter $self $sourceparam

	if { [isdebug] } {
			puts "New flow : $fid_"
	}
	# Do you need different source per node in a pattern ?
	# $self new-flow $j $fid [value $nodeindex $agentparam_]
        $self new-flow
}


Flow instproc get-agent {} {
	$self instvar agent_
	return $agent_
}

Flow instproc print {fd index} {
	$self instvar agent_ class_
	puts $fd "\t\tagent class: [$agent_ info class] fid: [$agent_ set fid_] flow index: $index traffic class: $class_" 
}


Class Flow/TCP -superclass Flow 

Flow/TCP instproc new-flow {} {
	$self instvar ns_ agent_ d_agent_ app_
	$self instvar node_source_  node_dest_ agentparam_
	$self instvar tcpversion_ tcpsink_ fid_ packetSize_
	

        # create source and destination agent and connect them
        # Destination agent is a sink that will ack every packet

        # return a list of agent
        set agents [$ns_ create-connection-list $tcpversion_ $node_source_ $tcpsink_ $node_dest_ $fid_]
        set agent_ [lindex $agents 0] ; #source agent
        
        if {[info exists agentparam_]} {
	        instance-parameter $agent_ $agentparam_
        }
        
	set packetSize_ [$agent_ set packetSize_]
        # attach application object to the source agent
        set app_ [$agent_ attach-app FTP]
        set d_agent_  [lindex $agents 1]
        $agent_ set flow_ $self

}


Flow/TCP instproc select-ca {} {
        $self instvar agent_ tcpversion_ congestion_avoidance_

        if {$tcpversion != "TCP/Linux"} {
                return
        }
        puts "select-ca for flow"
        $agent_ select_ca  $congestion_avoidance_
}

# For Dynamique flow only
Flow/TCP instproc conf-short {RVstart {RVsize ""} {transfersize ""}} {
        $self instvar RVstart_ RVsize_ transfersize_ 

        set RVstart_ $RVstart
        set RVsize_ $RVsize
        set transfersize_ $transfersize
}

Flow/TCP instproc reschedule {} {
        $self instvar RVstart_ RVsize_ nodeidx_ inter_ ns_
        $self instvar pattern_  transfercnt_ transfersize_ packetSize_

        set now [$ns_ now]
        set inter_ [$RVstart_ value]
        #set starttime [$pattern_ newstarttime $nodeidx_ $inter]
        set starttime [expr $now + $inter_]
        if {$starttime <= $now} {
                puts "WARNING: Flow/TCP reschedule schedule time is not in the futur"
        }
        if {$RVsize_ == ""} {
                if {$transfertsize_ == ""} {
                        puts "ERROR: no transfert size for TCP"
                } else {
                       set size [value $transfercnt_ $transfersize_]
                }
        } else {
                set size [expr round([$RVsize_ value])]
        	#set size [expr round([$RVsize_ value])*$packetSize_]
	}
        $self schedule-app $starttime $size
        $ns_ at $starttime "$pattern_ StartTransfer $nodeidx_"
}



Flow/TCP instproc schedule-app {starttime transfersize} {
	$self instvar ns_ flowstart_ transfercnt_ size_ fid_
        
        incr transfercnt_ ;# selector for transfer  list
	set flowstart_  $starttime
        set size_       $transfersize
	$ns_ at $flowstart_ "$self start-app"
}

Flow/TCP instproc start-app {} {
        $self instvar ns_ agent_  size_ logfilefd_ fid_ class_ app_ traffic_
        $self instvar node_source_  node_dest_ tracefilefd_ classintera_

        $app_ send $size_
        set tmp [$traffic_ start]
        set fanum [lindex $tmp 0]
        set classintera_ [lindex $tmp 1]
        puts $logfilefd_ "[format %-8.5f [$ns_ now]] START  flow [format %3d $fid_] class $class_ from [format %3d [$node_source_ id]] to [format %3d [$node_dest_ id]] [format %6d $size_] bytes inter [format %.5f $classintera_] when [expr $fanum-1] flows"

       puts $tracefilefd_ "[format %8.5f [$ns_ now]] START $fid_ $class_ [$node_source_ id] $size_ [format %.5f $classintera_]"

}

Flow/TCP instproc reset-agent {fid} {
        $self instvar d_agent_ agent_

        $d_agent_ reset
        $agent_   reset

        $d_agent_ set fid_ $fid
        $agent_ set fid_ $fid
}

# write stats in paramater file at the connection terminaison
Flow/TCP instproc done {lib} {
	$self instvar  pattern_ fid_ testsuite_
	
        # write statistics
        $self printstat  [$pattern_ FinishTransfer $self ] $lib
        # Reinit the agents
        set fid_ [$testsuite_ flow-new]
        $self reset-agent $fid_

}

Flow/TCP instproc printstat {activeflow lib} {
        $self instvar size_ agent_ flowstart_ ns_ classintera_ fid_ logfilefd_ tracefilefd_ pattern_ class_
        $self instvar node_source_  node_dest_

        set now [$ns_ now]
        set delta [expr $now - $flowstart_]

        puts $logfilefd_ "[format %-8.5f $now] FINISH flow [format %3d $fid_] class $class_ from [format %3d [$node_source_ id]] to [format %3d [$node_dest_ id]] [format %6d $size_] bytes inter [format %.5f $classintera_] when $activeflow flows T: [format %.5f $delta] $lib "
        puts $tracefilefd_ "[format %-8.5f $now] FINISH  $fid_ $class_ [$node_source_ id] $size_ [format %.5f $classintera_] [format %.5f $delta]"
}

# The following procedure is called whenever a connection ends
Agent/TCP instproc done {} {
	$self instvar flow_
	$flow_ done [$self getstats]
}

#  return trans-pkts, transm-bytes, retrans-bytes, lastack number
Agent/TCP instproc getstats {} {
        $self instvar ndatapack_ ndatabytes_ nrexmitbytes_ ack_

        return [list "$ndatapack_ $ndatabytes_  $nrexmitbytes_  $ack_ "]
}


Class Flow/UDP -superclass Flow

Flow/UDP instproc new-flow {} {
	$self instvar ns_ agent_  fid_ pattern_
	$self instvar node_source_ node_dest_ agentparam_
        $self instvar app_ trafficgenerator_ trafficparam_
	
        # create source and destination agent and connect them
        # return a list of agent
        set agents [$ns_ create-connection-list UDP $node_source_ LossMonitor $node_dest_ $fid_ ]
        set agent_ [lindex $agents 0] ; #source agent
        $agent_ set d_agent_ [lindex $agents 1] ; #destination agent

        if {[info exists agentparam_]} {
	        instance-parameter $agent_ $agentparam_
        }

        set app_ [$agent_  attach-app $trafficgenerator_]
        if { $trafficgenerator_ == "Traffic/Exponential" || \
              $trafficgenerator_ == "Traffic/Pareto"} {
                set replication [$pattern_ set replication_]
                set randgen [new RNG]
                for {set i 1} {$i < $replication} {incr i} {
                              $randgen next-substream
                }
                $app_ use-rng $randgen
        }                                     
	instance-parameter $app_ $trafficparam_
 }


Flow/UDP instproc schedule-app {starttime {finishtime -1}} {
	$self instvar ns_ agent_ app_ flowstart_ flowfinish_
	
	set flowstart_ $starttime
	set flowfinish_ $finishtime

        $ns_ at $starttime  "$self start-app"
        if {$finishtime >= 0} {
                $ns_ at $finishtime "$app_ stop"
        }
}

Flow/UDP instproc start-app {} {
        $self instvar ns_ agent_  size_ logfilefd_ fid_ class_ app_ traffic_
        $self instvar node_source_  node_dest_ tracefilefd_ classintera_

        $app_ start
        set tmp [$traffic_ start]
        set fanum [lindex $tmp 0]
        set classintera_ [lindex $tmp 1]
        
        puts $logfilefd_ "[format %-8.5f [$ns_ now]] START  flow [format %3d $fid_] class $class_ from [format %3d [$node_source_ id]] to [format %3d [$node_dest_ id]] "

       puts $tracefilefd_ "[format %8.5f [$ns_ now]] START $fid_ $class_ [$node_source_ id]  [format %.5f $classintera_]"

}


