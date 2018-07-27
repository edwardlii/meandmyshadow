Me and My Shadow Script API Reference
=====================================

(draft)

The script language is Lua 5.2 (later we may bump it to 5.3).

Always check `ScriptAPI.cpp` for the newest API changed unmentioned in this document.

How to edit script
==================

To edit the script of a block, right click the block and select "Scripting".

To edit the script of the level, right click the block and select "Scripting".

Currently the scenery block doesn't support scripting.

Available event types of block:

Event type            | Description
----------------------|--------------
"playerWalkOn"        | Fired once when the player walks on. (For example this is used in fragile block.)
"playerIsOn"          | Fired every frame when the player is on.
"playerLeave"         | Fired once when the player leaves.
"onCreate"            | Fired when object creates.
"onEnterFrame"        | Fired every frame.
"onPlayerInteraction" | Fired when the player press DOWN key. Currently this event only fires when the block type is TYPE_SWITCH.
"onToggle"            | Fired when the block receives "toggle" from a switch/button.
"onSwitchOn"          | Fired when the block receives "switch on" from a switch/button.
"onSwitchOff"         | Fired when the block receives "switch off" from a switch/button.

NOTE: During the event execution the global variable "this" temporarily points to current block. (Ad-hoc workaround!)
When the event execution ends the global variable "this" is reset to `nil`.

Available event types of level:

Event type | Description
-----------|--------------
"onCreate" | Fired when the level creates. This happens after all the blocks are created and their `onCreate` is called.
"onSave"   | Fired when the game is saved.
"onLoad"   | Fired when the game is loaded.
"onReset"  | Fired when the game is reset.

For the newest lists of event types, see `init()` function in `Functions.cpp`.

NOTE: the following methods to specify scripts can be used:

* Specify scripts for each events in the block script editing dialog.
* Only specify "onCreate" script in the block script editing dialog,
  and use setEventHandler() function in script to specify scripts for other events dynamically.
* Only specify "onCreate" script in the level script editing dialog,
  and use setEventHandler() function in script to specify scripts for other events for level/blocks dynamically.

Script API reference
====================

The "block" library
-------------------

### Static functions:

* getBlockById(id)

Returns the first block with specified id. If not found, returns `nil`.

Example:

~~~
local b=block.getBlockById("1")
local x,y=b:getLocation()
print(x,y)
~~~

* getBlocksById(id)

Returns the list of all blocks with specified id.

Example:

~~~
local l=block.getBlocksById("1")
for i,b in ipairs(l) do
  local x,y=b:getLocation()
  print(x,y)
end
~~~

### Member functions:

* moveTo(x,y)

Move the block to the new position, update the velocity of block according to the position changed.

Example:

~~~
local b=block.getBlockById("1")
local x,y=b:getLocation()
b:moveTo(x+1,y)
~~~

* getLocation()

Returns the position of the block.

Example: see the example for moveTo().

* setLocation(x,y)

Move the block to the new position without updating the velocity of block.

Example: omitted since it's almost the same as moveTo().

* growTo(w,h)

Resize the block, update the velocity of block according to the size changed.

NOTE: I don't think the velocity need to be updated when resizing block, so don't use this function.

Example: omitted since it's almost the same as setSize().

* getSize()

Returns the size of the block.

Example:

~~~
local b=block.getBlockById("1")
local w,h=b:getSize()
print(w,h)
~~~

* setSize(w,h)

Resize the block without updating the velocity of block.

Example:

~~~
local b=block.getBlockById("1")
local w,h=b:getSize()
b:setSize(w+1,h)
~~~

* getType()

Returns the type of the block (which is a string).

Example:

~~~
local b=block.getBlockById("1")
local s=b:getType()
print(s)
~~~

* changeThemeState(new_state)

Change the state of the block to new_state (which is a string).

Example:

~~~
local b=block.getBlockById("1")
b:changeThemeState("activated")
~~~

* setVisible(b)

Set the visibility the block.

NOTE: The default value is `true`. If set to `false` the block is hidden completely,
the animation is stopped, can't receive any event, can't execute any scripts (except for 'onCreate'),
can't be used as a portal destination,
doesn't participate in collision check and game logic, etc...

NOTE: This is a newly added feature.
If you find any bugs (e.g. if an invisible block still affects the game logic)
please report the bugs to GitHub issue tracker.

Example:

~~~
local b=block.getBlockById("1")
if b:isVisible() then
  b:setVisible(false)
else
  b:setVisible(true)
end
~~~

* isVisible()

Returns whether the block is visible.

Example: see the example for setVisible().

* getEventHandler(event_type)

Returns the event handler of event_type (which is a string).

Example:

~~~
local b=block.getBlockById("1")
local f=b:getEventHandler("onSwitchOn")
b:setEventHandler("onSwitchOff",f)
~~~

* setEventHandler(event_type,handler)

Set the handler of event_type (which is a string). The handler should be a function or `nil`.
Returns the previous event handler.

