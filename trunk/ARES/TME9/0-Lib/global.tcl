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

#puts "#load global.tcl"
#------------
set lastfid 0
set debug 0
# output suffix file
set env(suffix)    {  {pkt pkt} \
                      {trace tr} \
		      {param prm} \
		      {nam nam} \
		      {data data} \
		      {gnuplot gpl} \
		      {xgraph  xgr} \
                      {log     log} \
                      {ressource rc} \
                    }
			
#-------------

#Object instproc set-opt { name value } {
#	eval "\$self instvar ${name}_"
#	set key 1
#	set keyName ""
#	foreach elt $value {
#		if { $key } {
#			set keyName $elt
#			set key 0
#		} else {
#			eval "set $name{_}($keyName) $elt"
#			set key 1
#		}
#	}
#}

#Object instproc set-default-value { name value } {
#	$self instvar opt_
	
#	if { [array names opt_ $name] == "" } {
#		set opt_($name) $value
#	}
#}

proc puts-error {msg} {
        puts stderr "ERROR: $msg"
        exit 1
}

proc getopt {tab argc argv} {
upvar $tab opt

     for {set i 0} {$i < $argc} {incr i} {
                set arg [lindex $argv $i]
                if {[string range $arg 0 0] != "-"} continue

                set name [string tolower [string range $arg 1 end]]
                set opt($name) [lindex $argv [expr $i+1]]
                puts "$tab\($name\): $opt($name)"
     }
}


proc rtt {src dst meanpktsize} {

	set ns [Simulator instance]

	set routingTable [$ns get-routelogic]
        set srcid  [$src id]
#	puts "src id: $srcid"
        set dstid [$dst id]
#	puts "dst id: $dstid"
	
        if {[Agent/TCP set useHeaders_]} {
                set use 2.0
        } else {
                set use 1.0
        }
        set delay 0.0
        set tmpid $srcid
        while {$tmpid != $dstid} {
                set nextid [$routingTable lookup $tmpid $dstid]
                set tmpnode [$ns get-node-by-id $tmpid]
                set nextnode [$ns get-node-by-id $nextid]
                set tmplink [[$ns link $tmpnode $nextnode] link]
                set delay [expr $delay + [expr 2.0 * [$tmplink set delay_]]]
                set delay [expr $delay + [expr ($meanpktsize+ (40*$use))*8 / [$tmplink set bandwidth_]]]
                set tmpid $nextid
        }
	return $delay
}


proc instance-parameter {instance parameters} {

	if {[isdebug]} {
		puts "instance-parameter: $parameters; number of items: [llength $parameters]"
	}
    foreach opt $parameters {
	if {[llength $opt] != 2} {
		puts-error "in [info level] of instance-parameter function : Wrong number for \
		parameter: $opt \nExtracted from list: $parameters; number of items: [llength $parameters]"
	}
	set name [lindex $opt 0]
	append name "_"
	set val [lindex $opt 1]


	$instance set $name $val
   }
}

# return a value of element at index or the value of the last element in list values
proc value {index values} {
	set lgvalue [expr [llength $values] -1]
	if {$index >  $lgvalue} {
		set temp [lindex $values $lgvalue] 
	} else {
		set temp [lindex $values $index]
	}
	if {[isdebug]} {
		puts "Value: $temp; list: $values number of items: [llength $values]"
	}

	return $temp
}

# return a pair of value of element at index in list values
proc lvalue {index values} {
	set temp [value $index $values]
	if {[llength $temp] == 1} {
		lappend temp $temp
	}
	return $temp
}


# return the value of parameter "name" from the list "parameters"
proc find-parameter {parameters name} {
	set index [lsearch -regexp $parameters $name]
	if {$index == "-1"} {
		puts-error "in find parameter: $name not found in $parameters"
	}
		
	set listparam [lindex $parameters $index]
	return [lindex $listparam 1]

        #  foreach item $list {
        #        if {[lsearch -exact $item $pat] != -1 } {
        #                set value [lindex $item 1]
        #         }
        #  }
        #     return $value
}


# Return a list of all the subclass of class test
proc subclasslist {test} {
	set lclass $test
	foreach t [$test info subclass] {
		set lclass [concat $lclass [subclasslist $t]]
	}
	return $lclass
}

# return the last name after "/" in of the object test
proc GetClassName {test} {
	set lclass [$test info class]
	return [lindex [split $lclass "/"] end]
}


proc isdebug {} {
	global debug
	
	return $debug
}

# return a writable form of a sorted list
proc firstlast {llist} {
	return [list [lrange $llist 0 0] "-" [lrange $llist end end]]
}


proc usage {argv0 args} {
        puts stderr "usage: ns $argv0 -<option> <value> ..."
        puts stderr "      $args"
        exit 1
}

# read a list and build the equivalent index list
proc castall {len} {

    set cast {}
    #puts "$len"
    for {set i 0} {$i < $len} {incr i} {
        lappend cast $i
    }
    return $cast
}




#-------------------------------
#  File Management
#-------------------------------

proc name-prefix {prefix id1 id2} {

	set name ""
	append name $prefix "_" $id1 "_" $id2

	return $name
}


proc makefile {prefix suffix} {
        set tmp [format "%s.%s" $prefix $suffix]
       
        if {[file exists $tmp]} {
              exec rm $tmp
         }
        exec touch $tmp 

        return $tmp
}

proc file-rm {pat} {
        set lfile [glob -nocomplain "$pat"]
        foreach item $lfile {
                exec rm -f $item
        }
}

# save wd/filename in sd/saveprefix
proc file-cp {wd sd  filename saveprefix} {
       
 	if {$filename == {}} {
		return
	}
	#puts "Ask to save $filename"
        if {[file exists ${wd}/[file tail $filename]]} {
	    set suffix [file extension $filename]
	    set savefile [format "%s/%s%s" $sd $saveprefix $suffix]
            exec cp -f ${wd}/[file tail $filename] $savefile
                #puts "\tSave        : ${wd}/[file tail $filename]"
                #puts "\t     in file: $savefile"
		return $savefile
	} else {
		puts "\tERROR       : $filename not found"
		return {}
	}
}


# move file in sd directory
proc file-mv {file sd} {
	if {$file == {}} {
		return
	}
	if {[file exists $file]} {
	    	exec mv -f $file $sd/[file tail $file]
	    	#puts "\tCopy file in  : $sd/[file tail $file]"
                return [list $sd/[file tail $file]]
	} else {
		puts "\n\tERROR     : $file not FOUND"
                return {}
	}
}


proc GetSuffix {filetype} {
	global env
	
	return [find-parameter  $env(suffix) $filetype]
}


