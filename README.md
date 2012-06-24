koohar
======

Library to process http requests.

Building.

Linux:

To build library go to koohar/src and execute "make". It would bring you
shared and static libraries. (gcc 4.5(+/- 0.1) or higher is required)

Windows:

Such as MSVS 10 does not supprt "mutex" header you need to install
<a href="www.boost.org">boost</a>.

Koohar uses some c++11 stuff, so make sure you have Visual Studio 10 or higher.
Then create console win32 project, and add files from koohar/src to it. Do not
forget to add include paths for boost, and set in project setting to build a
static library(.lib). Dynamic library is not supported cause i dont buy all
that "WINAPI dllexport" and other things.

Test app.

To build test app first build a library. Then copy it to koohar/tests and run
"make". Now execute the 'koohar' binary and enjoy the application at 7000 port.
