# xmp-JustLog
JustLog General Plugin for the XMPlay audio player

# Description
Just a short and sweet plug-in to log files played by XMPlay.

# Change Log
v1.9
- Compiled with /MT flag to remove runtime dependencies
- Relative paths should now always work relative to XMPlay.exe
- Config now has a path display and test button to show view relative paths easier

v1.8.2
- Added %4 tag for date
- Added %5 tag for track
- Added %6 tag for genre
- Added %7 tag for comment
- Added %n tag for new line
- Fixed "Exclude untitled" option crash
- String format length is now 125 characters up from 50
- Typo in the "Exclude untitled" conditional check
- Version number in About & DLL corrected

v1.7
- Added %z tag for stream title if present

v1.6
- New option to remove path from %0 filename
- New option to include even if there is no title

v1.5
- %0 filename added to string formatting
- Tab order fixed a little bit

v1.4
- If make new is not checked the existing file will just remove the oldest lines
- The old file doesn't have .s in the name anymore

v1.3
- Added separate string for stream formatting
- Changed the max file from MB to KB
  
v1.2
- Added option to ignore streams
- Added option to ignore local files
- Added option to ignore specific file types MP3;FLAC;MOD and so on
- Changed the way the plug-in worked, originally it worked like the MSN plug-in using a hook to catch title changes
  Instead it now uses the proper NewTrack and NewTitle plug-in events.

v1.1
- Fixed bugs in the max size setting not working properly

v1.0
- Initial Release
