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

# Options of post-process trace file
# getrc:
#  	Perl script to extract trace lines that match certain characteristics.
#	-s node1	extract all packets from node1.
#	-b -s node1	extract all packets to or from node1.
#	-s node1 -d node2	extract packets on link <node1 -> node2>
#	-b -s node1 -d node2	extract pkts. on link <node1 <-> node2>
#
# raw2xg:
#       Perl script to translate the trace file into a format plotable to
# xgraph. 
#    -a		plot acks
#    -s SCALE    scale TCP sequence numbers by SCALE
#    -n FACTOR	scale flow number by FACTOR
#    -m MODULUS  treat TCP sequence numbers as mod MODULUS
#    -q		show queueing delay by connecting lines
#    -l		show packet length
#    -t TITLE    title of test
#    -e		plot ecn flags
#    -d		plot only drops and ECN marks
# Results:
#        The x-axis shows the time in seconds. The y-axis shows the packet number mod 90. 
# There is a mark on the graph for each packet as it arrives and departs from the congested
# gateway and a "x" for each packet dropped by the gateway.
#       Some of the graphs show more that one active connection. In this case, packets numbered 1 to 90 on the y-axis belong to the first connection, packet numbered 101 to 190 on the y-axis belong to second connection and so on.

#puts "#load test-metric.tcl"
source global.tcl
source test-math.tcl


# TO DO 
#      check noclobber option to false in order to avoid error in redirection
#      print the RTT and RTT with tp only
#      fairness
#      latency (file transfert)
#      Agregate window

#==========================================================================
Class Metric
#==========================================================================

Metric instproc init {test parameters metrics verbose} {
	$self instvar metrics_ test_ log_ verbose_
	$self instvar suffixpkt_ suffixdata_ suffixparam_
	$self instvar GETRC_ RAW2XG_
	
	set metrics_ $metrics
	set test_ $test
	set log_ ""
        set verbose_ $verbose

	set suffixpkt_ [GetSuffix pkt]
	set suffixdata_ [GetSuffix data]
	set suffixparam_ [GetSuffix param]

	#set the trace file name: filenamepkt_, filenameparam_
	$self set filenamepkt_ $test.$suffixpkt_
	$self set filenameparam_ $test.$suffixparam_

	$self default-parameter
	# set intance variable from conf array to self
	instance-parameter $self $parameters

	set path [glob -nocomplain "$GETRC_"] 
	if {[llength $path] > 0} {
		set GETRC_ $path
	}

	set path [glob -nocomplain "$RAW2XG_"] 
	if {[llength $path] > 0} {
		set RAW2XG_ $path
	}
}

Metric instproc default-parameter {} {
	$self instvar AWK_ PERL_ GETRC_ RAW2XG_ SFID_
        global env
        
	$self set timeinterval_ 1.0; # tsw and rate
	$self set weightlength_ 1.0/500; # Queue length
	$self set weight_ 1.0/32 ;# EWMA
	$self set timetransit_ 0; # time before steady state
	$self set sizesampledrop_ 1; # Drop ratio (number of sending packets)
	$self set accuracy_ 1.0/100; # pdf
	$self set bottlebw_ 1Mb; # to normalize rate
	$self set floating_ 20; # Floating average
	$self set modulo_ 1000; # counting and sequence number
	$self set wrap_ 90; # seqnum modulo for activity
        $self set scale_ 0.01; # TCP seq num
        $self set sizesample_ 1; # number of packet to estimate the rate

	set AWK_ "awk"
	set PERL_ "perl"
	set GETRC_ "$env(NS2)/getrc"
	set RAW2XG_ "$env(NS2)/raw2xg"
	set SFID_ "$env(NS2)/set_flow_id"
}

# call procedure to make an appropriate datafile
Metric instproc MakeData {} {
	$self instvar metrics_ log_ verbose_

	foreach i $metrics_ {
                if {$verbose_} {
		        puts "     Computing $i"
		}
                set datafile [$self $i ]
		lappend log_ [list $i $datafile]
	}
	puts "     Result: $log_"
	return $log_
}


# Return a temporary filename
Metric instproc GetFileName {lib suffix} {
	$self instvar test_ 

	set inputfile ""
	set scope [$self FindScope] 
	set timestamp [$self TimeStamp] 
	
    while {$inputfile == ""}  {
   	append inputfile $test_ $scope $lib $timestamp "." $suffix
        if {![file exists $inputfile]} {
    		break
        }
	set inputfile ""
	incr timestamp 	
    }
    exec touch $inputfile
    return $inputfile
}
    

# Return the subclass name of Metric
Metric instproc FindScope {} {
    set name [$self info class]
    set scope [lindex [split $name /] end]

   return $scope
}


# Return a timestamp on 3 digit 
Metric instproc TimeStamp {} {

	set time [clock seconds]
	set len [string length $time]
	set num [string range $time [expr $len -3] $len]
	while {[string index $num 0] == "0"} {
		set num [string range $num 1 end]
	}
	if {$num == ""} { set num 0}
	return $num
}


# Evaluate the fid list and return a list. The final list contains an id for each fid
Metric instproc exlfid {lfid} {

	set fid ""
	foreach item $lfid {
		if {[lsearch -regexp $item "-"] != -1} {
			set range [split $item "-"]
			for {set i [lindex $range 0]} {$i <= [lindex $range 1]} {incr i} {
				lappend fid $i
			}
		} else {
		        lappend fid $item
		}
	}
	return $fid
}

# Return fid list from packet trace file
Metric instproc listfid {tracefile} {

	set tmp [exec sort -n -k 8 -k 2  $tracefile | cut -d " " -f 8 | uniq ]
	return [split $tmp "\n"]

}

# Filter Packet trace file according a fid list
Metric instproc InputFile {lfid} {
$self instvar suffixpkt_ filenamepkt_ AWK_
 	
    set inputfile [$self GetFileName "all" $suffixpkt_ ]	
    foreach i $lfid {
        exec $AWK_  { $8 == fid { print $0 } } fid=$i $filenamepkt_ >>$inputfile
    }
    return $inputfile
}


# keep trace for a particular fid in packet trace file
# result is put in outputfile
Metric instproc filterflow {inputfile fid outputfile} {
	$self instvar AWK_
	
set awk_code {
 BEGIN {
		# field numbers
		FEVENT=1 ; FTIME=2
		FINGRESS=3 ; FEGRESS=4
		FPTYPE=5 ; FPLEN=6
		FFLAG=7 ; FFID=8
		FSA=9 ; FDA=10
		FSEQ=11 ; FUID=12
	}
$FFID == fid {
	print $0
	}
}
	exec $AWK_ $awk_code fid=$fid $inputfile >> $outputfile
}


# Keep direction : source -> sink 
Metric instproc Flowsimplex {inputfile} {
	$self instvar AWK_
	
set awk_filter {
 BEGIN {
 	FEVENT=1 ; FTIME=2
        FINGRESS=3 ; FEGRESS=4
        FPTYPE=5 ; FPLEN=6
        FFLAG=7 ; FFID=8
        FSA=9 ; FDA=10
        FSEQ=11 ; FUID=12
        
        currentfid=-1;
 }
    {
        # new FID, Init state variable for new FID
         if ($FFID != currentfid) {
           destid= $FDA;
           currentfid=$FFID
	   }
	 if (destid == $FDA) {
	 	print $0
	}
   }
}
	set simplexfile [file rootname $inputfile]
	append simplexfile "simp" "[file extension $inputfile]"
	exec sort -n -k 8 -k 2 $inputfile | $AWK_ $awk_filter > $simplexfile
	if {[file size $simplexfile] == 0} {
		puts "ERROR: file size null for  $simplexfile"
		exit 1
	}
	return $simplexfile
}

# Activate Metric per agregate (the flow list is seen like a single flow)
Metric instproc OverAll {} {
	$self instvar flowgranularity_ mark_

	set mark_ 1; # for Drop Event

	set flowgranularity_ OverAll
}

# Activate Metric per flow 
Metric instproc PerFlow {} {
	$self instvar flowgranularity_
	
	set flowgranularity_ PerFlow
}
	
# Do an action on Per flow based
Metric instproc DoPerFlow {todo inputfile outputfile} {
	$self instvar flowlist_ AWK_ mark_

	if {![info exists flowlist_]} {
		puts "ERROR: flowlist in DoPerFlow doesn't exist in class [$self info class]"
		exit 1
	}

	set tmp "tmp.data"
	foreach flow $flowlist_ {
		catch "exec rm $tmp; touch $tmp"
		$self filterflow $inputfile  $flow $tmp
		set ptype [exec head -n 1 $tmp | $AWK_ {{print $5}} ]
		set source [exec head -n 1 $tmp | $AWK_ {{print $9}} ]
		exec echo "\"$ptype\t$flow\t$source\"" >>$outputfile
		set mark_ $flow ;# for Drop event metric
		$self $todo $tmp $outputfile
	}
}

# launch the awk script 
Metric instproc DoMetric {todo inputfile outputfile} {
	$self instvar flowgranularity_ flowlistlib_ AWK_

	if {$flowgranularity_ == "OverAll"} {
    		set ptype [exec head -n 1 $inputfile | $AWK_ {{print $5}} ]
                exec echo "\"*\t$flowlistlib_\t-\"" >>$outputfile
		$self $todo $inputfile $outputfile
        } else {
		$self DoPerFlow $todo $inputfile $outputfile
	}
}


