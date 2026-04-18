# TestEventPlugin

A simple test plugin to verify the FOSE Event Manager API works from external code.

## Purpose

This plugin queries the Event Manager interface and registers handlers for:
- OnHit
- OnDeath
- OnLoad
- OnEquip

When these events fire, the plugin logs messages to the FOSE log.

## Building

### Option 1: Add to existing FOSE solution
1. Add this folder as a new project to the FOSE solution
2. Configure as a DLL project
3. Add include path to `../fose/fose`
4. Link against `../fose/Debug/fose_common.lib` (or Release equivalent)
5. Set output directory to your Fallout 3\Data\FOSE\Plugins\ folder

### Option 2: Standalone build
1. Create a new Visual Studio DLL project
2. Add main.cpp
3. Add include path to FOSE headers (fose/fose directory)
4. Link against fose_common.lib
5. Build and copy DLL to Fallout 3\Data\FOSE\Plugins\

## Expected FOSE Log Output

```
TestEventPlugin: Query successful
TestEventPlugin: Load started
TestEventPlugin: Event Manager interface queried successfully (version 1)
TestEventPlugin: Event handlers registered successfully
TestEventPlugin: Load complete
TestEventPlugin: OnHit event fired! target=XXXXXXXX
TestEventPlugin: OnDeath event fired! target=XXXXXXXX
TestEventPlugin: OnLoad event fired! target=XXXXXXXX
TestEventPlugin: OnEquip event fired! target=XXXXXXXX
```

## Verification

After loading the plugin and playing the game, check fose.log for the TestEventPlugin messages. If you see the event handler messages, the Event Manager API is working correctly from external plugins.
