Port of Goliath 1.0.0 to m68k classic MacOS
===============

https://macintoshgarden.org/apps/goliath-101

Includes all needed dependencies.

For CodeWarior Pro 6.

+ Includes hacked "MacOS Support" and "MSL" from CodeWarrior 8.
(Needed to build the application)
+ Includes hacked "MacOS Support" from CodeWarrior 6.
(Needed to build OpenSSL libraries)

+ Includes hacked OpenSSL 0.9.6f.

For MacOS 7.5.0+.

Does NOT fully support Unicode, as original source used MacOS 8.5+ system library, which never existed for m68k Macs (as far as I know).

+ Requires Appearance Manager (Will not launch or crash at start without).
+ Requires OpenTransport (Starts, but not functional without).
+ Requires Internet Config 2.0+ (Will warn if not avail).
