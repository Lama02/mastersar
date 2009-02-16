#
# Copyright (c) 2006 University of La Reunion - France
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

#puts "#load test-grapher.tcl"

Class Grapher 

Grapher instproc init {toplot datafile verbose} {
	$self instvar graphfilename_ suffixgrapher_
	$self instvar verbose_ inputfile_ toplot_

	set suffixgrapher_ [GetSuffix [file tail [$self info class]]]
	set verbose_ $verbose
        
	# set the graph file
	set graphfilename_ [file rootname $datafile]
	append graphfilename "." $suffixgrapher_

	# datafile to plot
	set inputfile_ $datafile
	# what to plot
	set toplot_ $toplot
}


Grapher instproc draw {quiet} {
	$self instvar toplot_ quiet_ verbose_
        
	set quiet_ $quiet
        if {$verbose_} {
	        puts "Call plotting $toplot_"
	}
        if {[catch "eval $self $toplot_"]} {
		$self param
	}
}

Grapher instproc finishstate {inputfile} {

set awk_code {
BEGIN {
	newindex = 0
	lastline = ""
      }	
NF == 0 {
	if (newindex) {
		print lastline
		lastline = ""
		newindex =0
	}
	next
	}
/^\"/	{
	 newindex = 1
	 printf "%s\t", $0 
	}
NF >= 2 {
	lastline = $0
	}
END { print lastline}
}
        set tmp "tmp.data"
        catch "exec rm $tmp; touch $tmp"
	#puts -nonewline "  Finishstate: "
	exec awk $awk_code $inputfile >> $tmp
	return $tmp
}

Grapher instproc cat {inputfile} {

	set filefd [open $inputfile r]
	while {[gets $filefd line]>= 0} {
		puts $line
    	}
	close $filefd
}


#----------------------------------------------------
Class Grapher/xgraph -superclass Grapher

Grapher/xgraph instproc graph {tittle ylib {xlib "time (s)"}} {
	$self instvar inputfile_ quiet_
       
       if {$quiet_} {
                return
        }
	file stat $inputfile_ t
	if {$t(size) == 0} {
		puts "\t $inputfile_: empty file"
		return
	}

	exec xgraph -bg white -bb -tk -x "$xlib" -y "$ylib" -t "$tittle" $inputfile_ &
}

Grapher/xgraph instproc unknown {} {
	$self instvar inputfile_
	
        puts "  with file: $inputfile_ "
	set tmp [$self finishstate $inputfile_]
	$self cat $tmp
	$self graph  "$inputfile_"  "Nb"

}

Grapher/xgraph instproc countingT {} {
	$self instvar inputfile_
	
	set tmp [$self finishstate $inputfile_]
	puts "flow <nl> time #packet total size"
	$self cat $tmp
}

Grapher/xgraph instproc param {} {
	$self instvar inputfile_ toplot_

	$self graph "$toplot_" "$toplot_"
}

Grapher/xgraph instproc activity {} {
	$self instvar inputfile_ quiet_
       
       if {$quiet_} {
                return
        }
	file stat $inputfile_ t
	if {$t(size) == 0} {
		puts "\t $inputfile_: empty file"
		return
	}

     exec xgraph -bg white -bb -tk -nl -m -x time -y packets -t "activity" $inputfile_ &

}

Grapher/xgraph instproc inputcurve {} {
	$self instvar inputfile_

	$self graph "Input curve" packet

}

Grapher/xgraph instproc delay {} {
	$self instvar inputfile_

	$self graph "Delay" delay
}


Grapher/xgraph instproc OverAll {} {
	# dummy proc
}

Grapher/xgraph instproc PerFlow {} {
	# dummy proc
}


Grapher/xgraph instproc dropratio {} {
	$self instvar inputfile_

	$self graph "Drop ratio (%)" "ratio (%)"
}

Grapher/xgraph instproc dropevent {} {
	$self instvar inputfile_ quiet_
       
        if {$quiet_} {
                return
        }
	file stat $inputfile_ t
	if {$t(size) == 0} {
		puts "\t $inputfile_: empty file"
		return
	}

        exec xgraph -bg white -bb -tk -nl -m -x "time" -y "event" -t "Drop events" $inputfile_ &

}

Grapher/xgraph instproc dropput {} {
	$self instvar inputfile_

        $self graph "Drop put"  "rate (%)" 
}

Grapher instproc dropratioT {} {
	$self instvar inputfile_

	set tmp [$self finishstate $inputfile_]
        puts "proto\tfid\tnodeid\ttime\tratio(%)"
	$self cat $tmp
 	$self graph  "Total Drop ratio (%)"  "ratio (%)"
}

Grapher instproc statistics {} {
	$self instvar inputfile_

	exec cat $inputfile_
}

Grapher instproc dropprecedenceT {} {
	$self instvar inputfile_ 

	set tmp [$self finishstate $inputfile_]
        puts "proto\tfid\tnodeid\ttime\tpacket "
	$self cat $tmp
	$self graph  "Drop precedence"  "packet"
}



Grapher/xgraph instproc receivepkt {} {
	$self instvar inputfile_

	$self coutingT
	$self graph  "received packet" "packet"
}

Grapher/xgraph instproc droppkt {} {
	$self instvar inputfile_

	$self coutingT
	$self graph  "Dropped packet" "packet"
}

Grapher/xgraph instproc sendpkt {} {
	$self instvar inputfile_

	$self coutingT
	$self graph  "Sent packet" "packet"
}