#------------------------------------------------------
#                  M E T R I C
#------------------------------------------------------
# Applied on packet trace file
 
 # To filter trace with AWK on a specific FID
 # if ( FID != "" && FID != $FFID )
 #                next
 
Metric instproc awk_delay {inputfile outputfile} {
        $self instvar AWK_ timetransit_

	# Input format: Generic packet trace
set awk_code {
        BEGIN {
                # Field numbers
                FEVENT=1 ; FTIME=2
                FINGRESS=3 ; FEGRESS=4
                FPTYPE=5 ; FPLEN=6
                FFLAG=7 ; FFID=8
                FSA=9 ; FDA=10
                FSEQ=11 ; FUID=12
        }
/^\+/	{ 
	 if ( $FINGRESS != int($FSA) )
                        next
                        
         pktid = $FSEQ
         sdtime = $FTIME
        }
/^r/	{
         if ( $FEGRESS != int($FDA) )
                                next
         if ($FSEQ = pktid) {
                        delay = $FTIME-sdtime
                        if ((delay > 0) && (sdtime >= startt)) {
                                        print sdtime, delay
                        }
          }
	}
END { printf "\n\n"
      }
}
	exec sort -n -k 12 -k 2 $inputfile | $AWK_ $awk_code startt=$timetransit_ >>$outputfile
}


Metric instproc awk_latency {inputfile outputfile} {
	$self instvar AWK_ 
	# Input format: Generic packet trace
set awk_code {
        BEGIN {
                # Field numbers
                FEVENT=1 ; FTIME=2
                FINGRESS=3 ; FEGRESS=4
                FPTYPE=5 ; FPLEN=6
                FFLAG=7 ; FFID=8
                FSA=9 ; FDA=10
                FSEQ=11 ; FUID=12
                starttime = -1
        }
/^\+/	{ 
	 if ( $FINGRESS != int($FSA) )
                        next
         if (starttime == -1) {
	 	fid= $FFID
	 	starttime = $FTIME
	 }
	}
/^r/	{
	 if ( $FEGRESS != int($FDA) )
                                next
	 lasttime = $FTIME
	}
END	{
	  print fid, lasttime, (lasttime - starttime)
	}
}
	exec sort -n -k 2 $inputfile | $AWK_ $awk_code >>$outputfile
}


#       x axis: time
#       y axis: drop ratio
#        # drop/ # sending packet per samplesize
Metric instproc awk_DropRatio {inputfile outputfile} {
        $self instvar sizesampledrop_ AWK_ timetransit_
        
        # input format: Generic packet trace
set awk_code {
        BEGIN {
                FEVENT=1 ; FTIME=2
                FINGRESS=3 ; FEGRESS=4
                FPTYPE=5 ; FPLEN=6
                FFLAG=7 ; FFID=8
                FSA=9 ; FDA=10
                FSEQ=11 ; FUID=12
                        
                sample = 0;
                dropitem=0
		init=1
		
        }
$FTIME <startt {
		next
		}
/^\+/	{
          if ($FINGRESS== int($FSA))  {
	  	if (init) {
			print $FTIME, 0
			init =0
		}
	  	sample = sample+1;
                if (sample == smooth) {
                	drop = dropitem*1.0/sample;
                        print $FTIME, drop;
                        predtime = $FTIME;
                        dropitem = 0;
                        sample = 0;
               }
          }
	}
/^d/	{
          dropitem += 1;
        }
END { printf "\n\n" }
}
       exec sort -n -k 2 $inputfile | $AWK_ $awk_code startt=$timetransit_ smooth=$sizesampledrop_  >> $outputfile

}


Metric instproc awk_DropEvent {inputfile outputfile} {
        $self instvar AWK_ timetransit_ mark_
        
        # Input format: Generic packet trace
set awk_code {
        BEGIN {
                        FEVENT=1 ; FTIME=2
                        FINGRESS=3 ; FEGRESS=4
                        FPTYPE=5 ; FPLEN=6
                        FFLAG=7 ; FFID=8
                        FSA=9 ; FDA=10
                        FSEQ=11 ; FUID=12
	 }
$FTIME <startt {
		next
		}
/^d/	{
         print $FTIME, mark;
        }
END { printf "\n\n" }
}
        exec sort -n -k 2 $inputfile | $AWK_ $awk_code startt=$timetransit_ mark=$mark_ >>$outputfile
}


#Counting the overall drop precedence
Metric instproc awk_DropPrecedenceT {inputfile outputfile} {
	$self instvar AWK_ timetransit_ modulo_

# Input format: Generic packet trace
set awk_code {
	function pline (time, eventrow) {
		printf ("%g\t", time);
		for (i=1; i< eventrow ; i++) {
			printf "\t"
		}
		printf "1\n"
	}
	BEGIN {
               FEVENT=1 ; FTIME=2
               FINGRESS=3 ; FEGRESS=4
               FPTYPE=5 ; FPLEN=6
               FFLAG=7 ; FFID=8
               FSA=9 ; FDA=10
               FSEQ=11 ; FUID=12
	       currentuid= -1;
	       nbpacket= 1;
	       nboutpacket=2;
	       nbdropoutpacket=3;
	       nbdropinpacket=4;
	 }
$FTIME <startt {
		next
		}
/^\-/	{
	  if ($FUID != currentuid) {
		pline($FTIME, nbpacket);
		currentuid = $FUID;
		marked = 0;
                }
	  if ((substr($FFLAG, 2, 1) == "P") && (!marked)) {
		pline($FTIME, nboutpacket);
		marked =  1
	       }
	}
/^d/	{
	   if ((substr($FFLAG, 2, 1) == "P")) {
		pline($FTIME,nbdropoutpacket);
	   }
	   if ((substr($FFLAG, 2, 1) == "-")) {
		pline($FTIME,nbdropinpacket);
	   }
	}
}

set awk_count {
	BEGIN {
		FTIME = 1
		nd= 0;
	}
$2 == 1	{
	 print $FTIME, (++nb % modulo)
	}
END {printf "\n\n" }
}

	set tmp "tmp.data"
        catch "exec rm $tmp; touch $tmp"
        exec sort -n -k 12 -k 2 $inputfile | $AWK_ $awk_code startt=$timetransit_ >>$tmp

	set i 2
	foreach compt {nbpacket nboutpacket nbdropoutpacket nbdropinpacket} {
	     exec echo "\"$compt\"" >>$outputfile
	     exec sort -n -k 1 $tmp | cut -f 1,$i | $AWK_ $awk_count modulo=$modulo_ >>$outputfile
	     incr i
	}
	exec  echo "\"nbinpacket\"" >>$outputfile
        exec sort -n -k 1 $tmp | $AWK_ {BEGIN {FTIME=1;FPKT=2; FOUT=3; nb=0; nbout=0; FS= "\t"}
	                               $FPKT == 1 {nb++; print $FTIME, ((nb-nbout)%modulo); next}
				       $FOUT == 1 {nbout++; print $FTIME, ((nb-nbout)%modulo) ;}
				       END{printf "\n\n"}} modulo=$modulo_ >>$outputfile
}


# print the total drop ratio over the time
Metric instproc awk_DropratioT {inputfile outputfile} {
	        $self instvar AWK_ timetransit_

# Input format: Generic packet trace
set awk_code {
        function pline (time, nlost, seqno) {
		if (seqno == 0) {
			return
		}
		Tdroprate = nlost*100/seqno;
        	printf("%g\t%3.3f\n", time, Tdroprate);
 	}
BEGIN {
       FEVENT=1 ; FTIME=2; FINGRESS=3 ; FEGRESS=4; FPTYPE=5 ; FPLEN=6;
       FFLAG=7 ; FFID=8; FSA=9 ; FDA=10; FSEQ=11 ; FUID=12
                        
        nlost=0;
        seqno=0;
	init=1
        }
$FTIME <startt {
		next
		}
/^d/	{
         nlost++;
	 pline($FTIME, nlost, seqno);
	 lasttime=$FTIME
	 next
     	}
/^\+/	{
         seqno++;
	}
	{
	 lasttime =$FTIME
	 if (init) {
	 	print $FTIME, 0
		init =0
	  }
	}
END { 
	pline(lasttime, nlost, seqno);
	printf "\n\n"
    }
}
	exec sort -n -k 2 $inputfile | $AWK_ $awk_code startt=$timetransit_ >>$outputfile
}

# rate over the time according to a jumping time window
Metric instproc awk_rate {inputfile outputfile} {
	$self instvar AWK_ maxbytes_ timeinterval_ timetransit_

	# Input format: Generic packet trace
set awk_code {
	BEGIN { 
		FEVENT=1 ; FTIME=2
		FINGRESS=3 ; FEGRESS=4
		FPTYPE=5 ; FPLEN=6
		FFLAG=7 ; FFID=8
		FSA=9 ; FDA=10
		FSEQ=11 ; FUID=12
			
		slot=startt*1.0;
		size = 0.0;
		init=1;
		}
$FTIME < startt {
                next
                }
	{
	 if (init) {
	 	print $FTIME, 0
		init=0
	 }
	 while ($FTIME >= slot+timeinterval) {
		 slot +=timeinterval
		 print slot, size/timeinterval
		 size = 0.0
	 }
	 size+= $FPLEN*1.0/bw;
	}
END { printf "\n\n"}
}
    	
    exec sort -n -k 2 $inputfile | $AWK_ $awk_code startt=$timetransit_ timeinterval=$timeinterval_ bw=$maxbytes_  >> $outputfile
}


