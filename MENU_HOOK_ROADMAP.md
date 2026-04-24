# Menu Hook Implementation Roadmap

## Objective
Implement menu click detection by hooking UI button processing functions in Fallout3.exe.

## Approach
Trace from menu-related string push addresses to find the functions that process button clicks, then hook those functions using trampolines.

## Target Addresses (from string reference table)

### Priority 1: UI Button Action Hooks (Direct Click Detection)

These strings are pushed when UI buttons are clicked - hooking the functions that reference them will give us direct click detection.

1. **UIMenuOK** - Address: `0061E8B4`
   - String: "UIMenuOK"
   - Purpose: Detect OK button clicks in menus
   - Action: Trace xrefs, find function, hook

2. **UIMenuCancel** - Address: `0061E8F4`
   - String: "UIMenuCancel"
   - Purpose: Detect Cancel button clicks in menus
   - Action: Trace xrefs, find function, hook

3. **UIMenuMode** - Address: `0061E846`
   - String: "UIMenuMode"
   - Purpose: Detect menu mode changes
   - Action: Trace xrefs, find function, hook

4. **UIMenuFocus** - Address: `0061E8D0`
   - String: "UIMenuFocus"
   - Purpose: Detect menu focus changes
   - Action: Trace xrefs, find function, hook

5. **UIMenuPrevNext** - Address: `0061E8A8`
   - String: "UIMenuPrevNext"
   - Purpose: Detect Prev/Next button clicks
   - Action: Trace xrefs, find function, hook

### Priority 2: Menu Type Identification

These strings identify specific menu types - useful for filtering events by menu.

6. **InventoryMenu** - Address: `00BEC0E6`
7. **DialogMenu** - Address: `00BEC0AA`
8. **ContainerMenu** - Address: `00BEC096`
9. **MessageMenu** - Address: `00BEC136`
10. **MapMenu** - Address: `00BEC122`

## Implementation Steps

### Step 1: Hook UIMenuOK
1. Go to address `0061E8B4` in x64dbg
2. Find references to this string (xrefs)
3. Identify the function that uses this string
4. Analyze the function to find entry point and prologue
5. Record: function address, prologue end address, prologue bytes
6. Implement hook in Hooks_Menu.cpp
7. Test

### Step 2: Hook UIMenuCancel
Repeat process for `0061E8F4`

### Step 3: Hook UIMenuMode
Repeat process for `0061E846`

### Step 4: Hook UIMenuFocus
Repeat process for `0061E8D0`

### Step 5: Hook UIMenuPrevNext
Repeat process for `0061E8A8`

### Step 6: Create OnMenuClick Event
1. Define event in EventManager
2. Add event registration to PluginAPI
3. Combine data from all hooks into single event
4. Test with TestEventPlugin

## Notes
- Each hook will use the standard trampoline pattern (see Hooks_Gameplay.cpp for reference)
- Hook addresses are version-specific (Fallout 3 1.7)
- Need to determine prologue size for each function by analyzing assembly
- Event parameters will include: menu type, button type, tile info
