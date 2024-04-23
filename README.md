Port of Goliath 1.0.0 to m68k classic MacOS
===============

https://macintoshgarden.org/apps/goliath-101

Very nice, WebDAV client, drag and drop from "Finder" like windows.

For CodeWarior Pro 6.

Includes all needed dependencies.

+ DAVLib
+ Includes hacked "MacOS Support" and "MSL" from CodeWarrior 8.
(Needed to build the application)
+ Includes hacked "MacOS Support" from CodeWarrior 6.
(Needed to build OpenSSL libraries)
+ OpenSSL 0.9.6f (slightly hacked to get working)


Does NOT fully support Unicode, as original source used MacOS 8.5+ system library, which never existed for m68k Macs (as far as I know).

May not fully support non-standard scripts/encoding for same reason.

For MacOS 7.5.0+.

+ Requires Appearance Manager (Will not launch or crash at start without).
+ Requires OpenTransport (Starts, but not functional without).
+ Requires Internet Config 2.0+ (Will warn if not avail).

NOTE: There is a bug that will cause files to have generic looking icons on some systems that I have not fully tracked down.