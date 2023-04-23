# /tool wrapper

## introduction
a wrapper is a stand-in for another program that makes some decisions or adjustments before exec()ing the other program.

## configuration requirements

This program should be compiled c or c++ code.

This program should take the argv vector and use the first element (the program name called) to determine:

1. The program_name (filename portion)
2. The program_path directory (directory portion)

This program should look for an environment variable called WRAPPER_PATH.  It should

* split it on ':' (colon)
* append the program_path determined above with 'bin' replaced with 'config'
* throw away elements that are not directories that exist and are readable
* save the resulting list in wrapper_path.

This program should then look in each element of wrapper_path for a plain readable file called program_name.

This program should open the first program_name found, and read it for a line beginning with PATH=.  If found, it should take the bit after '=' but before any ' #' as 