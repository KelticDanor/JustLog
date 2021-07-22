# JustLog
JustLog General Plugin for the XMPlay audio player

# Description
Just a short and sweet plug-in to log files played by XMPlay.

# Change Log
v1.4
- If make new is not checked the existing file will just remove the oldest lines
- The old file doesn't have .s in the name anymore.

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
