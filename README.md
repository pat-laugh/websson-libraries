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

All libraries except structures require the library [various](https://github.com/pat-laugh/various-cpp).

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
library's header files if imports are not disabled. The location of the library
various also needs to be added like this.

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

For the library various, you can make a symbolic link to it.

## The language

This is a short introduction to WebSSON. I'll make comparisons with JSON since
that's probably well-known by anybody reading this.

WebSSON allows the same structures as JSON, and more, except an object is called
a dictionary and an array is called a list.

The syntax is similar. However, keys in dictionaries don't require quotes, and
assignment is usually done using the equal sign rather than a colon. The colon
actually represents a *line-string*, a string that ends at a newline or a comma
(a separator). The behavior isn't exactly like this, but this is a good summary.

Comments are allowed. `//` is a line comment and `/*...*/` a multiline comment.

An "object" in JSON...
```json
{
	"firstName": "John",
	"lastName": "Doe"
}
```
is equivalent to a "dictionary" WebSSON:
```websson
{
	firstName: John
	lastName: Doe
}
```

An "array" in JSON...
```json
[
	true,
	123
]
```
is equivalent to a "list" in WebSSON:
```websson
[
	true
	123
]
```

The cool thing in WebSSON is using entities alongside templates and tuples.
(Entities could be considered as variables, but all constants. In XML, the
word entity is used, so I thought this was a better name.)

A tuple is a structure that could be considered like the mix of a dictionary and
a list. Its values can be accessed by name (if they have a key associated with
them) and index. It's mostly useful when used with a template.

A template basically sets up keys that must be given values by a tuple
associated with the template.

Instead of doing...
```websson
person
{
	firstName: John
	lastName: Doe
	age = 38
}
```
you could do:
```websson
person<firstName, lastName, age>
(
	:John
	:Doe
	38
)
```

Entities allow to avoid duplication. There are two types of entities: abstract
and concrete. Basically, a concrete entity stores a value that is intended to
be manipulated by a program using the data in the document. It could be said
that abstract entities provide support and that only the parser and serializer
should really have to deal with them &mdash; at least when consuming the data.

For the last example, an entity could have been declared as such:
```websson
!Person<firstName, lastName, age>
```
This is an **abstract** entity that represents a *template head*, the head part
of a template. It can be completed by a *template body*, which is a tuple. The
above example can become:
```websson
person = Person(:John, :Doe, 38)
```

Finally, I leave a more complex example.

This in JSON...
```json
{
	"person1": {
		"firstName": "John",
		"lastName": "Doe",
		"age": 38,
		"numbers": {
			"home": "890 456-7123",
			"work": "890 357-1246",
			"cell": null
		}
	},
	"person2": {
		"firstName": "Jane",
		"lastName": "Doe",
		"age": 38,
		"numbers": {
			"home": "733 435-0187",
			"work": "733 853-7211",
			"cell": "629 457-3173"
		}
	}
}
```
is equivalent to this in WebSSON:
```websson
!Numbers<home=null, work=null, cell=null>
!Person<firstName, lastName, age, <^Numbers>numbers>

person1 = Person(:John, :Doe, 38, ::(890 456-7123, 890 357-1246))
person2 = Person(:Jane, :Doe, 38, ::(733 435-0187, 733 853-7211, 629 457-3173))
```
