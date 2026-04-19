# ScriptInterface Implementation Notes

## Status
Currently implemented as stub functions that log calls. Full implementation requires significant infrastructure.

## Required Infrastructure (from xNVSE)

### UserFunctionManager (Medium Complexity)
- Manages function call stack (prevents infinite recursion)
- Caches FunctionInfo for scripts
- Coordinates function execution
- Requires: Stack management, caching logic

### FunctionInfo (Simple-Medium Complexity)
- Parses function script bytecode to extract:
  - Parameter count and types
  - Parameter variable indices
- Creates event list for function variables
- Requires: Bytecode parsing, event list creation

### FunctionContext (Simple Complexity)
- Manages execution context for a single function call
- Holds event list, result, invoking script
- Handles lambda variable list management
- Complexity: Simple wrapper around event list

### LambdaManager (Medium Complexity)
- Identifies lambda scripts (inline functions)
- Manages parent event list sharing for lambdas
- Handles lambda cleanup
- Requires: Script analysis, reference tracking

### ScriptToken system (Complex)
- Represents script values (number, form, string, array)
- Type conversions between token types
- Memory management for tokens
- Complexity: Full type system with conversions

### ExpressionEvaluator (Complex)
- Evaluates NVSE expressions (arithmetic, comparisons, etc.)
- Handles variable lookups
- Manages evaluation state
- Complexity: Full expression parser/evaluator

## Implementation Order Recommendation
1. FunctionContext - Simple struct wrapper
2. FunctionInfo - Bytecode parsing is straightforward
3. UserFunctionManager - Stack + cache, manageable
4. LambdaManager - After FunctionInfo is working
5. ScriptToken system - Major undertaking
6. ExpressionEvaluator - Most complex, last

## Current Stub Functions
- CallFunction - Calls user-defined function scripts
- GetFunctionParams - Returns parameter count and types
- ExtractArgsEx - Extracts arguments without type checking
- ExtractFormatStringArgs - Extracts format string and arguments

## xNVSE Reference
See: C:\Users\djuil\Documents\robloxmod\fo3fose\NVSE-master\nvse\nvse\FunctionScripts.cpp
