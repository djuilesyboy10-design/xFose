# Arrays

xFOSE introduces array support for Fallout 3 scripts, allowing modders to store and manipulate collections of data.

## Overview

Arrays are ordered collections of elements stored in memory and identified by a unique integer ID. They support integer, float, and string element types. Arrays persist across save/load operations, making them suitable for storing dynamic data that needs to be preserved between game sessions.

## Commands

### ar_create

Creates a new array and returns its ID.

**Syntax:** `ar_create`

**Parameters:** None

**Returns:** The ID of the created array

**Example:**
```
set myArrayID to ar_create
```

---

### ar_set

Sets an element at a specific index in an array.

**Syntax:** `ar_set arrayID index value`

**Parameters:**
- `arrayID` - The ID of the array
- `index` - The index to set
- `value` - The integer value to set

**Returns:** 1 on success, 0 on failure

**Example:**
```
ar_set myArrayID 0 42
```

---

### ar_get

Gets an element from an array at a specific index.

**Syntax:** `ar_get arrayID index`

**Parameters:**
- `arrayID` - The ID of the array
- `index` - The index to get

**Returns:** The value at the specified index, or 0 if invalid

**Example:**
```
set value to ar_get myArrayID 0
```

---

### ar_size

Returns the number of elements in an array.

**Syntax:** `ar_size arrayID`

**Parameters:**
- `arrayID` - The ID of the array

**Returns:** The number of elements in the array

**Example:**
```
set arraySize to ar_size myArrayID
```

---

### ar_push

Pushes an element to the end of an array.

**Syntax:** `ar_push arrayID value`

**Parameters:**
- `arrayID` - The ID of the array
- `value` - The integer value to push

**Returns:** 1 on success, 0 on failure

**Example:**
```
ar_push myArrayID 100
```

---

### ar_pop

Removes and returns the last element from an array.

**Syntax:** `ar_pop arrayID`

**Parameters:**
- `arrayID` - The ID of the array

**Returns:** The popped value, or 0 if invalid

**Example:**
```
set poppedValue to ar_pop myArrayID
```

---

### ar_remove

Removes an element at a specific index from an array.

**Syntax:** `ar_remove arrayID index`

**Parameters:**
- `arrayID` - The ID of the array
- `index` - The index to remove

**Returns:** 1 on success, 0 on failure

**Example:**
```
ar_remove myArrayID 2
```

---

### ar_clear

Removes all elements from an array.

**Syntax:** `ar_clear arrayID`

**Parameters:**
- `arrayID` - The ID of the array

**Returns:** 1 on success, 0 on failure

**Example:**
```
ar_clear myArrayID
```

---

### ar_is_valid

Checks if an array ID is valid (exists).

**Syntax:** `ar_is_valid arrayID`

**Parameters:**
- `arrayID` - The ID to check

**Returns:** 1 if the ID is valid, 0 if it is not

**Example:**
```
if ar_is_valid myArrayID
    ; Array exists
endif
```

---

## Usage Examples

### Basic Array Operations

```
; Create an array
set myArray to ar_create

; Push elements
ar_push myArray 10
ar_push myArray 20
ar_push myArray 30

; Get the size
set size to ar_size myArray  ; Returns 3

; Get an element
set value to ar_get myArray 0  ; Returns 10

; Remove an element
ar_remove myArray 1

; Clear the array
ar_clear myArray
```

### Stack Operations

```
; Use an array as a stack
set stack to ar_create

ar_push stack 100
ar_push stack 200
ar_push stack 300

; Pop elements (LIFO)
set val1 to ar_pop stack  ; Returns 300
set val2 to ar_pop stack  ; Returns 200
set val3 to ar_pop stack  ; Returns 100
```

### Dynamic List Management

```
; Build a dynamic list of item IDs
set itemList to ar_create

ar_push itemList itemID1
ar_push itemList itemID2
ar_push itemList itemID3

; Process all items
set i to 0
set count to ar_size itemList
while i < count
    set currentItem to ar_get itemList i
    ; Process currentItem
    set i to i + 1
loop
```

### Save/Load Persistence

Arrays are automatically saved and loaded with the game. You can create arrays in one session and they will persist after loading a save.

```
; Create an array
set questItems to ar_create
ar_push questItems 100

; Save the game
; (arrays are saved automatically)

; Load the game
; (arrays are loaded automatically)

; The array still exists
ar_is_valid questItems  ; Returns 1
```

---

## Notes

- Array IDs are unique integers assigned by xFOSE
- Arrays persist across save/load operations
- Arrays are cleared when starting a new game
- Arrays support integer elements (float and string support planned for future versions)
- Array indices are zero-based (first element is at index 0)
- Use `ar_is_valid` to check if an array ID exists before using it
- `ar_push` adds to the end of the array, `ar_pop` removes from the end

---

## Plugin API

C++ plugins can also access arrays through the `FOSEArrayVarInterface`:

```cpp
// Get the interface
auto arrayIntf = (FOSEArrayVarInterface*)fose->QueryInterface(kInterface_ArrayVar);

// Create an array
UInt32 arrayID = arrayIntf->CreateArray(nullptr);

// Get array size
UInt32 size = arrayIntf->GetArraySize(arrayID);

// Set an element
arrayIntf->SetArrayElement(arrayID, 0, 42);

// Get an element
UInt32 value;
if (arrayIntf->GetArrayElement(arrayID, 0, &value))
{
    // value now contains the element
}

// Check if valid
if (arrayIntf->IsArrayValid(arrayID))
{
    // Array exists
}
```

---

## Future Enhancements

Planned features for future xFOSE releases:
- Float element support
- String element support
- Multi-dimensional arrays
- Array sorting functions
- Array search functions
- Array concatenation
- Array slicing