Grapher/xgraph instproc sendrate {} {
	$self instvar inputfile_

	$self graph "Sending rate" "rate (%)"
}


Grapher/xgraph instproc throughput {} {
	$self instvar inputfile_

	$self graph "throughput" "rate (%)"
}

Grapher/xgraph instproc throughputT {} {
	$self instvar inputfile_

	set tmp [$self finishstate $inputfile_]
	puts "proto\tfid\tnodeid\ttime\trate"
	$self cat $tmp
	$self graph "Total throughput" "rate (%)"
}

Grapher/xgraph instproc goodputT {} {
	$self instvar inputfile_

	set tmp [$self finishstate $inputfile_]
	puts "proto\tfid\tnodeid\ttime\tgoodput"
	$self cat $tmp
	$self graph "Total goodput" "rate (%)"
}

Grapher/xgraph instproc goodput {} {
	$self instvar inputfile_

	$self graph "Goodput" "rate (%)"
}


Grapher/xgraph instproc throughput_TSW {} {
	$self instvar inputfile_

	$self graph "TSW throughput" "rate (%)"
}


Grapher/xgraph instproc length {} {
	$self instvar inputfile_ quiet_
       
       if {$quiet_} {
                return
        }
	file stat $inputfile_ t
	if {$t(size) == 0} {
		puts "\t $inputfile_: empty file"
		return
	}
      exec xgraph  -bg white -bb -tk -x time -y "queue size" -t "Queue length " $inputfile_ &

}

Grapher/xgraph instproc pdFunction {} {
	$self instvar inputfile_ quiet_
       
       if {$quiet_} {
                return
        }
	file stat $inputfile_ t
	if {$t(size) == 0} {
		puts "\t $inputfile_: empty file"
		return
	}

      exec xgraph  -bg white -bb -tk  -ly 0,1 -x value -y "prob" -t "pdf" $inputfile_ &

}

Grapher/xgraph instproc cdFunction {} {
	$self instvar inputfile_ quiet_
       
       if {$quiet_} {
                return
        }
	file stat $inputfile_ t
	if {$t(size) == 0} {
		puts "\t $inputfile_: empty file"
		return
	}

      exec xgraph  -bg white -bb -tk -ly 0,1 -x value -y "prob" -t "cdf" $inputfile_ &

}


#----------------------------------------------------
Class Grapher/gnuplot -superclass Grapher


Grapher/gnuplot instproc plot { {arguments ""} {curvesToTrace ""}} {
	
	set flowslist [$self split-xgraph-file $inputfile]
	
	if { [array names opt_ "gnuplot_pre_command"] != "" } {
		set pre_command $opt_(gnuplot_pre_command)
	} else {
		set pre_command ""
	}
	set gnuplot_command "plot"
	
	if { $curvesToTrace != "" } {
		# eg : set curvesToTrace " cbr 1 / cbr 2 "
		set curves [split $curvesToTrace /]
	}
	
	set i 1
	foreach flow $flowslist {
		if { [info exists curves] != 0 && [lsearch -exact $curves "$flow"] < 0 } {
			continue
		}
		
		set gnuplot_command "$gnuplot_command \"$flow\""
		
		if { [array names opt_ "title.$flow"] != "" } {
			# eg : set opt(title.\ cbr\ 1\ ) "\"EF flows\""
			set gnuplot_command "$gnuplot_command title $opt_(title.$flow)"
		} else {
			set gnuplot_command "$gnuplot_command notitle"
		}
		
		set gnuplot_command "$gnuplot_command with lines"
		if { [array names opt_ "style.$flow"] != "" } {
			# eg : set opt(style.\ cbr\ 1\ ) 1
			set pre_command "$pre_command set linestyle $opt_(style.$flow) linetype $opt_(style.$flow) linewidth 1\n"
			set gnuplot_command "$gnuplot_command ls $opt_(style.$flow)"
		}
		
		set gnuplot_command "$gnuplot_command,"
		incr i
	}
	
	set idx [string last "," $gnuplot_command]
	if { $idx == [expr [string length $gnuplot_command] - 1]} {
		set gnuplot_command [string range $gnuplot_command 0 [expr $idx - 1]]
	}
	
	exec echo "gnuplot << !" > plot.sh
	
	if { $arguments != "" } {
		# eg : "set title 'titre';set xlabel 'x';set ylabel 'y'"
		exec echo "$arguments" >> plot.sh
	}
	
	exec echo "$pre_command" >> plot.sh
	exec echo "$gnuplot_command" >> plot.sh
	exec echo "pause $opt_(displaydelay)" >> plot.sh
	
	if { [array names opt_ "epsplot"] != "" } {
		# eg : set opt(epsplot) "set terminal postscript eps monochrome \"Helvetica\" 25"
		exec echo $opt_(epsplot) >> plot.sh
		exec echo "set output '$inputfile.eps'" >> plot.sh
		exec echo "replot" >> plot.sh
	}
	
	if { [array names opt_ "pngplot"] != "" } {
		# eg : set opt(pngplot) "set size ratio 0.7 0.7,0.7;set terminal png small color"
		exec echo $opt_(pngplot) >> plot.sh
		exec echo "set output '$inputfile.png'" >> plot.sh
		exec echo "replot" >> plot.sh
	}
	exec echo "!" >> plot.sh
	exec sh plot.sh
}

