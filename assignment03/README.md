# CS 184 Fall 2016
Instructor: James O'Brien
Names: Evan Limanto (cs184-adf), Yihan Lin(cs184-acz)

# USAGE
‘cd’ into your assignment directory, type into the Terminal the following step by step

1. mkdir build
2. cd build
3. cmake ..
4. make

# Keyboard features
1. 'ESC': Exit
2. 'Q': Exit
3. 'F': Full screen
4. 'Shift + ↓': Translate objects down
5. 'Shift + ↑': Translate objects up
6. 'Shift + ←': Translate objects left
7. 'Shift + →': Translate objects right
8. 'A'/'S': Change object to transform using arrow keys.
9. →/←/↓/↑: Rotate object about axes.
10 +/-: Scale object on all axes.

# Extra features
- Multiple objects in one scene, as specified in .scene file.
  These support individual transformations as in the raytracer project.
  When the program loads, use the A and D keys to select the object to
  transform using arrow keys.
  Example contents (scn.scene)
	xft 1 1 1
	xfs 2 1 1
	xfr 1 0 1 2
	obj input/teddy.obj
	xfz
	xfr 1 1 1 2
	obj input/cow-nonormals.obj
	xfr 1 1 0 2
	bez input/teacup.bez

- Obj input/output (output is specified with -o flag)