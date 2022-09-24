# General TODO list

* Move all water anim to `TextureFrameTable` to avoid many duplication of water animation handling code.

* Implement Quicksave / Quickload.

* Add short term / long term project goals - expand milestones.

* Are spiders not masking skull texture?

* Rename unidentifed variables.
* Remove all platform-dependant functions.

* Is spirit resistance a thing or not?

* Additional features:
  - Harden party logic so that team size can be varied.
  - Map editor? i think there was a semi complete example somewhere.
  - Fog should reduce ability of party to autoaim
  - 'Of light' items which add glow around party

* QoL improvements:
  - global merchant / identify skills?

* Possible graphics improvements:
  - keep track of light source intensity and add subtle flicker effect. proof of concept Engine/Graphics/Indoor.cpp line 115.

* Optimization targets:
  - Currently engine fps is heavily CPU bound in directX mode. This is because of the amount of draw calls outside, optimized batching on draw terrain traingles will give FPS boost.

* Minor Projects:
  - Remove zdrawtexturealpha
  - Reduce logger spam
  - Drop all software mode rules code

* Major Projects:
  - New GUI/ window system
  - New sounds system
  - New (.WMM)?? save format with extended capabilites??
  - Check WOMM features all fixes of Grayface patches
  - Add all Grayface patch enchancements to WOMM
  - Lua scripting
  - Debug logger improvements