Example:

~~~
local b=block.getBlockById("1")
b:setEventHandler("onSwitchOff",function()
  print("I am switched off.")
end)
~~~

* onEvent(eventType)

Fire an event to specified block.

NOTE: The event will be processed immediately.

Example:

~~~
local b=block.getBlockById("1")
b:onEvent("onToggle")
~~~

NOTE: Be careful not to write infinite recursive code! Bad example:

~~~
-- onToggle event of a moving block
this:onEvent("onToggle")
~~~

* isActivated() / setActivated(bool) -- get/set a boolean indicates if the block is activated
  -- the block should be one of TYPE_MOVING_BLOCK, TYPE_MOVING_SHADOW_BLOCK, TYPE_MOVING_SPIKES,
  TYPE_CONVEYOR_BELT, TYPE_SHADOW_CONVEYOR_BELT.

* isAutomatic() / setAutomatic(bool) -- get/set a boolean indicates if the portal is automatic
  -- the block should be TYPE_PORTAL

* getBehavior() / setBehavior(str) -- get/set a string (must be "on", "off" or "toggle")
  representing the behavior of the block -- the block should be TYPE_BUTTON, TYPE_SWITCH

* getState() / setState(num) -- get/set a number (must be 0,1,2 or 3)
  representing the state of a fragile block -- the block should be TYPE_FRAGILE

* isPlayerOn() -- get a boolean indicates if the player is on -- only works for TYPE_BUTTON

* getPathMaxTime() -- get the total time of the path of a moving block

* getPathTime() / setPathTime(num) -- get/set the current time of the path of a moving block

* isLooping()/setLooping(bool) -- get/set the looping property of a moving block

The "playershadow" library
--------------------------

### Global constants:

* player

The player object.

* shadow

The shadow object.

### Member functions:

* getLocation()

Returns the location of player/shadow.

Example:

~~~
print(player:getLocation())
print(shadow:getLocation())
~~~

* setLocation(x,y)

Set the location of player/shadow.

Example:

~~~
local x,y=player:getLocation()
player:setLocation(x+1,y)
~~~

* jump([strength=13])

Let the player/shadow jump if it's allowed.

strength: Jump strength.

Example:

~~~
player:jump(20)
~~~

* isShadow()

Returns whether the current object is shadow.

Example:

~~~
print(player:isShadow())
print(shadow:isShadow())
~~~

* getCurrentStand()

Returns the block on which the player/shadow is standing on. Can be `nil`.

Example:

~~~
local b=player:getCurrentStand()
if b then
  print(b:getType())
else
  print(nil)
end
~~~

The "level" library
-------------------

### Static functions:

* getSize() -- get the level size

* getWidth() -- get the level width

* getHeight() -- get the level height

* getName() -- get the level name

* getEventHandler(event_type) -- get the event handler

* setEventHandler(event_type,handler) -- set the event handler, return the old handler

* win() -- win the game

* getTime() -- get the game time (in frames)

* getRecordings() -- get the game recordings

* broadcastObjectEvent(eventType,[objectType=nil],[id=nil],[target=nil])

Broadcast the event to blocks satisfying the specified condition.

NOTE: The event will be processed in next frame.

Argument name | Description
--------------|-------------
eventType     | string.
objectType    | string or nil. If this is set then the event is only received by the block with specified type.
id            | string or nil. If this is set then the event is only received by the block with specified id.
target        | block or nil. If this is set then the event is only received by the specified block.

Example:

~~~
level.broadcastObjectEvent("onToggle",nil,"1")
~~~

The "camera" library
--------------------

### Static functions:

* setMode(mode) -- set the camera mode, which is "player" or "shadow"

* lookAt(x,y) -- set the camera mode to "custom" and set the new center of camera

The "audio" library
-------------------

NOTE: the following functions are not going to work if the sound/music volume is 0.

### Static functions:

* playSound(name[,concurrent=-1[,force=false[,fade=-1]]])

Play a sound effect.

Argument name | Description
--------------|-------------
name          | The name of the sound effect. Currently available: "jump", "hit", "checkpoint", "swap", "toggle", "error", "collect", "achievement".
concurrent    | The number of times the same sfx can be played at once, -1 is unlimited. NOTE: there are 64 channels.
force         | If the sound muse be played even if all channels are used. In this case the sound effect in the first channel will be stopped.
fade          | A factor to temporarily turn the music volume down (0-128). -1 means don't use this feature.

Return value: The channel of the sfx. -1 means failed (channel is full, invalid sfx name, sfx volume is 0, etc.)

* playMusic(name[,fade=true])

Play a music.

Argument name | Description
--------------|-------------
name          | The name of the song.
fade          | Boolean if it should fade the current one out or not.

* pickMusic() - pick a song from the current music list.

* setMusicList(name_of_the_music_list) - set the music list.

* getMusicList() - get the music list.

* currentMusic() - get the current music.