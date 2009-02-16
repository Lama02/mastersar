#!/usr/bin/tclsh
#
# Copyright (c) 2007 University of La Reunion - France
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


#puts "#load makeres.tcl"

source global.tcl

proc writeset {name value} {
        global fd
        puts $fd "set opt($name) [llist $value]"
}

proc changeparam {name value line} {

        set reg "\{${name}\[^\}\]+\}"
        if {![regexp -indices "$reg" $line ind]} {
        	#puts "Changeparam: $name NOT FOUND"
		return $line
        }
        #puts "Input: $line"
        #puts "\tFind: ${name}"
        #puts "\tFound:[string range $line [lindex $ind 0] [lindex $ind 1]]"
        #puts "\tChange to $value"
        set inf [expr [lindex $ind 0] ]
        set sup [expr [lindex $ind 1] -1]
        set oline [string range $line 0 $inf]
        append oline "$name $value" [string range $line [lindex $ind 1]  end] 
       #puts "Changeparam: Output line ->\n\t$oline"
        return $oline
}


proc replaceres {tab savename args} {
        upvar $tab param
	if {[isdebug]} {
		puts " Debug: replaceres"
	}

        if {![file exists $param(f)]} {
                puts "ERROR: replaceres - No ressource file given"
                exit 1
        }
	set suffixrc [GetSuffix ressource]
	set tmp [format "%s.%s" $savename $suffixrc]
        catch "exec rm -f $tmp"

        if {[llength $args]} {
                set specificfunction [lindex $args 0]
                set arguments [lindex $args 1 end]
         } else {
                set specificfunction {}
         }
         
        set ofd [open  $tmp w]
        set ifd [open $param(f)]
        while {[gets $ifd line] >= 0} {
               foreach name [array names param] {
                        if {$name == "f"} continue
                        set oline [changeparam $name $param($name) $line]
                        if {[string compare "$oline" "$line"] != 0} {
                                #   unset param($name)
                                set line $oline
                        }
               }
               if {$specificfunction != ""} {
                        set line [$specificfunction $line $arguments]
               }
               puts $ofd $line       
        }
        close $ifd
        close $ofd

       return $tmp
}

proc adaptres {nameres savename configlist} {

        if {![file exists $nameres]} {
                puts "ERROR: No ressource file given "
                puts "[info level [info level]]"
                exit 1
        }

        set suffixrc [GetSuffix ressource]
	set tmp [format "%s.%s" $savename $suffixrc]
        catch "exec rm -f $tmp"
        set ofd [open  $tmp w]
        set ifd [open $nameres]
        while {[gets $ifd line] >=0 } {
                 set ll [split $line]
                 set nl {}
                 foreach item $ll {
                        foreach confitem $configlist {
                                set key [lindex $confitem 0]
                                if {$key == $item} {
                                        set item [lindex $confitem 1]
                                        break
                                }
                        }
                        lappend nl $item
                 }
                 puts $ofd "[join $nl]"
        }
        close $ifd
        close $ofd

        return $tmp
}


#=======================================
##    Specific function
#=======================================

proc startsource {line args} {
    
    set starttime [list $args]
    #puts "startsource: $starttime"
    if {[regexp "starttime" $line]} {
        regsub {^(.*\{)(starttime )([0-9]+|\{[0-9 e\-+\.]*\})(\}.*\\)} $line "\\1\\2${starttime}\\4" line
    }
    
    return $line
}



