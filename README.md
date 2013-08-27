list
====
The objective of the program is to sort a given list of bank transaction and print it out in an expected format

Files present:
listtest.c
my402List.c
warmup1.c
Makefile(warmup)

my402List.h
cs402.h

Assumptions:
The input file needs to have the following specification. Failing to meet any of the below mentioned expectations will make the program to exit with the appropriate error.

1>Should provide exactly 4 fields in the following order
	a>The type of transaction; has to be either “+” or “-“

b>The timestamp, has to be only digits, and less than 11.

c>The amount, has to have only two digits after the decimal point, anything lesser or greater will result in an error. The amount value should be less than 10 million, if not the program will error out with an appropriate error message.

d>Description: If empty will result in error, if filled with spaces and no description, results in an error. If filled with spaces before the actual description, then the initial spaces will be truncated and just the actual description will be stored.

Code:
Files my402list.c and warmup1.c has been written from scratch.
Few codes that were googled from forums are:
a>To check file status and print if it’s a directory
b>sscanf to take the amount in integer form to maintain precision.

Method used to parse input file: 
Function strtok() has been used to search for the delimiter 

Method used to sort:
Input is sorted while insertion.

Testing:
Tested with the grading guideline scripts.
Tested with malformed inputs and failing to meet the conditions mentioned above in the Input file description.
Erroneous command line input.