Metric instproc awk_rate_sustainable {inputfile  outputfile} {
	$self instvar maxbytes_ timetransit_ AWK_ timeinterval_
	
	# Input format: Generic packet trace
set awk_code {
        function pline (time, num, deno) {
		if (deno > 0) {
		     ratio = num/deno;
		} else {
		     ratio = 0
		}
        	printf("%g\t%3.2f\n", time, ratio);
 	}

   BEGIN { 
	FEVENT=1 ; FTIME=2
	FINGRESS=3 ; FEGRESS=4
	FPTYPE=5 ; FPLEN=6
	FFLAG=7 ; FFID=8
	FSA=9 ; FDA=10
	FSEQ=11 ; FUID=12

        init = -1;
	slot = startt*1.0;
	size = 0.0;
	}

$FTIME < startt {
		next;
	}	
	{
	 if (init) {
                init=0
		print $FTIME, 0
                starttime=$FTIME
	 }
  	 while ($FTIME >= slot+timeinterval) {
		 slot +=timeinterval
                # if ((slot-starttime) >= timeinterval)
		pline(slot, size, ($FTIME-starttime));
	 }
	
         lastpkt= $FPLEN*1.0/bw;
	 size+= lastpkt;
	 lasttime = $FTIME;
        }
END {
  	pline(lasttime, (size-lastpkt), (lasttime-starttime));
	print "\n\n"
    }
}

	exec sort -n -k 2 $inputfile | $AWK_ $awk_code bw=$maxbytes_ startt=$timetransit_ timeinterval=$timeinterval_  >>$outputfile
}



# Throughput over the time according to a Time Sliding Window
Metric instproc awk_throughputTSW {inputfile outputfile} {
	$self instvar timetransit_ AWK_ maxbytes_ timeinterval_ 

set awk_code {
	BEGIN { 
		FEVENT=1 ; FTIME=2
		FINGRESS=3 ; FEGRESS=4
		FPTYPE=5 ; FPLEN=6
		FFLAG=7 ; FFID=8
		FSA=9 ; FDA=10
		FSEQ=11 ; FUID=12
			 
		currentfid = -1
	     }
$FTIME < startt {
                next
                }
	{
		if (currentfid == -1) {
			       arate=0;
				currentfid=$FFID;
				t_front = $FTIME;
		}
		windowsize = arate*timeinterval
		arate = ((($FPLEN/maxbytes)+windowsize)/ ($FTIME - t_front+timeinterval))
		print $FTIME, arate
		t_front= $FTIME
	}
END { printf "\n\n" }
}
	exec sort -n -k 2 $inputfile | $AWK_ $awk_code startt=$timetransit_  maxbytes=$maxbytes_ timeinterval=$timeinterval_ >>$outputfile

}

#Throughput over the time according Altman-Lochin Algo (variable size time interval)
Metric instproc awk_rate_time {inputfile outputfile} {
	$self instvar AWK_
	$self instvar timeinterval_ maxbytes_ timetransit_

	# input format : Generic packet trace
set awk_code {
  BEGIN { 
	FEVENT=1 ; FTIME=2
	FINGRESS=3 ; FEGRESS=4
	FPTYPE=5 ; FPLEN=6
	FFLAG=7 ; FFID=8
	FSA=9 ; FDA=10
	FSEQ=11 ; FUID=12

	steptime = -1;
	}
	
$FTIME < startt {
		next;
	  }

	{
	  if (steptime == -1) {
	  	steptime = int($FTIME);  
		beginperiod = $FTIME
		recvsize = 0.0
		size =$FPLEN
		next
   	  }
	  recvsize += size
	  size= $FPLEN;
	  if (($FTIME-steptime) > timeinterval) {
	  	period = $FTIME - beginperiod
		rate = recvsize /(period*bw);
		print beginperiod, rate
		beginperiod= $FTIME;
		recvsize =0.0;
		steptime+=timeinterval
         }
    }
END { printf "\n\n"}
}


exec sort -n -k 2 $inputfile | $AWK_ $awk_code timeinterval=$timeinterval_ bw=$maxbytes_ startt=$timetransit_ >> $outputfile
}


Metric instproc awk_rate_size {inputfile outputfile} {
	$self instvar AWK_
	$self instvar sizesample_ maxbytes_ timetransit_

set awk_code {
  BEGIN { 
	FEVENT=1 ; FTIME=2
	FINGRESS=3 ; FEGRESS=4
	FPTYPE=5 ; FPLEN=6
	FFLAG=7 ; FFID=8
	FSA=9 ; FDA=10
	FSEQ=11 ; FUID=12

	beginperiod = -1;
	}
	
$FTIME < startt {
		next;
	  }

	{
	  if (beginperiod == -1) {
		beginperiod = $FTIME
		size =$FPLEN
                sample= 1
		next
   	  }
	  if (sample >= smooth) {
                 if ($FTIME > beginperiod) {
	  	        period = $FTIME - beginperiod
		        rate = size /(period*bw);
		        print beginperiod, rate
                }
		beginperiod= $FTIME;
		size =0.0;
                sample= 0;
	  }
          size=$FPLEN
          sample++
      }
END { printf "\n\n"}
}

exec sort -n -k 2  -k 12 $inputfile | $AWK_ $awk_code smooth=$sizesample_ bw=$maxbytes_ startt=$timetransit_ >> $outputfile

}


Metric instproc awk_goodput {inputfile outputfile} {
	$self instvar maxbytes_ timetransit_ AWK_ timeinterval_

	# Input format: Generic packet trace
set awk_code {
   BEGIN { 
	FEVENT=1 ; FTIME=2
	FINGRESS=3 ; FEGRESS=4
	FPTYPE=5 ; FPLEN=6
	FFLAG=7 ; FFID=8
	FSA=9 ; FDA=10
	FSEQ=11 ; FUID=12
	totalsize= 0.0;
	slot =startt;
	}
	
$FTIME < startt {
		next;
	  }
/^r/ {
	if (($FPTYPE == "ack") && ($FEGRESS == int($FDA)) && !($FFID in newack)) {
		newack[$FFID]=$FSEQ;
		print $FTIME, 0
		next
        }
	if (($FPTYPE == "tcp") && ((($FEGRESS == int($FDA))|| ($FINGRESS == int($FSA))))) {
		packetsize[$FFID]= $FPLEN;
		next
	}
	while ($FTIME >= slot+timeinterval) {
		 slot +=timeinterval
		 print slot, totalsize/(timeinterval*bw)
		 totalsize = 0.0
	}
	if (($FPTYPE == "ack") && ($FEGRESS == int($FDA)) && (newack[$FFID]!=$FSEQ) ) {
		size = ($FSEQ - newack[$FFID])*packetsize[$FFID];
		newack[$FFID] = $FSEQ;
		totalsize += size;
	}
      }
END { printf "\n\n" }
}
	exec sort -n -k 2 -k 8 -k 11 $inputfile | $AWK_ $awk_code startt=$timetransit_ bw=$maxbytes_ timeinterval=$timeinterval_ >>$outputfile
}



Metric instproc awk_goodputT {inputfile outputfile} {
	$self instvar maxbytes_ timetransit_ timeinterval_ AWK_

	# Input format: Generic packet trace
set awk_code {
        function pline (time, num, deno) {
		if (deno > 0) {
		     ratio = num/deno;
		} else {
		     ratio = 0
		}
        	printf("%g\t%3.2f\n", time, ratio);
 	}
BEGIN { 
	FEVENT=1 ; FTIME=2
	FINGRESS=3 ; FEGRESS=4
	FPTYPE=5 ; FPLEN=6
	FFLAG=7 ; FFID=8
	FSA=9 ; FDA=10
	FSEQ=11 ; FUID=12

	starttime = -1.0;
	finishtime = -1.0;
	totalsize = 0.0;
	slot= startt;
	}

$FTIME < startt {
		next;
	}

/^r/  {
	if (($FPTYPE == "ack") && ($FEGRESS == int($FDA)) && !($FFID in newack)) {
		newack[$FFID]=$FSEQ;
		if (starttime == -1) {
			starttime= $FTIME;
			print $FTIME, 0
		}
		next
        }
	if (($FPTYPE == "tcp") && ((($FEGRESS == int($FDA))|| ($FINGRESS == int($FSA))))) {
		packetsize[$FFID]= $FPLEN*1.0/bw;
		next
	}
	while ($FTIME >= slot+timeinterval) {
		 slot +=timeinterval
		 pline(slot, totalsize, slot-starttime)
	}

	if (($FPTYPE == "ack") && ($FEGRESS == int($FDA)) && (newack[$FFID]!=$FSEQ) ) {
		size = ($FSEQ - newack[$FFID])*packetsize[$FFID];
		newack[$FFID] = $FSEQ;
		totalsize += size;
		finishtime = $FTIME
        }
       }
END {
	pline(finishtime, totalsize, finishtime-starttime);
	printf "\n\n"
    }
}
     exec sort -n -k 2 $inputfile | $AWK_ $awk_code bw=$maxbytes_ timeinterval=$timeinterval_ startt=$timetransit_  >>$outputfile
}




