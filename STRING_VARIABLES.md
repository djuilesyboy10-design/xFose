# String Variables

xFOSE introduces string variable support for Fallout 3 scripts, allowing modders to store and manipulate text data.

## Overview

String variables are stored in memory and identified by a unique integer ID. They persist across save/load operations, making them suitable for storing dynamic text data that needs to be preserved between game sessions.

## Commands

### sv_create

Creates a new string variable and returns its ID.

**Syntax:** `sv_create "string"`

**Parameters:**
- `string` - The initial text value for the string

**Returns:** The ID of the created string variable

**Example:**
```
set myStringID to sv_create "Hello World"
```

---

### sv_set

Sets the value of an existing string variable.

**Syntax:** `sv_set id "string"`

**Parameters:**
- `id` - The ID of the string variable to modify
- `string` - The new text value

**Returns:** 1 on success

**Example:**
```
sv_set myStringID "New text"
```

---

### sv_get

Gets the length of a string variable (used to verify the string exists).

**Syntax:** `sv_get id`

**Parameters:**
- `id` - The ID of the string variable

**Returns:** The length of the string, or 0 if the ID is invalid

**Example:**
```
set stringLen to sv_get myStringID
```

---

### sv_length

Returns the character count of a string variable.

**Syntax:** `sv_length id`

**Parameters:**
- `id` - The ID of the string variable

**Returns:** The number of characters in the string

**Example:**
```
set stringLen to sv_length myStringID
```

---

### sv_concat

Appends one string to another.

**Syntax:** `sv_concat targetID sourceID`

**Parameters:**
- `targetID` - The ID of the string to append to
- `sourceID` - The ID of the string to append

**Returns:** None

**Example:**
```
sv_concat myStringID otherStringID
```

---

### sv_compare

Compares two strings for equality.

**Syntax:** `sv_compare id1 id2`

**Parameters:**
- `id1` - The ID of the first string
- `id2` - The ID of the second string

**Returns:** 1 if the strings are equal, 0 if they are not

**Example:**
```
set isEqual to sv_compare myStringID otherStringID
```

---

### sv_is_valid

Checks if a string ID is valid (exists).

**Syntax:** `sv_is_valid id`

**Parameters:**
- `id` - The ID to check

**Returns:** 1 if the ID is valid, 0 if it is not

**Example:**
```
if sv_is_valid myStringID
    ; String exists
endif
```

---

## Usage Examples

### Basic String Creation and Manipulation

```
; Create a string
set playerName to sv_create "Vault Dweller"

; Modify the string
sv_set playerName "Lone Wanderer"

; Get the length
set nameLength to sv_length playerName

; Create another string
set greeting to sv_create "Hello, "

; Concatenate strings
sv_concat greeting playerName

; Check if equal
set isSame to sv_compare greeting "Hello, Lone Wanderer"
```

### Dynamic Text Building

```
; Build a dynamic message
set msgID to sv_create "You have "
sv_concat msgID itemCount
sv_concat msgID sv_create " items"
```

### Save/Load Persistence

String variables are automatically saved and loaded with the game. You can create strings in one session and they will persist after loading a save.

```
; Create a string
set questNote to sv_create "Quest started"

; Save the game
; (strings are saved automatically)

; Load the game
; (strings are loaded automatically)

; The string still exists
sv_is_valid questNote  ; Returns 1
```

---

## Notes

- String IDs are unique integers assigned by xFOSE
- Strings persist across save/load operations
- Strings are cleared when starting a new game
- Maximum string length is limited by the game's message buffer (typically 512 characters)
- Use `sv_is_valid` to check if a string ID exists before using it

---

## Plugin API

C++ plugins can also access string variables through the `FOSEStringVarInterface`:

```cpp
// Get the interface
auto strVarIntf = (FOSEStringVarInterface*)fose->QueryInterface(kInterface_StringVar);

// Create a string
UInt32 stringID = strVarIntf->CreateString("Hello", nullptr);

// Get string value
const char* value = strVarIntf->GetString(stringID);

// Set string value
strVarIntf->SetString(stringID, "World");
```

---

## Future Enhancements

Planned features for future xFOSE releases:
- Array variables for structured data storage
- String formatting commands
- String search and replace functions
- Script-level event handlers for string operations
