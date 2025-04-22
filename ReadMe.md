This project contains a program that calculates the distances along a straight path over two height maps.
It prints the coordinates, the path distances, and the difference between the two distances.

To configure and build:

	mkdir release
	cd release
	cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-G Ninja \
		-DCMAKE_VERBOSE_MAKEFILE=ON
	cmake --build .

If your two height map files are in a folder called `path_data` at the same level as this projects `path_distance` folder, then the command to run it would be:

	cd release
	./src/code_test 512 512 4 5 500 501 ../../path_data/pre.data ../../path_data/post.data

A debug build is simply:

	mkdir debug
	cd debug
	cmake .. \
		-DCMAKE_BUILD_TYPE=Debug \
		-G Ninja \
		-DCMAKE_VERBOSE_MAKEFILE=ON
	cmake --build .

And to run it:

	cd debug
	./src/code_test 512 512 4 5 500 501 ../../path_data/pre.data ../../path_data/post.data

If you have node installed and prefer to use the scripts in the package.json file, do this:

	yarn
	yarn run watch-debug
