Emscripten is a project to compile c/c++ code int the asm.js format which can be run in some browsers.
See http://kripken.github.io/emscripten-site for more information.

emscripten support for Irrlicht is a work in process. Use at your own risk. 
Might take work and knowledge to get it running.

------------
REQUIREMENTS
------------
You have to install the emscripten environment.

----------------------------
BUILDING Irrlicht & your App
----------------------------

Linux:
Go into source/Irrlicht folder and call:
emmake make emscripten


Go into examples/01.HelloWord_emscripten folder and call:
emmake make all_emscripten
