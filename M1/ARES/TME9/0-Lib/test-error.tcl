#
# Copyright (c) 2008 The University of La Reunion - France
# Authors:   P. ANELLI
#
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

# creat an error model according to a list given in parameter

#puts "#Load error model"

Class ErrorModel/Rate -superclass ErrorModel

Class Error 

Error instproc init {error replication} {
	$self instvar emod_ type_ paramlist_ todo_ replication_ rvdelay_

	#puts "Error class: $error"
	instance-parameter $self $error
	set replication_ $replication

        #puts "        Create ErrorModel $type_ : $paramlist_"
	set emod_ [new ErrorModel/$type_]
	instance-parameter $emod_ $paramlist_
        if {$type_ == "List"} {
            set errno   [catch "$emod_ set actionlist_" msg]
            if {$errno == 0} {
                $emod_ droplist  $msg
            }
        }

	if {[info exists todo_]} {
	       if {$todo_ == "delay_pkt"} {
                   $emod_ set delay_pkt_ true
	  	   if {[info exists rvdelay_]} {
		       $self add-rvdelay [lindex $rvdelay_ 0] [lindex $rvdelay_ 1]
                   }
	       }
	} else {
		set todo_ "drop"
	}

	#puts "                               : $todo_ "
	 #create RNG 
        set randgen [new RNG]
        for {set i 1} {$i < $replication} {incr i} {
              $randgen next-substream
        }
        set randvar [new RandomVariable/Uniform]
        $randvar use-rng $randgen
        $emod_ ranvar $randvar
}


Error instproc print {fd} {
        $self instvar paramlist_ type_ todo_ emod_

        puts $fd "  ErrorModel $type_: $paramlist_"
        puts $fd "                 : $todo_"
        puts $fd "                 : [$emod_ set rate_]"

}


Error instproc get-emod {} {
	$self instvar emod_
	
	return $emod_
}

Error instproc get-type {} {
	$self instvar type_
	return $type_
}

Error instproc todo? {todo} {
	$self instvar todo_
	
	return "[expr {$todo_ == $todo}]"
}

Error instproc add-rvdelay {type paramlist} {
	$self instvar emod_ todo_ replication_

      # puts "  Add RV $type for delay_pkt: $paramlist"
     set randgen [new RNG]
     for {set i 1} {$i < $replication_} {incr i} {
              $randgen next-substream
     }
     set randomvar [new RandomVariable/$type]
     $randomvar use-rng $randgen
     instance-parameter $randomvar $paramlist
       	     
     $emod_ rvdelay $randomvar
}

Error instproc tracevar {filefd} {
	$self instvar emod_ bt_

        # BaseTrace: to write a string in channel
	set bt_ [new BaseTrace/Event]
	$bt_ attach $filefd
	$emod_ eventtrace $bt_
}
