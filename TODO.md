# TODO:

* Move all water anim to TextureFrameTable to avoid many duplication of water animation handling code*

* Quicksave / Quickload

* Sound refactor - transfer to OpenAL - remove MSS (AIL)
* music tracks not realeased when stopped - memory leak
* full screen movie sound is choppy - suspect reading incorrect amounts of bits - AV_SAMPLE_FMT_FLT -leaving gaps in suadio stream??

* use of memset is unsafe sometimes if structs are rearranged
* issues with stickling control _507B98_ctrl_pressed - stealing (only used a couple of times remove?? OS_IfCtrlPressed instead??)

* add how to help section on readme
* add short term / long term project goals


* shop texts sometimes out of bounds
* resizing game windows - need to change use of co-ords to x/640 y/480 - normalised
* update file paths using "MakeDataPath"



* bloodsplats - not working outdoors
* yellow flickering on indoor levels
* spiders not masking skull texture?

* npc walking over water - cleanup


* clicking on asleep char cycles

* Rename unidentifed variables
* cleanup functions + refactor
* Remove all platform dependant fucntions

* is spirit resistacne a thing or not?



