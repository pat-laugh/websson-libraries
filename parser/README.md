# WebSSON Parser

## Imports

To import documents, the parser currently uses a static
[curl](https://curl.haxx.se/) library.

To prevent that, you can disable its use. This will disable imports. To do this
you have to define `WEBSSON_PARSER_DISABLE_IMPORT` before compiling. You can
also just call `make disable_import`.

## Binary endianness

WebSSON is meant to be parsed and serialized in little endian. If you wish to
parse binary values in little endian from a big endian machine or big endian
from a little endian machine, you can define `WEBSSON_REVERSE_ENDIANNESS`
before compiling. You can also just call `make reverse_endianness`.