#Counting event type 
Metric instproc awk_countingT {inputfile outputfile} {
	$self instvar event_ AWK_ timetransit_  modulo_
	
	# input format: Generic packet trace
set awk_code {
	BEGIN { 
		FEVENT=1 ; FTIME=2
		FINGRESS=3 ; FEGRESS=4
		FPTYPE=5 ; FPLEN=6
		FFLAG=7 ; FFID=8
		FSA=9 ; FDA=10
		FSEQ=11 ; FUID=12
			
		init= 1;
		currentuid = -1
		number=0;
		bytes=0;
	
    	    }
$FTIME < startt {
		next;
	}
	{
	 if (init) {
	 	print $FTIME, 0, 0
		init =0
	}
	if ($FEVENT != event)
			next;
	bytes +=$FPLEN;
	number = ++number % modulo
	print $FTIME, number, bytes
        # else {  #?????????
		#	if (event == "c")
		#		number++;
		#	}
	}
END { print "\n\n" }
}
	#set tmp "tmp.data"
	#catch "exec rm $tmp; touch $tmp"
	exec sort -n -k 2 $inputfile | $AWK_ $awk_code event=$event_ modulo=$modulo_ startt=$timetransit_  >>$outputfile
	#exec sort -n -k 1 $tmp | $AWK_ {BEGIN {nb=0; size=0}; {size+=$2;print $1, ++nb, size}} >>outputfile 
}


Metric instproc awk_icurve {inputfile outputfile} {
	$self instvar AWK_ modulo_ timetransit_
	
	# Input format: Generic packet trace	
set awkcode {
	BEGIN {
		# Field numbers
		FTIME=2; FPTYPE=5
		FPLEN=6; FFID=8
		FSEQ=11; FUID=12
		PacketSum=0
	}
$FTIME < startt {
         next
        }
 	{
	  # FID and PTYPE must be parameter to activate theses tests
	   if ( (FID != "") && (FID != $FFID) )
				next
  	   if ( (PTYPE != "") && (PTYPE != $FPTYPE) )
				next
	   PacketSum = ++PacketSum % modulo;
	   print $FTIME, PacketSum
	}
END { printf "\n"; }
}
	exec sort -n -k 2 $inputfile | $AWK_ $awkcode startt=$timetransit_ modulo=$modulo_  >>$outputfile
}


Metric instproc awk_acurve {inputfile outputfile} {
        $self instvar AWK_ timetransit_ timeinterval_ modulo_

set awk_code {
BEGIN {
		# Field numbers
		FTIME=2; FPTYPE=5
		FPLEN=6; FFID=8
		FSEQ=11; FUID=12
        frame=0;
	slot=startt*1.0;
        f[0]=0
        size=0
}
$FTIME < startt {
                next
                }
	{
	 while ($FTIME >= slot+timeinterval) {
                 frame++
		 f[frame]=size
                 slot +=timeinterval
	 }
	 size++;
	}
END {
     for (t= 0; t < frame; t++) { 
    	max = 0 
    	for (u =0;  u < frame; u++) {
	    interval = t+u
	    if (!(interval in f)) {
	    	break
	    }
	    decon = f[interval] - f[u]
	    if ( decon > max)
	    	max= decon
	 }
    	print t*timeinterval, max%modulo 
     }
     
    }
}
exec $AWK_ $awk_code starttt=$timetransit_ timeinterval=$timeinterval_ modulo=$modulo_ $inputfile >>$outputfile
}


#------------------------------------------------------
#                   S E C O N D   O R D E R 
#------------------------------------------------------
# Function called on datafile ONLY

Metric instproc filterdataflow {inputfile fid resultfile} {
	$self instvar AWK_
	
set awk_filter {
BEGIN {
	found=0	
      }
NF == 0  {
	found=0
	next
	}
/^\"/	{
	value=$2;
	if ($2~/\"$/) {
		value=substr(value,1,length(value)-1);
		}
	if (value == fid) found = 1
	next
      }
NF ==2 && found {
	print $1, $2
        }
}

	exec $AWK_ $awk_filter fid=$fid $inputfile >>$resultfile
}



Metric instproc statistics {} {
	$self instvar lastdata_ fieldid_ flowlist_ suffixdata_ AWK_
	
set awk_code {
function printfinal() {
	if (nbSample != 0) {
		mean=sum / nbSample
		variance = (sumsquare/ nbSample) - mean*mean
		stdev = sqrt(variance)
	} else {
		mean=0
		stdev =0
		variance =0
	}
	print "   Samples: " nbSample, " Mean: ", mean, " Stddev: ", stdev, " min: ", min, " Max: ", max
}
BEGIN {
	sum =-1;
}
/^\"/ 	{
		if (sum != -1)  { printfinal() }
	     	print "Statistic fid", $2
			sum = 0.0
			sumsquare = 0.0
			nbSample = 0
			max=0
			min=-1		
		next;
	}
NF == 2 {  
		sum +=  $2
		sumsquare += $2*$2
		nbSample++
		if ($2 > max)
				max=$2
		if (($2 < min) || (min == -1))
				min= $2
      }
END {
	printfinal()
    }
}
	set nameoutfile [$self GetFileName "statistics" $suffixdata_]
	exec echo "\"statistics $lastdata_\"" >>$nameoutfile
	exec $AWK_ $awk_code $lastdata_ >>$nameoutfile
	return  $nameoutfile
}



Metric instproc Average {} {
	$self instvar lastdata_ suffixdata_ weight_

# input format: time, rate
	set nameoutfile [$self GetFileName "Average" $suffixdata_]
	set math [new Math $lastdata_ $nameoutfile 1 2  ""]
        $math mean
	return  $nameoutfile
}

	
Metric instproc AverageFloat {} {
	$self instvar lastdata_ AWK_ suffixdata_ floating_
# input format: time, rate
set awk_code {
	BEGIN {
		FTIME=1; FRATE=2;
	      }
/^\"/ 	{
	   	print "\"Floating ", max+1 , "Average " , $2 , "\""
		i=0;
		cumul=0;
		next
	      }
NF == 2 {
		if (max in sample) {
			cumul = cumul + $2 - sample[i];
			print $FTIME, cumul/(max+1);
		} else {
			cumul = cumul + $2
		}
		sample[i] = $2
		i = (i+1) % (max+1)
	       }
NF == 0 {
		printf "\n"
 	       }
}
	set nameoutfile [$self GetFileName "FAverage" $suffixdata_]
	exec $AWK_ $awk_code max=[expr $floating_-1] $lastdata_ >>$nameoutfile
	return  $nameoutfile
}


Metric instproc AverageEWMA {} {
	$self instvar lastdata_ suffixdata_ weight_

# input format: time, rate
	set nameoutfile [$self GetFileName "Average" $suffixdata_]
	set math [new Math $lastdata_ $nameoutfile 1 2  "{weight $weight_}"]
        $math EWMA
	return  $nameoutfile
}


