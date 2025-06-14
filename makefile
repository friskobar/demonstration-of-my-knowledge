debug:
	g++ src/*.cpp -g -o bin/debug/DOMK.exe -Iinclude -IC:/VulkanSDK/1.4.309.0/Include -Llib -l vulkan-1 -l glfw3 -lgdi32 -D DEBUG
	bin\\debug\\DOMK.exe

release:
	g++ src/*.cpp -o bin/release/DOMK.exe -Iinclude -IC:/VulkanSDK/1.4.309.0/Include -D NDEBUG
	bin\\release\\DOMK.exe

shader_compile:
	glslc shaders/default.frag -o shaders/compiled/default_frag.spv
	glslc shaders/default.vert -o shaders/compiled/default_vert.spv