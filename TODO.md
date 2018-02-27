* **Move all water anim to TextureFrameTable to avoid many duplication of water animation handling code**

* Load and save functions

* Sound
* full screen movie sound is choppy

* use of memset is unsafe sometimes if structs are rearranged

* issues with stickling control _507B98_ctrl_pressed - stealing
- only used a couple of times remove?? OS_IfCtrlPressed instead??


* change how classes are loaded - seperate file for easy modifications?


* shop texts sometimes out of bounds

* Rename unidentifed variables
* cleanup functions
* Currently around 2600 warnings across the project
* consider making some struct members private??

*is spirit resistacne a thing or not


* resizing game windows - need to change use of co-ords to x/640 y/480 - normalised