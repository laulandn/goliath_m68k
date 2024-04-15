DAVLib for MacOS

Overview
--------
This library implements a simple object model, abstracting out the specifics of the 
WebDAV protocol and instead exposes a set of C++ classes which can be used for
web site content management.  Handling of XML documents which are passed as part
of the DAV protocol is also handled internally to the library.

Usage
-----
The main class in this library is CDAVRequest; this is where all of the 
high level operations (such as listDirectory, FindAllProperties, DeleteResource, 
etc) are defined.  Since this class utilizes the Networking classes found in 
PowerPlant, all DAV operations must be done in a thread.  However, this object
was designed to be re-entrant and can be used either in a single thread or across
multiple threads.  

Assuming that a CDAVRequest object (req) is in the scope of this code, here is a
sample snippet of code:

   CDAVItemVector kids;
   CDAVPropertyVector props;
   
   ...
   
   if (CDAVRequest::SUCCESS == req->ListDirectory(aThread, theResource, kids, props)) {
      //operate on the returned vector of DAV items
   } else {
      //handle the error gracefully....
   }	
 
Note that this library uses STL and most of the data types are templated.

PowerPlant Patches
------------------
   These classes use extensively modified versions of the PowerPlant networking
classes; these are found in the 'PowerPlant Patches' folder.  Thanks to i-drive.com
for providing these patches as well as much of the SSL support found in this version.
 
SSL Support
-----------
   Version 1.8 of DAVLib integrates OpenSSL to provide SSL support.  This option is 
configurable at compile time by defining the macro DAVLIB_SSL in your CodeWarrior
prefix file.  The library SSLGlue.Lib (contained in the DAVLib distribution) must
also be built and linked with any executable that uses DAVLib's SSL support.  Please
see the file 'SSL and PowerPlant' for more information on building and configuring
OpenSSL on the MacOS.  
   
License
-------
These classes are protected under the GNU General Public License, version 2.0.  
Contact the author to negotiate alternate licensing arrangements.

Also included with this library is J. Clark's Expat XML parser.  It's licensing 
information is included with that distribution.



Contact Information
-------------------
You can contact the author by e-mail at tbednarz@mediaone.net
or by mail at the following address

Tom Bednarz
19 Fisher Street
Natick, MA 01760
USA

Please report any defects discovered in this software.


Version History
---------------
1.0   - Sept 7, 1999.  Initial version. 
1.1   - Sept 28, 1999
1.1.1 - Sept 29, 1999
1.1.2 - Oct 10, 1999.  Relicensed under GPL.
1.5   - January 4, 2000.
1.5.1 - January 29, 2000.
1.6   - March 29, 2000
1.7   - July 16, 2000.
1.8   - October 21, 2000.
