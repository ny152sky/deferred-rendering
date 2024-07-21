

Due to personal hardware limitations, I did my project setup to run on MacOS.
In my project folder, there is an include and a library folder that contains the required files for GLFW, glad and glm libraries.
These are present because they are required for my project setup on MacOS.
I compiled my project by running the following command inside the project directory:

g++ --std=c++20 main.cpp glad.c -Iinclude -Llibraries -lglfw3 -o main -framework Cocoa -framework IOKit

I also used stb_image.h to test with texture images, which is included in my project folder.
No other libraries should be required when running the program.

I use the following commands to unzip and run the project:

unzip 469project.zip -d 469project
cd 469project
g++ --std=c++20 main.cpp glad.c -Iinclude -Llibraries -lglfw3 -o main -framework Cocoa -framework IOKit
./main