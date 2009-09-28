#
# Copyright (c) 2006 University of La Reunion - France
# Author:   Pascal ANELLI
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

#  This test suite is for master labs
# to run an individual test :
# ns test-suite-master.tcl <test> -f <ressourcefile>
# ....
#
# To view a list of available tests to run with this script:
# ns test-suite-master.tcl
#


#puts "   Read script file: [info script]"
#====================
# load common library
#====================
set currentdir [pwd]
catch "cd $env(LIBSUITE)"

source test-suite-template.tcl
catch "cd $currentdir"



#================================
# Parameter given by command line
#================================
# replication stoptime wd nam verbose quiet


#========================
# set default parameters
#========================

remove-all-packet-headers       ; # removes all except common
add-packet-header Flags IP TCP RTP  ; # hdrs reqd for TCP

# Default parameters for TCP
Trace set show_tcphdr_ 0
Queue set limit_ 50
Agent/TCP set tcpTick_ 0.1
Agent/TCP set rfc2988_ false
Agent/TCP set useHeaders_ false
Agent/TCP set windowInit_ 1
Agent/TCP set singledup_ 0
Agent/TCP set syn_ false
Agent/TCP set delay_growth_ false
Agent/TCP set minrto_ 1
Agent/TCP set bugFix_ false 
Agent/TCP/Newreno set newreno_changes1_ 0
Agent/TCPSink/DelAck set interval_ 500ms
Agent/TCP set maxburst_ 0
Agent/TCP/RFC793edu set syn_ false


set opt(stoptime) 20
set opt(verbose)  1


#=======================================
##           BASE CLASS            ##
#=======================================

Class Test/Simple -superclass TestSuite

Test/Simple instproc init {} {
	$self instvar net_ opt_ corenetwork_ patterntype_
	global conf

	set net_ "BestEffort"
	set patterntype_ Classic

	$self next

	# create topology class
	$self create-topology
	
        # configure queue size
	$corenetwork_ configure-corelink
    	
        # Create traffic source
    	$self create-pattern $opt_(pattern) $opt_(replication)

	# set specific measurments
#	set paramlist [lindex $conf(param) {}]
        $self post-process $conf(param)
	$self tracevar

	# RED and on line queue tracing
	$self QueueMonitor
}


Test/Simple instproc finish {} {

        $self next

        puts "\n>>>>>>>>>>>>>>>> Don't forget to read the logs  <<<<<<<<<<<<<<<<<<<<" 
        exit 0
}


# TO DO
 TestSuite instproc tcpDumpAll { tcpSrc interval fid } {
    $self instvar dump_inst_ ns_
    
    if ![info exists dump_inst_($tcpSrc)] {
        set dump_inst_($tcpSrc) 1
        set report $fid/window=[$tcpSrc set window_]/packetSize=[$tcpSrc set pac
ketSize_]
        puts $report
        $ns_ at 0.0 "$self tcpDumpAll $tcpSrc $interval $fid"
        return
    }
    
    $ns_ at [expr [$ns_ now] + $interval] "$self tcpDumpAll $tcpSrc $interval $f
id"
    set report time=[$ns_ now]/class=$fid/ack=[$tcpSrc set ack_]/packets_resent=
[$tcpSrc set nrexmitpack_]
    puts $report
}

#==========================================
##           OVERLOADED FUNCTIONS        ##
#==========================================

FQLink  instproc new-flow { src dst fid } {
        $self instvar classifier_ nactive_ queue_ link_ drpT_ 
        $self instvar queueManagement_ fq_

        incr nactive_ 

        set fq_ $queue_
        set limit_ [$fq_ set limit_]
        
        if {![info exists queueManagement_]} {
                set queueManagement_  [$class set queueManagement_]
        }
        set q [new Queue/$queueManagement_]
        $q set limit_ $limit_
        
        $q drop-target $drpT_

        set slot [$classifier_ installNext $q]
        $classifier_ set-hash auto $src $dst $fid $slot
        $q target $queue_
        $queue_ install $fid $q
}


#==========================================
##           SPECIALIZED CLASSES         ##
#==========================================


Class Test/BestEffort -superclass Test/Simple

Test/BestEffort instproc pkt_list {todo pkts} {
        
        set lerror [$self find-error $todo]
        foreach item $lerror {
                   #puts "item: [$item set todo_]"           
            if {[$item get-type] == "List"} {
                   set emod [$item get-emod]
                   #puts "emod: [$emod info class]"                
                   $emod droplist $pkts
           }
        }
}


Test/BestEffort instproc find-error {todo} {
        $self instvar re_

        set lerror {}
        foreach re $re_ {
                if {[$re todo? $todo]} {
                        lappend lerror $re 
                }
        }
        return $lerror
}


Class Test/ErrorAck -superclass Test/BestEffort

Test/ErrorAck instproc create-pattern {pattern replication} {
	$self  instvar pattern_ 

	$self next $pattern $replication
	
	for {set i 0} {$i <[llength $pattern]} {incr i} {
		$pattern_($i) add-error 1 0
	}
}


Class Test/Error -superclass Test/BestEffort

Test/Error instproc create-pattern {pattern replication} {
	$self  instvar pattern_ 

	$self next $pattern $replication
	
	for {set i 0} {$i <[llength $pattern]} {incr i} {
		$pattern_($i) add-error 0 0
	}
}


Class Test/TBF -superclass Test/Simple

Test/TBF instproc create-pattern {pattern replication} {
	$self instvar pattern_ 
	
	$self next $pattern $replication

	for {set i 0} {$i <[llength $pattern]} {incr i} {
                # shaper to true (use field shaper of pattern) 
		$pattern_($i) add-tbf true 0 1
                # shaper to false = policer (use field policer of pattern)
                $pattern_($i) add-tbf false 0 0
	}
}


Class Test/tahoe -superclass Test/Error

Test/tahoe instproc init {} {
        $self instvar tcpversion_ guide_

        set tcpversion_ TCP
        set guide_ "Tahoe with 1 spurious retransmit then 2 CE"
        Agent/TCP set singledup_ 1

        $self next
        $self pkt_list drop { 6 9 12 14}
}


#==============================
#            load Main
#==============================
source "$env(LIBSUITE)/test-main.tcl"

