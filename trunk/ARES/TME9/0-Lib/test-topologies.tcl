#
# Copyright (c) 2004-2005 The University of La Reunion - France
# Authors:  F. HARIVELO,
#	    P. ANELLI
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
# Contributed by the SMART Research Group, http://iremia.univ-reunion.fr
#


#puts "#load test-topology.tcl"


#---------------------------------------------------------

Class SkelTopology

SkelTopology instproc init { ns} {
	$self instvar ns_ 
	
	set ns_ $ns
	#$self set-opt $opt
	#array set opt_ $opt


	$self next
}

SkelTopology instproc node? n {
    $self instvar node_
    if [info exists node_($n)] {
	set ret $node_($n)
    } else {
	set ret ""
    }
    return $ret
}

SkelTopology instproc entrynode? {} {
	$self instvar entry_
	return $entry_
}

SkelTopology instproc exitnode? {} {
	$self instvar exit_
	return $exit_
	}

global node

SkelTopology instproc setnode {id hnode} {
global node
	set node($id) $hnode
}

SkelTopology instproc findnode {id} {
    global node
    if [info exists node($id)] {
	set ret $node($id)
    } else {
	set ret ""
    }
    return $ret
}




#--------------------------- NBMA Topologies

Class Bottleneck  -superclass SkelTopology

Bottleneck instproc init {ns testsuite } {
	$self instvar ns_ testsuite_ linkparam_
	
	set testsuite_ $testsuite
	$self next $ns
        $self set linkparam_ {}
}

Class Core -superclass Bottleneck

Core instproc init { ns testsuite routelength corelink } {
	$self instvar ns_ entry_ exit_ lastnum_ queuetype_ queuesize_ linkparam_
	$self instvar node_ testsuite_ bw_ tp_
# routelength: number of hop in core network (start from 1)
# node name start at value 0  	
#   	         	 
#     	        
#    0---1---2 ... N
#    	        	
#   	         
# 	          
#
	$self next $ns $testsuite
	instance-parameter $self $corelink

set lastnum_ [expr $routelength -1]
# Create nodes
for { set i 0} { $i <= $lastnum_ } {incr i} {
	set node_($i) [$ns_ node]
	$self setnode [$node_($i) id] $node_($i)
		
        if { [$testsuite_ nam?] } {
                $node_($i) shape hexagon
                $node_($i) color blue
       }
} 


set entry_ $node_(0)
set exit_ $node_($lastnum_)

# Create output links
# create a full duplex link between nodes with a bandwidth
# and propagation delay. By default access queue to the link is
# managed according DropTail i.e. FIFO.

# Create links
for {set i 0} { $i < $lastnum_ } {incr i} {
	set egressnode [expr $i +1]
	set bottlebw [value $i $bw_]
	set bottletp [value $i $tp_]
	set queuetype [value $i $queuetype_]
	
	$ns_ duplex-link $node_($i) $node_($egressnode) $bottlebw $bottletp $queuetype

 # tell nam how to arrange the nodes
    if { [$testsuite_ nam?] } {
        $ns_ duplex-link-op $node_($i) $node_($egressnode) orient right
        $ns_ duplex-link-op $node_($i) $node_($egressnode) queuePos 0.5
    }

  }  

}


Core instproc print {fd} {
	$self instvar node_ ns_

	puts $fd "Core network"
	set prev {}
	foreach i [array names node_] {
		puts $fd "\t\tnode id: [$node_($i) id] index: $i"
		if {$prev != {}} {
			set link [$ns_ link $prev $i]
			if {$link != {} } {
				puts $fd "\t\t\tbw          : [$link bw]"
				puts $fd "\t\t\tdelay       : [$link delay]"
				set q [$link queue]
				puts $fd "\t\t\tqueue type  : [$q info class]"
				puts $fd "\t\t\tqueue length: [$link qsize]"
			}
		}
		set prev $i
	}

}


Core instproc maxsecpbyte {} {
        $self instvar node_ ns_
        
        set secpby 0.0
        set prev {}
       	foreach i [array names node_] {
        	if {$prev != {}} {
			set link [$ns_ link $prev $i]
			if {$link != {} } {
                		set secpby [expr $secpby + (([$link qsize] - 1.0) / ([$link bw] /8))]
                        } 
                 }
                 set prev $i
        }
        return $secpby
}


