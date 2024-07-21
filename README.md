# Deferred Rendering in OpenGL

This is my term project for the Computer Graphics II course I completed at METU. You can read more about it on my blog:

https://skystuhr.blogspot.com/2024/06/deferred-rendering-in-opengl.html

The project setup is done to run on MacOS.
In my project folder, there are include and library folders that contain the required files for GLFW, glad and glm libraries.
I compiled my project by running the following command inside the project directory:

g++ --std=c++20 main.cpp glad.c -Iinclude -Llibraries -lglfw3 -o main -framework Cocoa -framework IOKit

I also used stb_image.h to test with texture images, which is included in my project folder.
No other libraries should be required when running the program.
