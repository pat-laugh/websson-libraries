# WebSSON Serializer

## Binary endianness

WebSSON is meant to be parsed and serialized in little endian. If you wish to
serialize binary values in big endian from a little endian machine or in little
endian from a big endian machine, you can define `WEBSSON_REVERSE_ENDIANNESS`
before compiling. You can also just call `make reverse_endianness`.