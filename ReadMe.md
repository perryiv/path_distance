This project contains a program that calculates the distances along a straight path over two height maps.
It prints the coordinates, the path distances, and the difference between the two distances.

It has been built and tested on `Linux Mint 22 Cinnamon`, but it is written with platform-independence in mind so should work in other operating system, too.

To configure and build:

	mkdir release
	cd release
	cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-G Ninja \
		-DCMAKE_VERBOSE_MAKEFILE=ON
	cmake --build .

If your two height map files are in a folder called `path_data` at the same level as this projects top-level `path_distance` folder, then the command to run it would be:

	cd release
	./src/code_test 512 512 4 5 500 501 ../../path_data/pre.data ../../path_data/post.data

The output should look something like this:

	Processing input file: ../../path_data/pre.data
	Path distance from: [4,5] to [500,501] = 21868.4 m
	Processing input file: ../../path_data/post.data
	Path distance from: [4,5] to [500,501] = 21846.2 m
	Change in distance: 22.2686 m

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