Core instproc configure-corelink {} {
	$self instvar lastnum_ node_ ns_ queuesize_ linkparam_

	if {![info exists queuesize_] || ([llength $queuesize_] == 0)} {
		puts "Queue size in core network: [Queue set limit_] packets"
		return
	}

    for {set i 0} { $i < $lastnum_ } {incr i} {
	set egressnode [expr $i +1]

        set size [value $i $queuesize_]
        set linki [$ns_ link $node_($i) $node_($egressnode)]
        set linke [$ns_ link $node_($egressnode) $node_($i)]

        foreach l [list $linki $linke] {
                set q [$l queue]
                #puts "[$q info class]"
                $q set limit_ $size
                # Not valid for FQLink: must be init by default value of Queue
                instance-parameter $q $linkparam_
        }
   }
}


Core instproc enable_traceRED {scopelist fdtracefile } {
	$self instvar  ns_

	if {$fdtracefile == 0} {
		return
	}
	
        set scopename [lindex $scopelist 0]
	if {$scopename  !="RED"} {
		puts "ERROR in [info level [info level]]: called without RED item"
		exit 1
	}
        set redlist [lindex $scopelist 1]
	foreach reditem $redlist {
                set casting [lindex $reditem 0]
	        if {[llength $casting] != 2} {
	  	    puts "ERROR in [info level [info level]]: wrong number of nodes to indicate a queue: $casting"
		    exit 1
	        }
	        set n1id [lindex $casting 0]
	        set n2id [lindex $casting 1]
	
	        set currentlink [$ns_ link [$self findnode $n1id] [$self findnode $n2id]]
	        set redq [$currentlink queue]
                if {[$redq info class Queue/RED]} {
	                $redq attach $fdtracefile
	                $redq trace curq_
	                $redq trace ave_
	        }
	}
}



# create and add an error model at the entry of the core network
#
Core instproc add-error {error} {
        $self instvar ns_ node_

        set nodei  $node_(0)
        set nodee  $node_(1)

        #$error set markecn_ $ecn

        # link-lossmodel: Insert error model in the link 
        #                       (after the queue and link (delay))
        #                 Can be seem by nam (link-lossmodel)
        #                   call SimpleLink  insert-linkloss
        # lossmodel: Insert loss model at the entry before the queue 
        #               call SimpleLink errormodule 
        $ns_ lossmodel $error $nodei $nodee
}



Class Core/BestEffort -superclass Core

Core/BestEffort instproc connect-flow {sourcenode destnode hook listbw listtp queue } {
	$self instvar ns_ node_ testsuite_

		
	set lsourcenode [list $sourcenode $destnode] 
	for {set i 0} {$i < 2} {incr i} { 
	  set corenode [$self node? [value $i $hook]]
	  
	  set bw [value $i $listbw]
	  set tp [value $i $listtp]

          set node  [lindex $lsourcenode $i] 
	  $ns_ duplex-link $node $corenode $bw $tp $queue

         if { [$testsuite_ nam?] } {
           $ns_ duplex-link-op $corenode $node queuePos -0.5
         }

	}

}

Class Core/Diffserv -superclass Core

Core/Diffserv instproc configure-corelink {} {
	$self instvar lastnum_ node_ ns_ testsuite_

	$self next

    for {set i 0} { $i < $lastnum_ } {incr i} {
	set egressnode [expr $i +1]

	$testsuite_ ConfigureCore $node_($i) $node_($egressnode) 
	$testsuite_ ConfigureCore $node_($egressnode) $node_($i) 
    }
}


Class Edge -superclass Bottleneck


Edge instproc init { ns testsuite core linkparam } {
	$self instvar core_  igress_ egress_ ns_ queuetype_ queuesize_ policy_
	$self instvar  testsuite_ bw_ tp_ hook_ entry_ exit_ debug_

#------- internal procedure
proc duplex-link {edgenode corenode bw tp queueigress queueegress} {
	
	set ns [Simulator instance]
	$ns simplex-link $edgenode $corenode $bw $tp $queueigress	
	$ns simplex-link $corenode $edgenode $bw $tp $queueegress
}
#-------
	set core_ $core
	$self next $ns $testsuite
	
	instance-parameter $self $linkparam

	set debug_ [isdebug]
	set queueegress [value 1 $queuetype_]
	
	set index 0
	foreach i "igress_ egress_" {
	  set nameedge ""; set namecore ""
	  append nameedge $i "(edge)"
	  append namecore $i "(core)"
	  set newnode  [$ns_ node]
	  set $nameedge $newnode
	  $self setnode [ $newnode id] $newnode
	  
	  set $namecore [$core node? [value $index $hook_]]

	  set bw [value $index $bw_]
	  set tp [value $index $tp_]

	  if {![info exists queuesize_] || ([llength $queuesize_] == 0)} {
		puts "Queue size in edge $i link: [Queue set limit_] packets"
	  } else {
		set size [value $index $queuesize_]
	        # set queue size in each direction
	        $ns_ queue-limit $newnode [eval set $namecore] $size
	        $ns_ queue-limit [eval set $namecore] $newnode $size
		if {$debug_} {
			puts "queue size $size"
		}
	  }
	  if { $debug_ } {
		puts  "Value $bw and $tp"
	  }	
	  duplex-link $newnode [eval set $namecore] $bw $tp \
	  	      [lindex $queuetype_ 0] $queueegress
	  incr index
        }

	set entry_ $igress_(edge)
	set exit_ $egress_(edge)
}


