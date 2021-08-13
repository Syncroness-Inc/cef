# CEF Coding Guidelines

Part of the CEF code will be written from scratch, part will be leveraged from existing projects, and part will be imported from external libraries.  As external code in particular will likely have maintenance updates, re-factoring code to meet a coding standard is not practical.  As such, CEF embraces that there will be different coding styles in the code base, but attempts to control the differences by setting an order of priority on how coding standards are enforced.

Priority of coding standards/styles

1. When modifying an existing file, code in the dominant style in the following priority
   * within the function
   * within the file
2. For new files, code in the dominant style in the following priority
   * within all the files in the sub-directory
   * within all the files in the parent directory
3. CEF Coding Amendments to the Syncroness Coding Guidelines (see below for the CEF Coding Amendments)
   * There are areas of the Syncroness Coding Guidelines that do not specify a specific style (example, CamelCase or snake_case).  In these cases the CEF amendments override the Syncroness Coding Guidelines.
   * There are cases where the majority of of the imported/re-used code runs counter to the Syncroness Coding Guidelines (example bool_t is specified in the Syncroness Coding Guidelines).  In these cases the CEF amendments shall override the Syncroness Coding Guidelines.
4. Syncroness Coding Guidelines

## CEF Coding Amendments to the Syncroness Coding Guidelines

In no particular order, the following rules make up the CEF Coding Amendments:

1. CamelCase rather than snake_case naming convention
2. Classes access specifier shall be listed in public, protected, private order
   1. Only one access specifier of each type shall be in each class
   2. Within an access specifier, the order listed shall be #define, enum, type declaration, functions, and then variables
3. The Syncroness Coding Standard header shall be modified to use the doxygen keyword "copyright"
   1. The copyright header shall be structured to be easily replaced by a text based "search and replace" or a python tool as it is anticipated that customers will require a customer specified header
4. Header guards will appear as the first (before the copyright notice), and last lines in the file
5. Four spaces, no tabs, per each level of indentation
6. Class names start with capital letters; everything else starts with lower case
7. Only one variable declaration is allowed per line in the source file
   1. Comments are allowed on the same line as the variable declaration
8. STL (standard template library), goto, friend and other such constructs that are typically not used in embedded software are only allowed if there is a robust explanation embedded in the code as to why these constructs are being used.  There are sometimes very good reasons to ease maintenance/increase code clarity to use these constructs, but the design intent needs to be captured.
9. "bool" rather than "bool_t"
10. "true" and "false" rather than "TRUE" and "FALSE" as this is dominant coding style of most imported code
11. If statements should always be of the form "if (src_size == 0)" for all variables except boolean.
12. All pointer names are prefixed with "p_" (e.g. p_src)
13. Class member variables shall be prefixed with "m_" (e.g. m_src).  If the member variable is a pointer, then "mp_".
14. Typedef shall have a suffix of "_t" (e.g. myType_t)
15. nullptr shall be used for C++, rather than NULL (convention of most C++ code being imported)
16. If there are units associated with an identifier, put the units in the variable name (e.g. missionSequenceMemorySizeBytes)
17. The first abstract class in an inheritance chain, should have a "_base" suffix to the class name
18. In C++ code, use of "static_assert" for compile time checking of limits is highly encouraged.  But, some compilers do not support the use of strings in the static assert, so a made up, undefined variable that describes the error is used instead.
19. A theory of operation is encouraged at the start of either header or source files to describe complex implementations.  The theory of operation may refer to a .md document, but code specific implementation should be kept in the source files to ease maintenance.
20. Each function in the header file shall have a description, @param, and @return section.
21. Getters/Setters are strongly encouraged rather than direct member access of class public variables except where it is too cumbersome.
22. Static variables should be commented with extra care, but no special naming convention of static variables shall be used (to ease maintenance)
23. As it is anticipated copyright headers will need to be replaced with customer specific header files, the doxygen keyword "copyright" shall be used in conjunction with a consistent copyright header to facilitate simpler header replacement (ideally with search and replace, likely with a python script)
24. The copyright header shall be the first line in all header files.  The first word on the second line of the header file shall start with the doxygen command "\copyright".  This not only aids doxygen, but simplifies a tool to replace all copyright headers in the system.
25. Some compilers pull in a "huge" amount of code when a pure virtual function is used.  Hence, a stub with a `LOG_FATAL`is put in place rather than have a `exampleFunction() = 0;` in order to keep code size small on all implementations.  Virtual functions that should be implemented are at least caught during run time execution this way.
26. In general, avoid abbreviations and use whole words instead (this eases a faster learning curve as one does not have to learn project specific abbreviations in order to read the code).
27. When specifying include files, System/Compiler specific files use `#include <filename>` where as project specific files use `#include "filename"`.  Avoid using path names in include files (path names should be in the make/build system to support moving files in the directory structure without breaking the build).