Metric instproc pdf_flow {} {
	$self instvar lastdata_ flowlist_ suffixdata_ accuracy_

	# find out the flow list
	set lfid [exec $AWK_ {/^\"/ {value=$2;if ($2~/\"$/) {value=substr(value,1,length(value)-1)};print value}} $lastdata_ ]
	set lfid [split $lfid "\n"]
        # check flow list with flow in lastdata file
	if {[llength $lfid] != [llength $flowlist_] } {
		puts "ERROR in pdf_flow: flow list not equal"
		puts "Packet trace: $flowlist_"
		puts "Data file   : $lfid"
		exit 1
		# set different flow list for data file and trace file if error occurs
	}

	# file names
	set nameoutfile [$self GetFileName "pdf" $suffixdata_]
	# per flow:
	foreach flow $flowlist_ {
                set tmp [makefile "tmppdffid" $suffixdata_]
	#	filter data set
		$self filterdataflow $lastdata_ $flow $tmp 
	#	exec echo "\"flow $flow\"" >>$nameoutfile
	#	compute the density function
                set math [new Math $tmp $nameoutfile 1 2]
                $math pdf $accuracy_
	}
	return  $nameoutfile
}


Metric instproc cdf_flow {} {
	$self instvar lastdata_ flowlist_ suffixdata_
	$self instvar AWK_

	set nameoutfile [$self GetFileName "cdf" $suffixdata_ ]

	foreach flow $flowlist_ {
	        set tmp [makefile "tmpcdffid" $suffixdata_] 	
		$self filterdataflow $lastdata_ $flow $tmp
		#exec echo "\"flow $flow\"" >>$nameoutfile
		set math [new Math $tmp $nameoutfile 1 2]
                $math cdf 
	}
	return  $nameoutfile
}


# Post process param file
#=======================================================================
Class Metric/Param -superclass Metric
#=======================================================================

Metric/Param instproc init {test parameters metrics verbose} {
       $self instvar tracedobjectclass_ 
       
       	
        set tracedobjectclass_ [lindex [split [$self info class] /] end ]


	$self next $test $parameters $metrics $verbose
}
        

Class Metric/TCP -superclass Metric/Param


Metric/TCP instproc MakeData {} {
	$self instvar  metrics_ log_ tracedobjectclass_ timetransit_
	$self instvar filenameparam_ AWK_ suffixdata_ modulo_ test_ verbose_

	# Input format: tcp tracer
	set awk_code {
		BEGIN {
			FTIME=1 ; FSADDR=2; FSPORT=3;
			FPARAM=6 ; FVALUE=7
		        OMFT = "%-8.5f"	
			currentsource =-1
			currentport = -1
		}
		$FSPORT !~/[0-9]/ {
			next
		}
                $FTIME < startt {
                        next
                }
		{
			if (($FSADDR != currentsource) || ($FSPORT != currentport)) {
				currentsource = $FSADDR
				currentport = $FSPORT
				printf "\n\n\"src %d.%d\"\n", $FSADDR, $FSPORT
			}
		
			if ($FSADDR == currentsource) {
				  time= int($FTIME*1000000)/1000000
			          printf "%-8.6f ", time
				  print  $FVALUE
				  # % mod;
			}
		}
	}
	foreach tracedvar $metrics_ {
                if {$verbose_} {
		        puts "     Computing  $tracedvar"
		}
                set nameoutfile [$self GetFileName ${tracedvar} $suffixdata_] 
    	    if {[catch "exec grep ${tracedvar}_  $filenameparam_ | wc -l" msg ]} {
                continue
            }
	    exec grep -w ${tracedvar}_ $filenameparam_ | sort -k 2n -k 3n -k 1n \
	    | $AWK_ $awk_code mod=$modulo_ startt=$timetransit_ >$nameoutfile
	    lappend log_ [list $tracedvar $nameoutfile]
	}
	
	puts "     Result $metrics_: $log_"
	return $log_
}

#=======================================================================
Class Metric/Box -superclass Metric/Param
#=======================================================================

# tracedvar list in the format : {who what}
Metric/Box instproc init {test parameters metrics verbose} {
	$self instvar log_ test_ filenameparam_ metrics_ suffixparam_
	$self instvar boxfile_ AWK_ tracedobjectclass_
	
        $self next $test $parameters $metrics $verbose


	set errno [catch "exec grep $tracedobjectclass_ $filenameparam_ | wc -l" nb]
	if {$errno != 0 || $nb == 0} {
		puts "     WARNING: Nothing to do for $tracedobjectclass_:  $filenameparam_ is empty"
	        return
		}
		
	set boxfile_ [$self GetFileName "boxobj" $suffixparam_]
	# find the list of nodeid
        set lnode [exec grep $tracedobjectclass_ $filenameparam_ | sort -k 2n  | cut -d " " -f 2 | uniq ]
        
         foreach item $lnode {
                if {$item == "0"} {
                        continue       
                }
                exec grep "\[.0-9\]* $item $tracedobjectclass_" $filenameparam_ >> $boxfile_
         }
}


Metric/Box instproc MakeData {} {
	$self instvar metrics_ AWK_ boxfile_ log_ suffixdata_ tracedobjectclass_

	if {![info exists boxfile_]} {
		return $log_
	}

set awk_code {
	BEGIN {
		FTIME =1;  FNODEID= 2; FNAME=3; FPARAM=6
		FVALUE=7
		currentbox =-1
		OMFT  = "%-8.5f"
		}

$FNODEID ~/0/ || $FNAME ~/[0-9]/ {
		next
	      }
		
$FNODEID != currentbox	{
		currentbox = $FNODEID
		printf "\n\n\"%s %d\"\n", $FNAME, $FNODEID
		printf "0.0 0\n"
	      }
	{
	  #printf "%g %g\n", $FTIME, $FVALUE
	  printf "%-8.5f ", $FTIME
	  printf "%8s "   , $FVALUE
          FNUM = FVALUE + 1
          while ( FNUM <= NF) {
                printf "%s ", $FNUM
                FNUM ++
          }
          printf "\n"
	}
}

        set tmp [makefile "tmpbox" "awk"] 
        exec echo "$awk_code" >> $tmp

        foreach tracedvar $metrics_ {
		puts "     Proceed for $tracedobjectclass_ $tracedvar"
		set nameoutfile [$self GetFileName ${tracedvar} $suffixdata_]
                set cmdline "grep -w ${tracedvar}_ $boxfile_ | $AWK_ -f $tmp  >$nameoutfile"
		if {[catch "exec $cmdline" msg]} {
                        if {[lindex $::errorCode 0] == "CHILDSTATUS"} {
                                puts "   ***ERROR on ${tracedvar}_ : code $::errorCode"
                                puts "      msg: $msg"; 
                                continue
                           }
                 }
		lappend log_ [list $tracedvar $nameoutfile]
		
	}
	puts "     Result for $tracedobjectclass_ $metrics_: $log_"
	return $log_
}


#=================================================================
Class Metric/RED -superclass Metric
#=================================================================

Metric/RED instproc init {test parameters metrics verbose} {
        $self instvar suffixparam_ filenameparam_ inputfile_
        $self instvar AWK_
        
        $self next  $test $parameters $metrics $verbose

set awk_code {
        $1 ~ /[aQ]/ && NF >2{
                print $0
                }
}
        set name [lindex [split [$self info class] /] end]
        set inputfile_ [$self GetFileName $name $suffixparam_]
        exec $AWK_ $awk_code $filenameparam_ >$inputfile_
}


# Metric the queue size and average queue size, for RED gateways.
# 	x axis: time
#       y axis: queue size
Metric/RED instproc length {} {
        $self instvar inputfile_ AWK_ suffixdata_
	#
set awk_code {
        BEGIN {
                system("touch tmp.awk")
                print "\"RED_current\""
	}
        {
	 if ($1 == "Q") {
			print $2, $3;
	 } else if ($1 == "a")
			print $2, $3 >> "tmp.awk";
	}
        END {
                print "\n\n\"ave_queue\""
                system("cat tmp.awk; rm -f tmp.awk")
        }
}
	
        puts "               for RED queue"
	set nameoutfile [$self GetFileName "length" $suffixdata_]
	
	exec $AWK_ $awk_code $inputfile_ >$nameoutfile
	return $nameoutfile
}

#=======================================================================
Class Metric/Error -superclass Metric
#=======================================================================
# Trace generated by Error model
Metric/Error instproc init {test parameters scopeitem verbose} {
	$self instvar filenameparam_ inputfile_ 
	$self instvar AWK_ suffixdata_
	
	$self next $test $parameters $scopeitem $verbose

        set tmp "tmp.prm"
        catch "exec rm $tmp; touch $tmp"

	catch "exec grep ErrModel $filenameparam_ >>$tmp"
        if {[file size $tmp]} {
	        set inputfile_ [$self GetFileName "Modul" $suffixdata_]
                exec sort -k 6n -k 2n $tmp >>$inputfile_
        }
}


Metric/Error instproc MakeData {} {
	$self instvar AWK_ metrics_ suffixdata_ log_ inputfile_

set awk_code {
BEGIN {
		FTIME=2; FPTYPE=5;
		FSA=6; FFID=7
		FSEQ=8; FUID=9
		FETYPE=10; FEOPT=11
	currentnode =-1
}
$FSA != currentnode {
               currentnode = $FSA
                printf "\n\n\"src %s\"\n", $FSA
              }
      {
	print $FTIME, $FUID, $FPTYPE, $FEOPT, $FETYPE ;
      }
}

if {![info exists inputfile_]}  {
        return {}
}

if {![file size $inputfile_]}  {
        puts" WARNING: $inputfile_ is EMPTY"
        return {}
}

foreach tracederror $metrics_ {
	puts "     Proceed for ErrorModule $tracederror"
	set nameoutfile [$self GetFileName $tracederror $suffixdata_]

        catch "exec $AWK_  {match(\$0, traced) {print \$0}} traced=$tracederror $inputfile_ | wc -l" len
        if {$len == 0} {
              puts " WARNING: no $tracederror in $inputfile_"
              return {}
        }
        exec grep -w $tracederror $inputfile_ |  $AWK_ $awk_code >>$nameoutfile 
        #if {$errno} 
        #        puts "***ERROR on Metric/Error:MakeData on exec call : $msg"
        #        exit 1
        lappend log_ [list $tracederror $nameoutfile]
}
    puts "     Result for ErrorModule $metrics_: $log_"
    return $log_
}


# post process packet flow
#=======================================================================
Class Metric/Flow -superclass Metric
#=======================================================================

Metric/Flow instproc init {test parameters scopeitem verbose } {
	$self instvar filenamepkt_ flowlist_ flowlistlib_ inputfile_ maxbytes_ bottlebw_
	$self instvar suffixpkt_ flowgranularity_ duplexfile_
	
	$self next $test $parameters  [lindex $scopeitem 1] $verbose
	set casting [lindex $scopeitem 0]
        # By default on the per flow basis
        $self PerFlow
	
	 
	if {$casting == "*"} {
		set duplexfile_ [$self GetFileName "allflow" $suffixpkt_]
		exec cp $filenamepkt_ $duplexfile_
		set flowlist_ [$self listfid $filenamepkt_]
        } else {
                set flowlist_ [$self exlfid $casting]
                set duplexfile_ [$self InputFile $flowlist_]
        }
	set flowlistlib_ [firstlast $flowlist_]
	set inputfile_ [$self Flowsimplex $duplexfile_]
	set maxbytes_ [expr [bw_parse $bottlebw_]/ (8.0*100)]
	
	puts "\t\tBottleneck bw set to $bottlebw_ for flow $flowlistlib_"
	
}

#----------------------------------------------------------------
#                    What to do
#----------------------------------------------------------------

Metric/Flow instproc activity {} {
	$self instvar duplexfile_ inputfile_ suffixpkt_ AWK_ PERL_ RAW2XG_ suffixdata_ wrap_ scale_

set awk_code {
BEGIN {
		# Field numbers
		FEVENT=1 ; FTIME=2
		FINGRESS=3 ; FEGRESS=4
		FPTYPE=5 ; FPLEN=6
		FFLAG=7 ; FFID=8
		FSA=9 ; FDA=10
		FSEQ=11 ; FUID=12
	}
$5~/ack/ { 
	 if (int($FDA) == $FEGRESS)  {
             if ($FEVENT == "r") {
	 	printf ("-"); # - instead of r for raw2xg
		for (i = 2; i <= NF ; i++) {
			printf (" %s", $i);
		}
		printf ("\n");
             }
	 }
	 next
	}
/^\+/	{
	if (int($9) == $3) {
		print $0
	}
	}
}
	#keep trace on source link
	set send [$self GetFileName "send" $suffixpkt_]
	exec $AWK_ $awk_code $duplexfile_  >>$send
        set tmp "tmpactivity.pkt"
        catch "exec rm $tmp; touch $tmp"
	
	# add drop in the network like a drop on source link
	catch "exec grep \"^d\"  $inputfile_ | wc -l" count ;
		# print the count of drop packet 
	        # or use -l option: file from which output would
		#              normally have  been  printed. The
                #              scanning will stop on the first match
	#set length [llength $filematch]
	set length [lindex $count 0]
	if {$length  > 0 } {
	      set drop [$self GetFileName "drop" $suffixpkt_]
	      exec grep "^d" $inputfile_  | sort -n -k 12 -k 2 >>$drop
	      set dfd [open $drop r]; 
	      set sfd [open $send r]
	      set ofd [open $tmp w]
	      
	      gets $dfd line
	      set dpkt [lindex [split $line " "] 11]
	    
	      while {[gets $sfd line] >0} {
		if {$dpkt == [lindex [split $line " "] 11] } {
		    set newline [lreplace [split $line " "] 0 0 "d"]
		    puts $ofd [join $newline " "]
		    if {[gets $dfd line] >0} {
                        set dpkt [lindex [split $line " "] 11]
                    } else {
                       	set dpkt ""
                    }
		} else {
			puts $ofd $line
		}	
	      }
	      close $ofd
	      close $sfd
	     close $dfd
	} else {
	     # no drop in the simplex packet trace file (inputfile_)
	      exec mv -f $send $tmp
	}

	set inputfile [$self GetFileName "Activity" $suffixpkt_]
	set nameoutfile [$self GetFileName "Activity" $suffixdata_]
        eval "exec sort -k 2n -k 11n -k 12n $tmp >>$inputfile"
	eval "exec  $PERL_ $RAW2XG_ -a -s $scale_ -m $wrap_ $inputfile >$nameoutfile"
	
	return $nameoutfile
}


# Metric the input curve of the flow 
#Metric/Flow instproc inputcurve {} {
#	$self instvar inputfile_ suffixpkt_ suffixdata_ AWK_
#
#	set send [$self GetFileName "send" $suffixpkt_]
	# eliminate trace on the transit links (keep trace on source link)
#        exec grep "^\-" $inputfile_ | $AWK_ {int($9) == $3 {print $0}} >>$send

#	set nameoutfile [$self GetFileName "icurve" $suffixdata_]
#	$self DoPerFlow awk_icurve $send $nameoutfile
	
#	return  $nameoutfile
#
#}


#-----------------------------------------------------------------------
#                 D E L A Y
#-----------------------------------------------------------------------


# 	x axis: time
#       y axis: end to end delay
Metric/Flow instproc delay {} { 
	$self instvar inputfile_ suffixdata_ lastdata_ suffixpkt_

	set lastdata_ [$self GetFileName "delay" $suffixdata_]
        set datapkt [$self GetFileName "tmpdatapkt" $suffixpkt_]
	exec grep -v ack $inputfile_  >>$datapkt

	$self DoMetric awk_delay $datapkt $lastdata_

	return $lastdata_
}

Metric/Flow instproc Delay_per_fid {} {
	$self instvar inputfile_ AWK_  suffixdata_ lastdata_
	
# Input format: Generic packet trace
set awk_code {
	BEGIN {
		# Field numbers
		FEVENT=1 ; FTIME=2
		FINGRESS=3 ; FEGRESS=4
		FPTYPE=5 ; FPLEN=6
		FFLAG=7 ; FFID=8
		FSA=9 ; FDA=10
		FSEQ=11 ; FUID=12
		currentfid = -1
	}
	{      
	       # To filter trace on a specific FID
	       #if ( FID != "" && FID != $FFID )
	       #		next
			
	       if ($FFID != currentfid) {
			sourceid= int($FSA);
			destid= int($FDA);
			proto=$FPTYPE
			currentfid=$FFID
			pktid  = 0;
				
			printf "\n\n"
	   		printf "\"%s %i\"\n", $FPTYPE, $FFID
	        }
	}
/^\+/ {	
		if ( $FINGRESS != sourceid )
			next
			
			pktid = $FSEQ
			sdtime = $FTIME
		}
/^r/ {
		if ( $FEGRESS != destid )
				next
		if ($FSEQ = pktid) {
			delay = $FTIME-sdtime
			if ((delay > 0) && (sdtime >= startt)) {
					print sdtime, delay
			}
	        }
      }
  END {			printf "\n\n"
      }
}
	
	set lastdata_ [$self GetFileName "delay" $suffixdata_]
	exec grep -v ack $inputfile_  | sort -n -k 8 -k 11 -k 12 -k 2 | $AWK_ $awk_code >>$lastdata_

	return $lastdata_
}


# 	x axis: time
#       y axis: delay for aggregate flow
Metric/Flow instproc Delay_with_tab {} {
	$self instvar inputfile_ AWK_ lastdata_ suffixdata_
	
# simple awk script to generate end-to-end packet lifetime statistics
# in a form suitable for plotting with xgraph.
set awk_code {
BEGIN {
		# field numbers
		FEVENT=1 ; FTIME=2
		FINGRESS=3 ; FEGRESS=4
		FPTYPE=5 ; FPLEN=6
		FFLAG=7 ; FFID=8
		FSA=9 ; FDA=10
		FSEQ=11 ; FUID=12
	}
# To filter trace on a specific FID or PTYPE
# ((FID != "") && ($FFID != FID)) || ((PTYPE != "") && (PTYPE != $FPTYPE)) {
#		next
#	}		
/^\+/	{
	# getting start time is not a problem, provided you're not starting
	# traffic at 0.0.
	#  test for sending node_1_address
	if ($FINGRESS != int($FSA)) {
		next
	}
	start_time[$FUID] = $FTIME;
	end_time[$FUID] = -1;
	next;
       }
/^d/ {
	delete end_time[$FUID]
     }
/^r/ {
	# only useful for small unicast where packet_id doesn't wrap.
	# checking receive means avoiding recording drops
	#  test for receiving node_2_address or flow_id here.
	if ($FEGRESS != int($FDA)) {
		next
	}
		end_time[$FUID] = $FTIME;
     } 
END {
	for (packet_id in end_time) {
		start = start_time[packet_id]; end = end_time[packet_id];
		packet_duration = end - start;
		if ( start < end ) 
			printf("%f %f\n", start, packet_duration);
		}
		printf("\n\n");
    }
}

	set lastdata_ [$self GetFileName "Agrgegat delay" $suffixdata_]
	exec sort -n -k 12 -k 2 $inputfile_ | $AWK_ $awk_code | sort -n -k 1 >>$lastdata_

	return $lastdata_
}


#-------------------------------------------------------------------------
#                      L O S S
#-------------------------------------------------------------------------

# 	x axis: time
#       y axis: drop ratio per fid
#        # drop/ # sending packet
Metric/Flow instproc dropratio {} {
	$self instvar inputfile_ suffixdata_ lastdata_
	
	set lastdata_ [$self GetFileName "dropratio" $suffixdata_]
	$self DoMetric awk_DropRatio $inputfile_ $lastdata_
	return  $lastdata_
}


# 	x axis: time
#       y axis: dropput 
#	lost bytes/ mesured time
Metric/Flow instproc dropput {} {
	$self instvar inputfile_ lastdata_ suffixdata_ suffixpkt_

	set lastdata_ [$self GetFileName "drop_put" $suffixdata_]
	set dropevent [$self GetFileName "tmpdropevent" $suffixpkt_]
	exec grep "^d" $inputfile_ >>$dropevent
	
	$self DoMetric awk_rate $dropevent $lastdata_
	return $lastdata_
}

	
#  Drop event in time
Metric/Flow instproc dropevent {} {
	$self instvar inputfile_ suffixdata_ lastdata_

	set lastdata_ [$self GetFileName "dropevent" $suffixdata_]
	$self DoMetric awk_DropEvent $inputfile_ $lastdata_
	return $lastdata_
}



#  Total drop ratio over the time
Metric/Flow instproc dropratioT {} {
	$self instvar inputfile_ suffixdata_ lastdata_ suffixpkt_ AWK_

	set lastdata_ [$self GetFileName "TDrop" $suffixdata_]
	set flowfile [$self GetFileName "tmpflow" $suffixpkt_]

	exec $AWK_ { 
                     /^d/ {print $0
                           next} 
                     $3 ==  int($9) {print $0}
                   } $inputfile_ >>$flowfile
	$self DoMetric awk_DropratioT $flowfile $lastdata_
	return $lastdata_
}

Metric/Flow instproc dropprecedenceT {} {
	$self instvar inputfile_ suffixdata_ lastdata_

	set lastdata_ [$self GetFileName "TPrec" $suffixdata_]
	$self DoMetric awk_DropPrecedenceT $inputfile_ $lastdata_
	return $lastdata_
}

#-----------------------------------------------------------------------
#                 T H R O U G H P U T 
#-----------------------------------------------------------------------


#	sending rate = f(time)
#       y axis: normalized rate  
Metric/Flow instproc rate {} {
	$self instvar inputfile_ suffixpkt_ suffixdata_ lastdata_ AWK_

    set lastdata_ [ $self GetFileName "sendrate" $suffixdata_]
    set sendevent [$self GetFileName "tmpsendevent" $suffixpkt_]
    # Filter inputfile_ (keep sending trace at source node for data packet only)
    exec grep "^\-" $inputfile_ | $AWK_ {int($9) == $3 {print $0}} >>$sendevent
    
	$self DoMetric awk_rate $sendevent $lastdata_
	return $lastdata_
}


#	throughput = f(time)
#       y axis: normalized rate  
Metric/Flow instproc throughput {} {
	$self instvar inputfile_ suffixpkt_ suffixdata_ lastdata_  AWK_

    set lastdata_ [ $self GetFileName "thruput" $suffixdata_]
    set receiveevent [$self GetFileName "tmpreceiveevent" $suffixpkt_]
    # Filter inputfile_ (keep receive trace at destination node fo data packet only)
    exec grep "^r" $inputfile_ | $AWK_ {int($10) == $4 {print $0}} >>$receiveevent
    
	$self DoMetric awk_rate $receiveevent $lastdata_
    	return $lastdata_
}

     # other method: to compute throughput agregat
     #set lreceiver [exec sort -n -k 8 -k 2 $filenamepkt_ | cut -d " " -f 1-10 | uniq -f 7 | awk {{print int($10)}} | uniq ]
     #set inputfile_ [$self GetFileName "network" suffixpkt_]
     #foreach line $lreceiver 
     #	exec grep "^r \[.0-9\]* . $line" $filenamepkt >> $inputfile_


Metric/Flow instproc throughput_TSW {} {
	$self instvar inputfile_ lastdata_ suffixdata_ suffixpkt_ AWK_
	
    set lastdata_ [$self GetFileName "TSWThruput" $suffixdata_]
    set receiveevent [$self GetFileName "tmpreceiveevent" $suffixpkt_]
    # Filter inputfile_ (keep receive trace at destination node
    exec grep "^r" $inputfile_ | $AWK_ {int($10) == $4 {print $0}} >>$receiveevent

	$self DoMetric awk_throughputTSW $receiveevent $lastdata_
        return $lastdata_
}


# Total throughput for the flow
Metric/Flow instproc throughputT {} {
	$self instvar inputfile_ suffixdata_ suffixpkt_ lastdata_ AWK_

      set lastdata_ [$self GetFileName "Tthruput" $suffixdata_]
      set receiveevent [$self GetFileName "tmpreceiveevent" $suffixpkt_]
      exec grep "^r" $inputfile_ | $AWK_ {int($10) == $4 {print $0}} >>$receiveevent
	
	$self DoMetric awk_rate_sustainable $receiveevent $lastdata_
        return $lastdata_
}


#	goodput = f(time)
Metric/Flow instproc goodput {} {
	$self instvar duplexfile_ lastdata_ suffixdata_ 

        set lastdata_ [ $self GetFileName "goodput" $suffixdata_]
	$self DoMetric awk_goodput $duplexfile_ $lastdata_
        return $lastdata_
}


# Total goodput 
Metric/Flow instproc goodputT {} {
	$self instvar duplexfile_ suffixdata_ suffixpkt_ lastdata_

        set lastdata_ [$self GetFileName "Tgoodput" $suffixdata_]
	$self DoMetric awk_goodputT $duplexfile_ $lastdata_
        return $lastdata_
}


Metric/Flow instproc receivepkt { } {
	$self instvar inputfile_ lastdata_ event_ suffixdata_ suffixpkt_ AWK_

	set lastdata_ [$self GetFileName "countr" $suffixdata_]
	set receiveevent [$self GetFileName "tmpreceiveevent" $suffixpkt_]
	set event_ r
        exec grep "^r" $inputfile_ | $AWK_ {int($10) == $4 {print $0}} >>$receiveevent

	$self DoMetric awk_countingT $receiveevent $lastdata_
       
	return $lastdata_
}


Metric/Flow instproc sendpkt { } {
	$self instvar inputfile_ event_ suffixdata_ lastdata_ suffixpkt_
	$self instvar AWK_
	
	set lastdata_ [$self GetFileName "countm" $suffixdata_]
	set sendevent [$self GetFileName "tmpsendevent" $suffixpkt_]
	set event_ {\-} 
	# keep sending packet by source nodes
        exec grep "^\-" $inputfile_ | $AWK_ {int($9) == $3 {print $0}} >>$sendevent
	
	$self DoMetric awk_countingT $sendevent $lastdata_

	return $lastdata_
}

Metric/Flow instproc droppkt { } {
	$self instvar inputfile_ event_ suffixdata_ lastdata_
	
	set lastdata_  [$self GetFileName "countd" $suffixdata_]
	set event_ d
	
	$self DoMetric awk_countingT $inputfile_ $lastdata_

	return $lastdata_
}



# Post process at a link
#=============================================================
Class Metric/Queue -superclass Metric
#=============================================================

# TO DO: compute TDroprate,  Trate
# TO DO: compute droprate for a link
 
Metric/Queue instproc init {test parameters scopeitem verbose} {
	$self instvar inputfilesimplex_
	$self instvar inputfileduplex_
	$self instvar n1_ n2_ maxbytes_ bottlebw_ flowlist_ AWK_
	
	# find out tracefile
	$self next $test $parameters  [lindex $scopeitem 1] $verbose

	set casting [lindex $scopeitem 0]
	if {[llength $casting] != 2} {
		puts "ERROR: wrong number of nodes to indicate a queue: $casting"
		exit 1
	}
	set n1_ [lindex $casting 0]
	set n2_ [lindex $casting 1]
	# Make the appropriate inputfile
	set inputfilesimplex_ [$self InputFile 0]
	set inputfileduplex_ [$self InputFile 1]
	
        set maxbytes_ [expr [bw_parse $bottlebw_]/ (8.0*100)]

	# find out fid list
	set tmp "tmp.data"; catch "exec rm -f $tmp ; touch $tmp"
  	exec grep "^r" $inputfilesimplex_ | sort -n -r -k 12 -k 2 | uniq -f 11 >>$tmp
	set lfid [exec sort -n -k 8 $tmp | cut -d " " -f 1-8 | uniq -f 7 | $AWK_ {{print $8}}]
	set flowlist_ [split $lfid "\n"]

        # By default work on aggregate
	$self OverAll


}

Metric/Queue instproc InputFile {{duplex 0}} {
	$self instvar suffixpkt_ filenamepkt_ n1_ n2_
 	$self instvar PERL_ GETRC_ 

    if { ! $duplex } {
        set lib "simp"
    } else {
        set lib ""
    }
    set inputfile [$self GetFileName [format "tmp%s" $lib] $suffixpkt_]    
    if {$duplex} {
       exec $PERL_ $GETRC_ -b -s $n1_ -d $n2_ -o $inputfile $filenamepkt_    
    } else {
       exec $PERL_ $GETRC_ -s $n1_ -d $n2_ -o $inputfile  $filenamepkt_    
    }

	if {[file size $inputfile] == 0} {
		puts "ERROR: file size null for  $inputfile "
		exit 1
	}
    return $inputfile
}

	

Metric/Queue instproc activity {} {
	$self instvar inputfileduplex_  PERL_ RAW2XG_ suffixdata_ wrap_ scale_
	
	set nameoutfile [$self GetFileName "Activity" $suffixdata_]
	
	# At the bottleneck
	eval "exec $PERL_ $RAW2XG_ -a -s $scale_ -m $wrap_ <$inputfileduplex_ >$nameoutfile"
	return  $nameoutfile
}


Metric/Queue instproc inputcurve {} {
        $self instvar flowgranularity_ flowlist_ AWK_
        $self instvar inputfilesimplex_ suffixdata_ n1_ n2_ lastdata_ suffixpkt_


        set lastdata_ [ $self GetFileName "icurve" $suffixdata_]
	set send [$self GetFileName "send" $suffixpkt_]
	exec grep "^\-" $inputfilesimplex_ >>$send

	if {$flowgranularity_ == "OverAll"} {
		exec echo "\"Input curve $n1_ $n2_ \"" >>$lastdata_
		$self awk_icurve $send $lastdata_
	} else {
               $self DoPerFlow  awk_icurve $send  $lastdata_
      	}	
        return $lastdata_

}


Metric/Queue instproc arrivalcurve {} {
        $self instvar flowgranularity_ flowlist_ AWK_
        $self instvar inputfilesimplex_ suffixdata_ n1_ n2_ lastdata_ suffixpkt_


        set lastdata_ [ $self GetFileName "acurve" $suffixdata_]
	set send [$self GetFileName "send" $suffixpkt_]
	exec grep "^\-" $inputfilesimplex_ >>$send

	if {$flowgranularity_ == "OverAll"} {
		exec echo "\"Arrival curve $n1_ $n2_ \"" >>$lastdata_
		$self awk_acurve $send $lastdata_
	} else {
               $self DoPerFlow  awk_acurve $send  $lastdata_
      	}	
        return $lastdata_

}

#-----------------------------------------------------------------------
#                       D R O P
#-----------------------------------------------------------------------

Metric/Queue instproc dropratioT {} {
	$self instvar inputfilesimplex_ suffixdata_  lastdata_ n1_ n2_

	set lastdata_ [ $self GetFileName "TDrop" $suffixdata_]
	exec echo "\"Link    -      $n1_ - $n2_ \"" >>$lastdata_
	$self awk_DropratioT $inputfilesimplex_ $lastdata_

	return $lastdata_
}

#-----------------------------------------------------------------------
#                        R A T E 
#-----------------------------------------------------------------------


Metric/Queue instproc  rateinstant {} {
	$self instvar inputfilesimplex_ lastdata_ suffixdata_ n1_ n2_ suffixpkt_
	$self instvar flowgranularity_

	
	set lastdata_ [ $self GetFileName "irate" $suffixdata_]
	set send [$self GetFileName "tmpsend" $suffixpkt_]
	exec grep "^\-" $inputfilesimplex_  >>$send
	
	exec echo "\"Link $n1_ - $n2_ \"" >>$lastdata_
	if {$flowgranularity_ == "OverAll"} {
		self awk_rate_size $send $lastdata_
	} else {
                exec echo "0 0 \n\n" >>$lastdata_
		$self DoPerFlow awk_rate_size $send $lastdata_
	}		
	return  $lastdata_
}


#  Compute the throughput of the link over the time 
#	x axis: time
#	y axis: rate (in normalized way i.e. in %)
Metric/Queue instproc throughput {} {
	$self instvar inputfilesimplex_ lastdata_ suffixdata_ n1_ n2_ suffixpkt_
	$self instvar flowgranularity_

	
	set lastdata_ [ $self GetFileName "Thruput" $suffixdata_]
	set receiveevent [$self GetFileName "tmpreceiveevent" $suffixpkt_]
	exec grep "^r" $inputfilesimplex_  >>$receiveevent
	
	if {$flowgranularity_ == "OverAll"} {
		exec echo "\"Link $n1_ - $n2_ \"" >>$lastdata_
		$self awk_rate $receiveevent $lastdata_
	} else {
		$self DoPerFlow awk_rate $receiveevent $lastdata_
	}		
	return  $lastdata_
}

#  Compute the throughput of the link over the time 
#	x axis: time
#	y axis: rate (in normalized way i.e. in %)
Metric/Queue instproc throughput_TSW {} {
	$self instvar inputfilesimplex_ lastdata_ suffixdata_ n1_ n2_ suffixpkt_
        $self instvar flowgranularity_

	
	set lastdata_ [ $self GetFileName "ThruputTSW" $suffixdata_]
	set receiveevent [$self GetFileName "tmpreceiveevent" $suffixpkt_]
	exec grep "^r" $inputfilesimplex_  >>$receiveevent

	if {$flowgranularity_ == "OverAll"} {
		exec echo "\"Link $n1_ - $n2_ \"" >>$lastdata_
		$self awk_throughputTSW $receiveevent $lastdata_
	} else {
		$self DoPerFlow  awk_throughputTSW $receiveevent $lastdata_
	}		
	return  $lastdata_
}


#       print the total throughput over the time
Metric/Queue instproc throughputT {} {
	$self instvar inputfilesimplex_ suffixdata_ n1_ n2_ lastdata_ suffixpkt_
        $self instvar flowgranularity_ flowlist_

        set lastdata_ [ $self GetFileName "Tthruput" $suffixdata_]
	set receiveevent [$self GetFileName "tmpreceiveevent" $suffixpkt_]
	exec grep "^r" $inputfilesimplex_  >>$receiveevent

	if {$flowgranularity_ == "OverAll"} {
		exec echo "\"Total throughput $n1_ $n2_ \"" >>$lastdata_
444¿		$self awk_rate_sustainable $receiveevent $lastdata_
	} else {
               $self DoPerFlow  awk_rate_sustainable $receiveevent  $lastdata_
      	}	
        return $lastdata_
}

#-----------------------------------------------------------------------
#                        L E N G T H  
#-----------------------------------------------------------------------


# 	x axis: time
#       y axis: queue size and average queue size
Metric/Queue instproc length {} {
    $self instvar inputfilesimplex_  weightlength_ suffixdata_ AWK_ bottlebw_ lastdata_

set awk_code {
	BEGIN {
	FEVENT=1 ; FTIME=2
	FINGRESS=3 ; FEGRESS=4
	FPTYPE=5 ; FPLEN=6
	FFLAG=7 ; FFID=8
	FSA=9 ; FDA=10
	FSEQ=11 ; FUID=12
	ave = 0.0
	curq= 0
	oldcurq = -1
	idletime = 0.0;
	system ("touch tmp.awk")
	print "\"current\""
        }
	$FEVENT ~ /\+/ {
		if (!curq) {
			m = int(ptc*($FTIME-idletime))
		  while (m>=1) {
			ave *= (1.0-q_weight);
			m--
		  }
		  	idletime = 0.0
		}
		ave *= (1.0-q_weight);
		ave +=  q_weight*curq;
		if (oldcurq != curq) {
		 	printf "%g %g\n", $FTIME, ave >>"tmp.awk"
			printf "%g %u\n", $FTIME, curq
			oldcurq = curq
		}
		curq += 1;
		next
	}
	/^-/ || /^d/ {
		curq -= 1;
		if (!curq) {
			idletime = $FTIME
		}
	  }
	END {
	    printf "\n\n"
            print "\"average\""
	system("cat tmp.awk; rm -f tmp.awk")
	}
}


# compute the average packetsize
set awk_packet {
BEGIN {
	FPLEN=6; totalsize=0;
	sample=0; 
}
/^\+/ {
	totalsize+= $FPLEN
	sample ++
}
END {
	print int(totalsize/sample)
}
}

	set lastdata_ [ $self GetFileName "length" $suffixdata_]
	set packetsize [exec $AWK_ $awk_packet $inputfilesimplex_]
	set ptc [expr [bw_parse $bottlebw_]/ (8.0*$packetsize)]
	exec $AWK_ $awk_code q_weight=[expr 1.0*$weightlength_] ptc=$ptc $inputfilesimplex_ > $lastdata_

	return $lastdata_
}

#-----------------------------------------------------------------------
#                        E X P E R I M E N T A L 
#-----------------------------------------------------------------------

Metric/Queue instproc congestion_event {} {
	$self instvar lastdata_ suffixdata_ AWK_

set awk_code {
BEGIN {
	print "\"Queue Congestion\"";
	congestion =0;
	event = 0;
	average=0;
}
/^\"/   {
	if (average){
		exit
	}
	average = 1;
}

$2 == qlimit {
	if (!congestion) {
		event++;
		congestion =1;
		print $1, 1, event
		start= $1
	}
}
$2 < (qlimit-2) {
	if (congestion) {
		congestion =0;
		if ($1 >= start) {
			print $1, 0, event, ($1-start);
		}
	}

}
END {
}
}
	set nameoutfile [$self GetFileName "CEvent" $suffixdata_]
	set limit [exec $AWK_ {BEGIN {maxsize =0;}
  	                     NF == 2 { if ($2 >maxsize) {maxsize = $2}}
	                     END {print maxsize} } $lastdata_]
	exec $AWK_ $awk_code qlimit=$limit $lastdata_ >$nameoutfile
	
	return $nameoutfile;
}




#==================================
# POST PROCESS
Class Metric/ICN -superclass Metric/Box
Class Metric/TSPrinter -superclass Metric/Box

# in test-metric: new Metric/$tracedobject

