#
# CoPilot generated the first draft of the top-level CMake file,
# and this was in it. I moved it into this file.
#

# Add the executable
add_executable ( ${PROJECT_NAME}
	main.cpp
	Terrain.cpp
)

# Add the dependencies.
target_link_libraries (
	${PROJECT_NAME}
	PRIVATE
		Eigen3::Eigen
)
