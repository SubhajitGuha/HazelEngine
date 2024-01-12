# HazelEngine
A GAME ENGINE

HazelEngine is a 3D and 2D Rendering engine that is capable of making 2D and 3D games and desktop applications.

Features:  

*deferred rendering  
*physically based rendering (cook-torrance BRDF)  
*image based lighting  
*material system (ability to create, edit and swap materials in real time)(WIP)  
*resource management(WIP)  
*entity component system  
*shadows with cascaded shadow maps  
*ambient occlusion  
*bloom effect  
*terrain rendering using height maps  
*dynamic level of detail based on player distance from terrain  
*terrain with multiple textures is WIP  
*foliage rendering (has its own class for fine control)  
*frustum culling  
*level of detail system  
*physics system using PhysX (WIP)  
*interactive ui for object placement (WIP)  
*procedural content generation using compute shaders(WIP)  
*asset loading via resource manager (can be done through code no ui is present)  
*uuid generation for all assets that engine can load  
*linking assets with their uuids  
*Serialization and deSerialization of scenes and assets using yaml(WIP)  
*content browser pannel, scene hierarchy pannel, details pannel, settings pannel ui implementation (WIP)  

There is also a path-tracer which runs individually for now has features like:  
*BVH generation for faster ray triangle intersection (WIP)  
*physically based rendering using cook-torrance BRDF, support for metallic and non metallic objects(WIP)  

Image of path-tracer  
![Screenshot (3525)](https://github.com/SubhajitGuha/HazelEngine/assets/102531274/a67d0d68-ed7b-4328-9d95-9f897570efa8)  

follow path-tracer showcase videos here:-  
https://www.youtube.com/playlist?list=PLTX2AQtZVYWatOlJ9zijldxM2c88LzLm7  
game engine showcases:-  
https://www.youtube.com/playlist?list=PLTX2AQtZVYWalcBrB7zJMjX-YOrMe6j1j  

Build Instruction:-  
First run "GenerateProjectFiles.bat" then the appropriate project files will be created (vs2019 files)
then launch the .sln file and then build and run the application using vs2019.

Some Screen shots of PBR rendering in my engine.
TERRAIN with FOLIAGE
![Screenshot (2953)](https://github.com/SubhajitGuha/HazelEngine/assets/102531274/29d2d029-1cbd-4536-918f-2d60ae450c82)

![Screenshot (2239)](https://user-images.githubusercontent.com/102531274/230664441-498f418d-9bb1-472f-98d7-2b574f9b454b.png)
With Ambiant occlusion(SSAO) Applied:
![Screenshot (2317)](https://user-images.githubusercontent.com/102531274/232682315-b756998f-3cf1-46d1-9556-368a71567b83.png)

All other pics are Without Ambiant Occlusion Applied!

Normal Mapping:
![Screenshot (2256)](https://user-images.githubusercontent.com/102531274/230779526-20c0415b-10c0-4a10-81a8-258b6b5a2432.png)
![Screenshot (2257)](https://user-images.githubusercontent.com/102531274/230782421-6a3082ca-26b3-40f2-9ebe-52009025430a.png)

![Screenshot (2230)](https://user-images.githubusercontent.com/102531274/230459807-70a9a2bc-dc8f-4222-a690-8d2ab8946ab0.png)

Screenshot of 2D rendering project (Trading Application)
![Screenshot (1935)](https://user-images.githubusercontent.com/102531274/230634976-bc39813d-5806-45e6-8643-19c42bc9f730.png)

Trading application needs to be build before running (how to build : first generate project files using the .bat file, then build the sandbox project and from the bin folder of root dir run the application).

But if you want the executable then go to the following repo:
Trading Application executable:

https://github.com/SubhajitGuha/Trading_Application

Contained in Application/Sandbox.zip

