# WebSSON Parser

## Imports

To import documents, the parser currently uses a static
[curl](https://curl.haxx.se/) library.

To prevent that, you can disable its use. This will disable imports. To do this
you have to define `WEBSSON_PARSER_DISABLE_IMPORT` before compiling. You can
also just call `make disable_import`.
