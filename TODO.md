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
  - Currently engine fps is typically CPU bound - Any scope/need for threading?

* Minor Projects:
  - Remove zdrawtexturealpha
  - Reduce logger spam
  - OpenGL shader structs - member types and padding

* Major Projects:
  - New GUI/ window system
  - New sounds system
  - New (.WMM)?? save format with extended capabilites??
  - Check WOMM features all fixes of GrayFace patches
  - Add all GrayFace patch enchancements to WOMM
  - Lua scripting
  - Debug logger improvements