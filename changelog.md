Changelog

commit 812b79fba3f8e72b22a7035216b57b3f84b80603
Author: Scott Tabaka <tabaka@hoare7.cs.umsl.edu>
Date:   Mon Mar 9 15:11:22 2020 -0500

    added comments, completed README

 Makefile    |   4 +-
 README      |  24 ++++--
 bin_adder.c | 127 +++++++++++++++++-------------
 inputfile   |  70 +++++++++++++++--
 master.c    | 338 +++++++++++++++++++++++++++++---------------------------------------------------
 5 files changed, 282 insertions(+), 281 deletions(-)

commit 8f945a92c6b2caea4a57832c92e559b0a188cfd2
Author: Scott Tabaka <tabaka@hoare7.cs.umsl.edu>
Date:   Sun Mar 8 12:01:11 2020 -0500

    added 2nd sum method

 bin_adder.c |  38 +++++------
 master.c    | 286 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++---------------
 2 files changed, 253 insertions(+), 71 deletions(-)

commit cd035063a7da5c173481b10b5712597e53cbf14d
Author: Scott Tabaka <tabaka@hoare7.cs.umsl.edu>
Date:   Sat Mar 7 12:05:26 2020 -0600

    added log output and 1st sum function

 bin_adder.c | 113 ++++++++++++++++++++++++++++++++++++++++++------------------
 inputfile   |   0
 master.c    | 151 +++++++++++++++++++++-----------------------------------------------------------
 3 files changed, 120 insertions(+), 144 deletions(-)

commit 08f76cb7803b64dedcc9191ce39fed03b2b5bf6a
Author: Scott Tabaka <tabaka@hoare7.cs.umsl.edu>
Date:   Fri Mar 6 16:08:59 2020 -0600

    semaphores added

 Makefile    |   9 ++--
 bin_adder.c |  70 +++++++++++++++++++++-----
 master.c    | 221 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-------------------
 3 files changed, 231 insertions(+), 69 deletions(-)

commit 9af324bb65f9fd34127292254fe89eb7887c8934
Author: Scott Tabaka <tabaka@hoare7.cs.umsl.edu>
Date:   Wed Mar 4 15:22:02 2020 -0600

    Added clock,random num,input file

 bin_adder.c |  24 ++++-------
 inputfile   |   8 ++++
 master.c    | 185 ++++++++++++++++++++++++++++++++++++++++++++++++++++++--------------------------
 3 files changed, 142 insertions(+), 75 deletions(-)

commit 3b73acc6772f2402197c8abc31f068f24fa2f45d
Author: Scott Tabaka <tabaka@hoare7.cs.umsl.edu>
Date:   Tue Mar 3 10:35:30 2020 -0600

    Initial commit

 Makefile    |  23 ++++++++
 README      |  23 ++++++++
 bin_adder.c |  72 ++++++++++++++++++++++++
 master.c    | 244 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 4 files changed, 362 insertions(+)