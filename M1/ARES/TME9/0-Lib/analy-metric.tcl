#
# Copyright (c) 2008 University of La Reunion - France
# Author:   Pascal ANELLI
# http://personnel.univ-reunion.fr/~panelli
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


puts "#load analy-metric.tcl"

source global.tcl
source test-math.tcl


Class Analysis

# filter: keep the ith index in datafile
Analysis instproc init {datafile prefix metrics  index parameters xy} {
        $self instvar metrics_ lastdata_ colx_ coly_  suffixdata_
        $self instvar test_ parameters_
        
        
        set metrics_ $metrics
        set test_ [format "%s%02d" $prefix $index] 
        
        set suffixdata_ [GetSuffix data]
        #$self default-parameter
        set parameters_ $parameters
        
        set lastdata_ [makefile $test_ $suffixdata_]
        set colx_ [lindex [split $xy] 0]
        set coly_ [lindex [split $xy] 1]


        # file to proceed : datafile
        if {[catch "exec grep -n {^$} $datafile | cut -f 1 -d : " linelist]} {
            puts "Inputfile is Only one block"
            exec cp -f $datafile $lastdata_
            return
        }
        # prepare the list of Number of line for each block
        set blockline [concat 1 [split $linelist]]
        lappend blockline [expr [lindex [exec wc -l $datafile] 0] +1]
        puts "-+-+>>>$blockline"
        
        # find the correct block
        set numbegin [lindex $blockline 0]
        set blocknum 0
        for {set i 1} {$i < [llength $blockline]} {incr i} {
                set numend [lindex $blockline $i]
                if { ([expr $numend - $numbegin] > 1) && ($blocknum == $index)} {
                        set numend [expr $numend - 1]
                         break
                }
                if {[expr $numend - $numbegin] > 1} {
                        incr blocknum
                }
                set numbegin  [expr $numend +1]
        }
        # do file to read in following : lastdata_
        if {$blocknum != $index} {
                puts "ERROR: Metrics on Test-analysis: Wrong number of Index"
                exit 1
        }
        #puts " CUT $datafile AT: $numbegin $numend"
        # cut the file
        set length [expr $numend - $numbegin ]
        exec head -n $numend $datafile | tail -n $length >$lastdata_
        puts "datafile for id $index : $lastdata_"
}


# Apply all the metrics on the datafile (lastdata_)
Analysis instproc MakeData {} {
        $self instvar metrics_ suffixdata_ colx_ coly_ lastdata_ parameters_
        $self instvar test_
        
        set log ""
        foreach i $metrics_ {
                puts "     Computing $i"
                set datafile [makefile [format "%s%s" $test_ $i] $suffixdata_]
                set math [new Math $lastdata_ $datafile $colx_ $coly_ $parameters_]
                $math $i 
                lappend log [list $i $datafile]
        }
	#exec rm -f $lastdata_
        return $log
}


######################################################################
# OBSOLETE
#-------------------------
# lastdata is a file with the name of the files to proceed
Analysis instproc bestoccurs {} {
        $self instvar test_ lastdata_ suffixdata_

      set fd [open $lastdata_ r]
      while {[gets $fd namefile] >= 0} {
                 set lb [$self DoRank $namefile]
                 foreach item $lb {
                          set case [lindex $item 0]
                          set score [lindex $item 1]
                          if {[info exists rank($case)]} {
                                    incr rank($case) $score
			  } else {
                                   set rank($case) $score
                          }
                 }
      }
      close $fd
      set nameoutfile [makefile [format "%sbo" $test_] $suffixdata_]
      set ofd [open $nameoutfile w]
      foreach item [lsort -integer [array names rank]] {
              puts $ofd "${item}\t$rank($item)"
      }
close $ofd
return $nameoutfile
}


Analysis instproc DoRank {namefile} {

        $self instvar colx_ coly_

         set colx [expr $colx_ - 1]
         set coly [expr $coly_ - 1]
        
        set tmp [makefile "tmp" "data"]
        exec sort -r -n -k $coly_ $namefile >>$tmp
        set fd [open $tmp r]
        set rank 0
        set start 1
        set rlist {}
        while {[gets $fd line] >=0} {
                set record [split $line]
                if {[string index [lindex $record 0] 0] == "#"} {
                        continue
                }
                if {$start} {
                        set start 0
                        set ref [lindex $record $coly]
                } else {
                      set nvalue [lindex $record $coly]
                      if {$nvalue < $ref} {
                        incr rank
                        set ref [lindex $record $coly]
                       }
                 }
                 lappend rlist [list [lindex $record $colx] $rank]
        }
        close $fd
        return $rlist
}



