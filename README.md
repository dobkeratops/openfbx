fbx LGPL importer c++ 
no use of autodesk SDK
reads subset of fbx 3d model file format(ascii).
Intended for a file translator or integration in other tools/engines 
only tested with a small number of fbx files from blender so far.
support at the moment:-
-coordinate frames
-animation curves
-mesh
-weighmaps
-animation
-UV's
verified in a debug-viewer(but no texture viewer)

includes a vertex reducer to find {xyz,uv..}  vertices for rendering

fbxviewer.cpp is the test program , loads a file passed in the command line & views it

