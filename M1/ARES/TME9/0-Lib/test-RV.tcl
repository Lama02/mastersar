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

#puts "#load test-RV.tcl"

Class RV

RV instproc init {randomvar replication} {
        $self instvar distribution_ paramlist_ randvar_

        instance-parameter $self $randomvar

         #create RNG 
        set randgen [new RNG]
        for {set i 1} {$i < $replication} {incr i} {
              $randgen next-substream
        }
        set randvar_ [new RandomVariable/$distribution_]
        $randvar_ use-rng $randgen
        instance-parameter $randvar_ $paramlist_
}

RV instproc value {} {
        $self instvar randvar_

        return [$randvar_ value]
}

RV instproc get-generator {} {
        $self instvar  randvar_

        return $randvar_
}
