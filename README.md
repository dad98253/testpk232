# testpk232
Program to help troubleshoot AX25 and KISS problems

I purchased an AEA Packrat back in the 1990s. At that time, I was running Red Hat 7 on a 386 PC. After spending days trying to get the AX25 to work on linux, I finally gave up. The Packrat worked just fine with my logging software on Windows 95, so I let it slide. Now that I am retired and have more free time, I decided that it was time to get this think working the way it was intended to (I.e., on linux). I naively thought that it must have been some linux bug and surely it would be corrected by now. 

Just like before, in normal "converse" mode, it worked just fine. But, when KISS is turned on, the PK232 went silent. I tried a lot of different things (thus the very messy condition of the code), but finally figured out that the Packrat expected RTS to be high when in KISS mode. (even though the KISS specification specifically states that there should be no flow control, hardware or software). I couldn't have done it without the help of this little program.

With all of the hurt that this caused me, I figured I'd post this out there just in case some other poor fool is going through similar anguish. It should work with any TNC that supports KISS.

Some of the code is semi-lifted from the linux mkiss driver (I wanted to make sure that it was de-encapsulating the PK232’s KISS data properly). So, those subroutines my require a GPL license. But, the rest is DWTFYWWI. 

Note that the code uses the appf2 library to do some housekeeping tasks. You can find a copy of the library here :  

What I found that the most useful feature, was the hex/ascii/shifted ascii dump mode. It's similar to the "TRACE" output on the PK232.

Anyway, folks... have fun!

73 DE W7OG (John)

Sample output:

############################################################################
Received: 2025-05-17 22:37:42
Byte                 Hex                  Shifted ASCII     ASCII
000: 0082a088 ae627060 96946e84 a4a6649c   APDW180KJ7BRS2N       bp`  n   d 
010: 6ab09aa8 40e2ae92 888a6440 6303f07d  5XMT qWIDE2 1 x>  j   @     d@c  }
020: 4b374949 493e4150 4b303034 2c544350  % $$$  (%    *!(  K7III>APK004,TCP
030: 49502c4b 4a374252 532d322a 3a3a4351  $( %% !))     !(  IP,KJ7BRS-2*::CQ
040: 20202020 2020203a 6465204a 61736f6e          22 %0977         :de Jason
050: 20434e38 377b3730                     !'  =             CN87{70        

call to APDW18-8  from KJ7BRS-3* (relayed via  N5XMT-0  WIDE2-0 )
Unnumbered frame (Unnumbered Information) with poll bit = 0 
content = }K7III>APK004,TCPIP,KJ7BRS-2*::CQ       :de Jason CN87{70
############################################################################
Received: 2025-05-17 22:38:55
Byte                 Hex                  Shifted ASCII     ASCII
000: 0082a0a6 82a44060 ac8a6ea6 8aac62ac   APSAR 0VE7SEV1V        @`  n   b 
010: 8a6ea698 86ee86a4 b2a6a898 e103f03d  E7SLCwCRYSTLp x    n             =
020: 34383237 2e36334e 2f313233 32322e36         '          4827.63N/12322.6
030: 36577630 2d312f30 30302f41 3d303030   +;               6Wv0-1/000/A=000
040: 3132315b 3a564537 534556                - +" )"+       121[:VE7SEV     

call to APSAR-0  from VE7SEV-6* (relayed via  VE7SLC-3* CRYSTL-12*)
Supervisory frame (Receive Ready) with poll bit = 0, rec seq. no. = 7
content = =4827.63N/12322.66Wv0-1/000/A=000121[:VE7SEV
############################################################################

