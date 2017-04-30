# WebSSON Parser

## Imports

To import documents, the parser currently uses a static
[curl](https://curl.haxx.se/) library.

To prevent that, you can disable its use &mdash; which will also disable imports
&mdash; by uncommenting the `#define DISABLE_IMPORT` in the file
"importManager.cpp".
