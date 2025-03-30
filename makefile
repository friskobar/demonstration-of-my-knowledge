debug:
	g++ src/*.cpp -o bin/debug/DOMK.exe -Iinclude -IC:/VulkanSDK/1.4.309.0/Include -Llib -l vulkan-1 -l glfw3 -lgdi32
	bin\\debug\\DOMK.exe

release:
	g++ src/*.cpp -o bin/release/DOMK.exe -Iinclude -IC:/VulkanSDK/1.4.309.0/Include
	bin\\release\\DOMK.exe