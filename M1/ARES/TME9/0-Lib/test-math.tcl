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

#puts "#load test-math.tcl"

Class Math

Math instproc init {inputfile outputfile colx coly parameters} {
        $self instvar inputfile_ outputfile_ suffixdata_
        $self instvar colx_ coly_ AWK_
        $self instvar accuracy_
        
        set AWK_ "awk"

        set inputfile_ $inputfile
        set outputfile_ $outputfile
        set colx_ $colx
        set coly_ $coly
        
        instance-parameter $self $parameters

        set suffixdata_  [GetSuffix data]
}



Math instproc pdf {} {
	$self instvar inputfile_ AWK_ outputfile_
        $self instvar colx_ coly_ suffixdata_ accuracy_ 

#-------
	# prepare the data according to precision
set awk_accurate {
        BEGIN {
                nbligne = 0
              }
NF == 0 || /^\"/ || /^#/ {
        next
	}
{
	value = int($col/accuracy)*accuracy
	printf "%-8.5f\n", value
        nbligne++
}
END {}
}
       # compute the pdf
set awk_density {
BEGIN {
	slotvalue=-1; numbervalue= 0.0
     }
     {
       if (slotvalue == $1) 
       		numbervalue++
	else {
		if (slotvalue != -1) { 
                        printf "%8.5f\t%8.5f\n", slotvalue, numbervalue/nbrecord;
                }
		slotvalue=   $1
		numbervalue= 1.0
	}
     }
 END {
        printf "%8.5f\t%8.5f\n", slotvalue, numbervalue/nbrecord;
	printf "\n\n"
     }
}
#------
        puts "On file: $inputfile_"
        exec $AWK_ {$0~/[#"']/ {print $0}}  $inputfile_ >> $outputfile_
	#"
        set tmp [makefile  "tmpint" $suffixdata_]
        exec $AWK_ $awk_accurate accuracy=[expr 1.0*$accuracy]  col=$coly_ $inputfile_ >>$tmp
	#	find out the number of line
        set pat  {{^[ 0-9.+-]+$}}
        set cmdline "grep -E $pat $tmp | wc -l"
        #puts "$cmdline"
        if {![catch "exec $cmdline" nbline]} {
           #puts "     found nbline: $nbline"
	   exec sort -n -k 1 $tmp |  $AWK_ $awk_density nbrecord=$nbline  >>$outputfile_
         } else {
                exec echo "0  1"  >> $outputfile_
                exec echo "" >> $outputfile_
                exec echo "" >>$outputfile_
         }
         exec rm $tmp
}

Math instproc cdf {} {
        $self instvar inputfile_ AWK_ outputfile_
        $self instvar colx_ coly_ suffixdata_

set awk_code {
        BEGIN {
                cumul=0.0
              }
             {
               cumul++
               print $1, cumul/nbrecord
             }
        END {
              printf "\n\n"
        }
}
        set tmp [makefile  "tmpcdf" $suffixdata_]
        exec $AWK_ {$0~/[#"']/ {print $0; exit}}  $inputfile_ >> $outputfile_
        exec $AWK_ {$0!~/[#"']/ && $0!~/^$/ {print $field}} field=$coly_  $inputfile_ | sort -n >> $tmp
        #"
        #puts " $coly_ $tmp"
        set pat  {{^[ 0-9.+-]+$}}
        set cmdline "grep -E $pat $tmp | wc -l"
        #puts "$cmdline"
        if {![catch "exec $cmdline" nbline]} {
                exec $AWK_ $awk_code nbrecord=$nbline $tmp >>$outputfile_
        } else {
                exec echo "0 0 \n\n"  >> $outputfile_
        }
        #exec rm $tmp
}


Math instproc EWMA {} {
        $self instvar inputfile_ AWK_ outputfile_
        $self instvar colx_ coly_ suffixdata_ weight_
                
set awk_code {
        BEGIN {
                average =-1
        }
/^\"/ || /#/ {
        print $0
        average =-1
        next
      }
NF >0 {
                if (average == -1) {average=$coly}
                 average *= (1-alpha)
                 average += $coly*alpha;
                 print $colx, average
       }
NF ==0 {
        printf "\n"
       }
    END {
        }
}
        exec $AWK_ $awk_code  alpha=[expr 1.0*$weight_] colx=$colx_ coly=$coly_ $inputfile_ >>$outputfile_
}


Math instproc mean {} {
       $self instvar inputfile_ AWK_ outputfile_
       $self instvar colx_ coly_ suffixdata_
                        
set awk_code {
/^\"/ || /#/  {
        sum = 0.0
        nbrecord = 0
        next
        }
NF > 0 {
        nbrecord++
        sum += $coly
        x= $colx
        print x, sum/nbrecord
       }
NF ==0 {
        printf "\n"
       }
}
        exec $AWK_ $awk_code  colx=$colx_ coly=$coly_ $inputfile_ >>$outputfile_
}


Math instproc wmean {} {
       $self instvar inputfile_ AWK_ outputfile_
       $self instvar colx_ coly_ suffixdata_ weight_

set awk_code {
BEGIN {
        prev =-1
      }
function last () {
        if (prev != -1) {
               print prev, sum/sumw, sumw, sum
               prev= -1
        }
}

/#/ || /^$/ { last()
              print $0
              next
            }
  NF >= 3   {
             if (prev == $colx) {
                sum += $coly
                sumw += $weight
             } else {
                if (prev == -1) {
                        prev= $colx
                        sum = $coly
                        sumw = $weight
                } else {
                        print prev, sum/sumw, sumw, sum
                        prev =$colx
                        sum = $coly
                        sumw = $weight
                }
            }
           }
END {
        last()
}
}

exec sort -n -k $colx_ $inputfile_ | $AWK_ $awk_code colx=$colx_ coly=$coly_ weight=$weight_ >>$outputfile_

}
