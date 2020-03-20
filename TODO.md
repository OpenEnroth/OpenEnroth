# TODO:

* Move all water anim to TextureFrameTable to avoid many duplication of water animation handling code*

* Quicksave / Quickload

* add short term / long term project goals - expand milestones

* spiders not masking skull texture?

* Rename unidentifed variables
* Remove all platform dependant fucntions

* is spirit resistacne a thing or not?

* Additional features:
- Harden party logic so that team size can be varied
- map editor? i think there was a semi complete example somewhere


* QOL improvements:
-  global merchant/ identify skills?


* Possible graphics improvements:
- keep track of light source intensity and add subtle flicker effect. proof of concept Engine/Graphics/Indoor.cpp line 115.



* Optimization targets:
- Currently engine fps is heavily CPU bound in directX mode. This is because of the amount of draw calls outside, optimized batching on draw terrain traingles will give FPS boost