# QUEUE SIZE TO set in init
# parameter link: enum type (igress, egress)
Edge instproc configure-edgelink {link} {
	$self instvar ns_ igress_ egress_ queuesize_

	set enum {igress egress}
	if { [lsearch $enum $link] == -1} {
		puts "ERROR in configure-edgelink: wrong value in variable link"
		exit 1
	}

}


Edge instproc connect-flow {sourcenode destnode bw tp queue} {
	$self instvar ns_ igress_ egress_
	
	# create link to igress edge node
	$ns_ duplex-link $sourcenode $igress_(edge) [lindex $bw 0] [lindex $tp 0] $queue 
	
	#create link to egress edge node
	$ns_ duplex-link $destnode $egress_(edge) [lindex $bw 1] [lindex $tp 1] $queue 
	
}


Class Edge/Diffserv -superclass Edge

Edge/Diffserv instproc configure-edgelink {link} {
	$self instvar ns_ igress_ egress_ testsuite_

	$self next $link

	append link "_"
	set nameedge $link; append nameedge "(edge)"
	set namecore $link; append namecore "(core)"
	set edgenode [eval set $nameedge]
	set corenode [eval set $namecore]

	$testsuite_ ConfigureCore $edgenode $corenode
 	$testsuite_ ConfigureCore $corenode $edgenode

 	# Configure Policer
 	set queueedge [[$ns_ link $edgenode $corenode] queue]
	$testsuite_ AddPolicer $queueedge
}


Edge/Diffserv instproc AddPolicy {sourcenode destnode cir cbs} {
	$self instvar igress_ egress_ ns_
	
        set cr [bw_parse $cir]
	foreach i {source dest} {
	
	     eval set nodename \$${i}node
	     if {$nodename == "ANY"} { 
		set $i -1
	     } else {
		set $i [$nodename id]
	     }
	}
	
	set queue [[$ns_ link $igress_(edge) $igress_(core)] queue]
        $queue addPolicyEntry $source $dest TokenBucket 10 $cr $cbs

	set queue [[$ns_ link $egress_(edge) $egress_(core)] queue]
        $queue addPolicyEntry $dest $source TokenBucket 10 $cr $cbs
}




#---------------------------  BMA Topologies

Class Topology/Broadcast -superclass SkelTopology

Topology/Broadcast instproc init { ns num opt} {
	$self instvar ns_ num_ node_ nodelist_
	
	$self next $ns $opt
	
	set num_ $num
	
	for {set i 0} {$i < $num_} {incr i} {
		set node_($i) [$ns_ node]
		lappend nodelist_ $node_($i)
	}
}

Class Topology/Lan -superclass Topology/Broadcast

Topology/Lan instproc init { opt ns num bw delay } {
	$self instvar opt_
	$self instvar ns_ nodelist_ network_ 
	
	$self next $ns $num $opt
	
	# Lan-node have a node id. It is the last id generated for a given lan
	set network_ [$ns_ make-lan $nodelist_ $bw $delay $opt_(ll) $opt_(ifq) $opt_(Lanmac) $opt_(Lanchan)]
}

Class Topology/WLan -superclass Topology/Broadcast

Topology/WLan instproc init { opt ns num bw delay } {
	$self instvar ns_ nodelist_ network_ num_ node_
	
	array set opt_ $opt
	# setup topography object
	set topo [new Topography]
	# define topology
	$topo load_flatgrid $opt_(x) $opt_(y)
	# Create God
	set god [create-god [expr $num]]
	#global node setting
	$ns node-config -adhocRouting $opt_(adhocRouting) \
                 -llType $opt_(ll) \
                 -macType $opt_(WLanmac) \
                 -ifqType $opt_(ifq) \
                 -ifqLen $opt_(ifqlen) \
                 -antType $opt_(ant) \
                 -propType $opt_(prop) \
                 -phyType $opt_(netif) \
                 -channel [new $opt_(WLanchan)] \
		 -topoInstance $topo \
		 -agentTrace ON \
                 -routerTrace OFF \
                 -macTrace OFF
	
	$self next $ns $num $opt
	
	# Lan-node have a node id. It is the last id generated for a given lan
	set network_ [$ns_ make-wlan $nodelist_ $bw $delay $god]
	
	if { $opt_(randomMovement) == 0} {
		for {set i 0} {$i < $num_} {incr i} {
			$node_($i) random-motion 0;# disable random motion
		}
	}
	
	$self setup-nodes-location
}

