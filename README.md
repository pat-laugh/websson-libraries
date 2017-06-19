# WebSSON Libraries

This is a set of libraries related to the WebSSON language:

- `utils` contains useful functions
- `structures` contains WebSSON structures and keywords
- `parser` contains a WebSSON parser
- `serializer` contains a WebSSON serializer

Both the parser and serializer depend on the utils and structures libraries.

The other libraries are:

- `parserWebssHtml` contains a WebSS (superset of WebSSON) parser
- `serializerHtml` contains an HTML serializer (currently only HTML5)
- `serializerJson` contains a JSON serializer
- `serializerXml` contains an XML serializer

See the README files in each library for more information.

The directory `tests` contains functional tests for the Parser and the
Serializer.

This README will contain information about the libraries and short information
about the language itself.
[In-depth documentation](https://pat-laugh.ca/websson-doc/) is available.

## Installation

### External libraries

The parser uses the [curl](https://curl.haxx.se/) library to fetch web
documents. If you use Linux or Mac, it's possibly already installed; otherwise
it's pretty easy to install. In Windows, it's a lot more complicated... You can
disable parsing the WebSSON feature (imports) that requires that. See the parser's
[README](https://github.com/pat-laugh/websson-libraries/tree/master/parser#imports)
for more information.

### Windows

Most of the project was done using [Microsoft Visual
Studio](https://www.visualstudio.com/) 2015. If you have that installed, you can
clone the project, then open the solution file websson-libraries.

For each project that requires files from another project, go in their
properties, and then in the tab Configuration Properties -> C/C++ -> General.
For "Additional Include Directories", add the value "$(SolutionDir)" (values are
separated by semi-colons). The parser also requires the directory of the curl
library's header files if imports are not disabled.

The project for tests is the only one that produces an executable, so the only
one that needs to be linked. If the curl library is used, in the properties of
the tests project, go to the tab Configuration Properties -> Linker -> General,
and for "Additional Library Directories", add the directory of the curl library.
Then go to the tab Configuration Properties -> Linker -> Input, and for
"Additional Dependencies", add the name of the curl library (possibly "libcurl_a.lib").

If you have linker errors, make sure that the solution build targets the same
platform as the library (for instance, 32-bit with 32-bit).

### Linux and Mac

I only tried compiling with [GCC](https://gcc.gnu.org/). The Makefile in the
tests directory can compile the whole project. It assumes the curl include files
and library are in a location that the compiler and linker check by default.