# Create regular polygon location with the first node in the center
Topology/WLan instproc setup-nodes-location { } {
	$self instvar ns_ node_ num_ opt_
	
	#setup nodes location
	$node_(0) set X_ $opt_(polyCenterX)
	$node_(0) set Y_ $opt_(polyCenterY)
	$node_(0) set Z_ 0
	
	$ns_ initial_node_pos $node_(0) $opt_(initialPos)
	
	set PI [expr acos(-1)]
	set step [expr 2 * $PI / [expr $num_ - 1]]
	
	for { set i 0 } { $i < [expr $num_ - 1]} { incr i } {
		set angle [expr $opt_(polyFirstAngle) + $i * $step]
		set x [expr $opt_(polyCenterX) + $opt_(polyRadius) * cos($angle)]
		set y [expr $opt_(polyCenterY) + $opt_(polyRadius) * sin($angle)]
		set z 0.000000000000
		
		$node_([expr $i + 1]) set X_ $x
		$node_([expr $i + 1]) set Y_ $y
		$node_([expr $i + 1]) set Z_ $z
		
		if { [expr $i + 1] == $opt_(mvtNodeId) } {
			set initialX $x
			set initialY $y
		}
		
		$ns_ initial_node_pos $node_([expr $i + 1]) $opt_(initialPos)
	}
}

Class Topology/WLanMobile -superclass Topology/WLan

Topology/WLanMobile instproc init { opt ns num bw delay } {
	$self next $opt $ns $num $bw $delay
}

# Create regular polygon location with the first node in the center
Topology/WLanMobile instproc setup-nodes-location { } {	
	$self instvar ns_ node_ opt_
	
	$self next
	
	#setup nodes movement
	if { $opt_(mvtNodeId) >= 0 } {
		$ns_ at $opt_(mvtStart) "$node_($opt_(mvtNodeId)) setdest $opt_(mvtX) $opt_(mvtY) $opt_(mvtSpeed)"
	}
	#$ns_ at $opt_(mvtStart2) "$node_($opt_(mvtNodeId)) setdest $initialX $initialY $opt_(mvtSpeed)"
}






#-----------------------------------------------#
# To deal with the good LAN (Ethernet or Wi-Fi)


# Base class of network
Class SkelNetwork

#SkelNetwork instproc init {ns nodes bw delay estType}
SkelNetwork instproc init {ns nodes bw delay} {
	$self instvar ns_ nodes_ nbw_ ndelay_
	
	set ns_ $ns
	set nodes_ $nodes
	
	# Bandwidth of overall network
	set nbw_ [$ns_ bw_parse $bw]
	# Delay introduced by LL layer
	set ndelay_ [$ns_ delay_parse $delay]
}

Class Network/Lan -superclass SkelNetwork

Network/Lan instproc init { ns nodelist bw delay llType ifqType macType chanType } {
	$self instvar ns_ lanNode_
	
	$self next $ns $nodelist $bw $delay
	
	if {[string compare $macType "Mac/Csma/Cd"] == 0} {
		puts "Warning: Mac/Csma/Cd is out of date"
		puts "Warning: Please use Mac/802_3 to replace Mac/Csma/Cd"
		set macType "Mac/802_3" 
        }
	
	set lanNode_ [new LanNode $ns -bw $bw -delay $delay -llType $llType -ifqType $ifqType -macType $macType -chanType $chanType]
	
	$lanNode_ addNode $nodelist $bw $delay $llType $ifqType $macType
	
	set nif [$lanNode_ set lanIface_($node)]
	set tr [$ns_ get-ns-traceall]
	
	if {$tr != ""} {
		$nif trace-drop $ns_ $tr
	}
	
	# TO DO Activate drop trace for nam
}

Class Network/WLan -superclass SkelNetwork

Network/WLan instproc init { ns nodelist bw delay god } {
	$self instvar god_
	
	$self next $ns $nodelist $bw $delay
	
	set god_ $god
	
	# TO DO: Make medium capacity variable
	
# 	puts "Basic : [Mac/802_11 set basicRate_]"
# 	puts "Data : [Mac/802_11 set dataRate_]"
	
	foreach node $nodelist {
		set tr [$ns get-ns-traceall]
		if {$tr != ""} {
			$node trace-drop $ns
		}
		
		puts [[$node set ragent_] info class]
	}
}


