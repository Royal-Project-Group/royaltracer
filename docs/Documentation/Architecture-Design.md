# Software Architecture and Design

## Overall Architecture
### Underlying paradigms and patterns
The project was designed with two architectural paradigms in mind: the microservices architecture and the component-based design paradigm.
Former describes that the software can be separated into two processes that run independently: the editor and the backend. Latter means that the software can be divided into components that offer their functionality independent of the architecture they are embedded in, and that system-wide functionality is achieved by connecting these components with one another.
The backend can be divided into two more sections: the Middleware section and the Raytracing section. The architecture of the backend is based around a combination of the Mediator and the Facade pattern.
Facades are used to encapsulate components, that is to ensure that no calls are to be made to subcomponents from outside and that no subcomponents can make calls to outside the component. This means that components can be smoothly replaced with other components as long as their facades adhere to the same interface.
To further enhance the scalability of the architecture, a Mediator is placed right into the center of the design; it is responsible for mediating between the components. This means that the complexity of component-to-component communication will **not** increase with each new component added since components do not communicate directly but through the Mediator.

![Component diagram overall](/docs/Documentation/media/UML/Komponentendiagramm_gesamt2.svg)
*This component diagram visualizes the most important aspects of the architecture.*

### Workflow
![Sequence diagram scene render](/docs/Documentation/media/UML/Sequenzdiagramm_raytracing.svg)
*This sequence diagram illustrates how a scene render request by the client is gradually processed and handed over to the pathtracer.*

## Editor
### Basic Structure
The editor is designed using the well-established model-view-controller (MVC) pattern, which facilitates a clear separation of display logic from data, contributing to the development of cleaner and more robust code. In conjunction with MVC, the editor also leverages the observer pattern. This pattern enables a view to subscribe to the model, ensuring that it stays informed about any changes within the underlying data.
#### Model
At the core of the architecture is the model which encapsulates the entire scene. This includes objects, lights, materials, and camera data. By centralizing scene-related information and operations, the model serves as the backbone for maintaining data in the frontend. To adhere to the standard YAML format for the raytracer, the model uses the YAMLParser to convert YAML files into a model or vice versa. 
#### Controller
The controller acts as the intermediary between the views and the model. Its primary responsibility is to map user interactions to corresponding actions in the model. This seamless interaction ensures that changes initiated by the user through the views are appropriately reflected in the underlying data model.
#### Views
- **View:** This view is directly responsible for managing the displayed HTML. It handles events and initializes the layout, providing a user-friendly interface for interacting with the editor.

- **RequestView:** This specialized view facilitates communication with the backend. Informed about any changes to the model, it promptly sends a http request containing the relevant information (e.g. an OBJ file) to the backend, ensuring synchronization between the frontend and backend components of the system.

![Class diagram of Editor](/docs/Documentation/media/UML/editor_class_diagramm.svg)
*Class diagram of the complete editor architecture.*

### View Structure
As can be seen in the class diagram above, the view is separated into four different parts to reduce complexity. The ThreeJS canvas used for the preview is managed by the view itself, while the top (menu buttons), the inspector (used to modify the scene), the popup (used to show the rendered image) and the tabs (workspace and overview) are outsourced into own classes. While the top and the popup only implement limited functionality, the structures of the inspector and the tabs are more complex and will be explained further in the next sections.
#### Inspector
For realizing the inspector logic, the composite pattern is used. As there are three different scene element types (camera, objects and lights), three different inspector types are necessary. As there are values, such as the position, that are relevant for all scene elements, all of the inspectors are extensions of the base inspector class which implements all the inspector parts that all scene elements have in common. As actually always only one inspector is displayed (the one for the current selected object), an inspector composite is used to create another layer of abstraction. Said inspector composite contains all the mentioned inspector types and delegates the commands from the view to the fitting inspector.
#### Tabs
In the current state of development, the tabs area contains the workspace and the overview. The concept is based on a tab controller which manages a list of tabs. These tabs can be added and removed dynamically, so the user interface can be extended with additional functionality at any time. Actual implementations of a tab extend the tab class and add the actual functionality which is necessary for example for the workspace.


### Workflow
The sequence diagram illustrates a typical MVC workflow, demonstrating the addition of a point light as an example. This visual representation showcases the orchestrated interactions between the model, views, and controller, highlighting the seamless flow of information and actions within the editor.

![Sequence diagram of Editor MVC pattern](/docs/Documentation/media/UML/Editor%20MVC.svg)

*Sequence diagram of Editor MVC pattern*

This architectural design promotes modularity, maintainability, and extensibility, key factors for the success of this project.

## WebEditorFacade
The WebEditorFacade is the interface between the backend and the editor. Specifically, it is a web server which provides the api endpoints for the communication between the editor and the backend. In the end, it contains no "real logic" but redirects the requests from the editor to the mediator and returns the according responses.
#### WebServer
The webserver is implemented in C++ using the [Crow framework](https://crowcpp.org/master/).
#### API
- **PUT `/api/obj/<id>`:**
  - Stores the given object from the request body in the backend.
  - This endpoint also exists for *materials* (`/api/mtls/<id>`) and *textures/maps* (`/api/txrs/<id>`)
- **DELETE `/api/obj/<id>`:**
  - Deletes the object with the given ID from the backend.
  - This endpoint also exists for *materials* (`/api/mtls/<id>`) and *textures/maps* (`/api/txrs/<id>`)
- **POST `/api/export`:**
  - Initiates the raytracing and returns the resulting image in the response.

## Middleware
Between the two main components of the project, the editor and the pathtracer, a key interaction layer has been added to enable modularity and scalability: The so-called "middleware". As its name suggests, it acts as a mediating layer for connecting the two subsystems. It defines communication schemes (ie the Mediator in the center) for component-to-component interactions but also offers supporting functionality (for example, the "*StorageSystem*" classes).

### Diagrams
<img alt="Class Diagram Middleware" src="/docs/Documentation/media/UML/Middleware_final.drawio.svg" width=800 />

*Class Diagram shows currently existing classes that belong to middleware.*

![Sequence Diagram Object storage](/docs/Documentation/media/UML/Sequenzdiagramm_storage.png)
*Sequence Diagram for storing an imported object. The middleware ensures that the pathtracing component can reference this object seamlessly.*

### Usage of Design Patterns
Just like the frontend, the middleware utilizes different design patterns. An exemplary one will be introduced in the following.
#### Factory Method: Interface Parser
The Parser class acts as an interface that all classes offering parsing functionality can implement.
```cpp
template<class Resource>
class Parser {
public:
    virtual std::shared_ptr<Resource> parse(const std::string& content) {
        std::shared_ptr<Resource> res = std::make_shared<Resource>();
        std::vector<std::string> lines = lexpp::lex(content, "\n", false);

        for(const std::string& line : lines) {
            determineElement(line, res);
        }

        return res;
    };

    virtual void determineElement(std::string line, std::shared_ptr<Resource> res)=0;
};
```
Subclasses of parser override `determineElement(line, res)` to implement their custom parsing behavior. They can specify the resulting object when inheriting from Parser, which is a template class.
```cpp
class MTLParser: Parser<std::vector<Material>> { ... }
```

# Raytracer

![Raytracer Architecture overview](/docs/Documentation/media/UML/raytracerArchitecture.webp)

*Architecture overview of the raytracer.*

## General information and architecture
### The Precompute Layer
The precompute layer is responsible for translating object meshes based on their positions, rotations, and scales. This primary computation is executed on the GPU due to its versatility and speed.

In the precompute layer, each mesh collects all its triangles into an input buffer for the GPU. Additionally, this input buffer contains the transformation matrix, calculated using MathUtils.cpp. It's worth noting that the rotation follows the same order as Three.js applies it in the web editor.

Using PrecomputeKernelCommunicator.cpp, data is transferred to the GPU and processed through the OpenCL script transformation.cl.

Within the precompute kernel, transformations are applied in parallel, normal vectors for translated triangles are computed, and essential data such as UV coordinates is retained.

### OpenCL as homogenous programming framework
Raytracing and especially path tracing is known to be a very computational expensive operation. To achieve good visual fidelity, it is necessary to perform several iterations and average the result.
While this can be very challenging when performed in a single thread, it can be sped-up quite easily through massive parallelization. The GPU as an accelerator to handle expensive tasks is a great tool to increase performance dramatically. To put things into perspective, modern GPUs achieve incredible floating point precision performance (flops, “floating point operations per second”), way above any contemporary CPU. For example, the NVIDIA RTX 4060 mobile, a newer mid-tier GPU, can perform around 10 tflops, which is around 40 times faster than an equivalent mobile CPU.
Another point is the memory specification of GPUs compared to CPUs. While a CPU has bigger caches and better cache accuracy, the GPU has faster and more modern RAM and very aggressive cache management, lowering access times. Due to the focus on parallelization on the GPU, this can be further optimized by accessing the memory in “warps”, which caches access requests from different workers (“cores”) and accesses the memory on a connected range which massively improves memory access speed.
When talking about ray and path tracing, the optimal performance can be achieved by using NVIDIA GPUs and Direct3d or Vulkan as the graphics API. These are the only available frameworks that support the usage of NVIDIA RT cores, which are specialized processors that can perform ray triangle intersection, necessary for ray tracing, faster than the normal shader units. This comes at the disadvantage of the lack of support for most present hardware, and only really matters for real time path tracing.
As this project should be usable for almost everyone, we decided to use OpenCL. This is a heterogeneous programming framework that supports nearly any kind of hardware. This enables the ray tracer to work with nearly any dedicated GPU, as well as any chipset GPU and even the CPU after the installation of a small driver. The biggest downside of this is the loss of performance as OpenCL introduces some overhead. The overall performance is acceptable, as we do not intend to support real-time path racing for now.

### Basic architectural considerations
The ray tracer module (part of the middleware architecture) based on C++ and OpenCL builds upon the pipeline architecture. The several processing steps required to generate the path traced image are split up into smaller modules that hand their generated output to the next module in line.
This architectural design choice works very well in the context of computer graphics in general. In this case, the workload is split into three main parts: 
-	Precompute Layer: This first module in the pipeline handles the precomputation of all data provided by the mediator. Currently, its main purpose is to apply transformation matrices to objects (list of triangle primitives) and to calculate their respective normal vectors. Also, texture and map data are prepared here.
-	Raytracing Layer: This is the core of the ray tracer. The modified data is used in an advanced raytracing algorithm to create the desired output image.
-	Output Layer: This layer cleans up the generated image and passes it back.

To improve performance, GPU acceleration is heavily used. Most of the expensive operations are moved to the GPU using an OpenCL kernel. To improve readability and debugging, the new kernel v3.0 for example uses an expanded system of structs to store data and is partitioned into several header files to break up the close to 1500 lines of code needed for the kernel v2.
Furthermore, the GPU is utilized by the precompute layer as well. In this case, it is used for efficient matrix multiplication and parallelization of the normal vector calculation (more on this in the section regarding the precomputation operations).

# OpenCL communication
## Host-kernel communication
As the GPU is somewhat decoupled physically from the CPU, OpenCL uses a “host” – “kernel” separation. The kernel code is contained in a text file with ending .cl that is supposed to be executed in parallel by the “kernel”- device. As it uses a modified version of c99, object orientation is not available. However, certain data structures can be used to ease managing larger amounts of data.
 The “kernel” – device can be any supported device the executing machine offers. GPUs, CPUs, and even self-designed FPGAs all are viable to run OpenCL-code. This gives it the edge over any other framework concerning device support. Usually, OpenCL is integrated into the device’s drivers.
The “host”-device serves as a manager for the “kernel”-device communication. This includes compiling the code that will be executed on the “kernel”-device, as well as preparing the memory in both devices. The “host” is no device in the general sense. It is rather the C++ code that forms the backend running on the CPU, mostly using only one main thread.
Setting up the OpenCL- “kernel” execution requires some basic steps:

•	Fetch the best device from all possible devices. This is evaluated based on the device's computational power in tflops.

•	Create the context and the command queue for the best device.

•	Build and compile the kernel code and feed it into the program object.

# Data transfer and struct system
To finish setting up the kernel device, the important data must be transferred to the device. To understand this, it is best to take a brief look at how OpenCL defines memory types. It manages the RAM on the kernel device as “global” memory. This enables all worker units (“cores”) in the kernel device to access it. GPUs usually have larger clusters of worker groups. These can access shared memory labeled as “local”. Currently, the ray tracer does not use local memory as it is mostly used to share temporary data between workers. 
Everything else is classified as “private” memory. This essentially means registers in the GPU context. As the GPUs memory management can only be described as “aggressive”. This leads to faster access but increases the error rate. Ultimately, it is even possible for data to be unloaded while it is still used. 
With this in mind, we can transfer the important data to the kernel device. To make the transfer more convenient to use, a template function is provided to handle the data transfer. The rather cryptic function takes in a variable amount of array type data parameters, output array data parameters and simple data type parameters to pass to the kernel and start it.

## Kernel structure and data structures
To improve memory performance and code readability, the kernel code uses structs to pack the data. Here we use the knowledge about memory types in the GPU to optimize memory access. Data “structs” are specially designed to minimize the memory size to increase the cache probability.


# Ray Intersection
## Triangle Intersection
The core technique used in Ray-/Path tracing is how each ray interacts with the modeled environment. First, the hit position and data describing the surface hit needs to be fetched. This is done by intersecting a line with a polygon. One of the most efficient approaches is the Möller–Trumbore intersection algorithm, which basically simplifies and optimizes a plane/ray intersection. 
The rays are generated from a single point, the “camera position”. A pixel grid is created in the view direction of the camera using its right and up vectors. The initial ray direction is then derived from the position of the camera and the center position of a given pixel.
This can be used to construct a line “shooting” out of the camera. This line describes the first ray, which is used to intersect with the environment consisting of triangles. Using an unoptimized “brute-force” approach, this leads to calculating the intersection of the ray with every triangle present in the scene – leading to a scaling of O(n) where n is the count of triangles. As there might be millions of triangles in the scene, this gets very expensive very fast.
## Acceleration structure: BVH as binary tree
Luckily, there are several approaches to tackling the scaling of the problem. One of the earlier approaches is the use of octrees. These split the 3D-space into voxels, containing the triangles at the leaf nodes. While this is quite good theoretically, it has significant drawbacks in a real application. The octree does not consider the triangle density. If a model has a very heterogeneous triangle density, an octree optimization will yield diminishing returns.
A similar approach, the KD-tree solves this problem by creating the voxels dynamically. The voxels size is adjusted dynamically to the size of the model/triangles. While this is one of the best accelerators for static scenes, it has a significant problem: once the scene changes, the complete structure has to be regenerated. This makes it unusable for real-time applications or an animation system.
The current basic state-of-the-art acceleration structure used is the Bounding-Volume-Hierarchy (BVH). Its idea is to not divide the space but the models. This means that a model and subsequent sub models up to the triangle layer are ordered in “bounding boxes”. The ray intersection can be performed against these bounding boxes. The tree structure ensures good runtime, scaling with O(log(n)). This means that the ray tracing itself is not computationally dependent, but memory dependent. It is possible to have millions of triangles in a scene. When using instancing, even billions are possible. The BVH has two advantages over KD-trees: instancing is supported, and animations are possible as the while Hierarchy does not need to be recalculated when sub models change their transform properties by using a 2-split architecture.
## BVH performance considerations
The creation of the BVH structure takes a significant amount of time. The unoptimized process has a runtime of O(n^2), making it unpractical for larger models. The creation can be optimized using binning, leading to a runtime of O(n). On a relatively fast CPU, the BVH creation without a split BVH architecture takes about 8 seconds for 1 000 000 triangles. The process can be multithreaded for each separate object in the scene. Furthermore, by adjusting the size of used data structures, these can be adapted to the CPUs cache size, reducing cache misses. This is used in the GPU as well, but as the cache size is much smaller, its advantages are only minor.
## BVH traversal on the GPU
To evaluate a ray/triangle intersection, the BVH can be traversed on the GPU. As its leaf nodes contain one or two triangles, only a low amount of intersection tests is performed. The BVH has another advantage: traversing the tree by intersecting the given ray with the Axis Aligned Bounding Boxes used as bounding volumes, traversal steps are significantly cheaper to compute then the triangle intersection.


# Tracing algorithm explanation
## Diffuse reflection
The strength of path tracing lies in the realistic calculation of indirect diffuse lighting. If light hits a non-reflective surface, it bounces randomly. Following a light path of several random bounces, the indirect lighting can be approximated.
Using the monte Carlo integration, a path tracer can approximate the indirect illumination. However, the outcome is highly noisy – only a very high number of samples can offset this. To reduce the effect, importance sampling is used. Instead of reflecting the ray in a random direction in a uniformly distributed hemisphere, we use a use a cosine weighted hemisphere and adjust the result using a probability density function (PDF). Doing so will result in a less noisy picture. This technique will be used in several other sampling methods as well.

## Specular reflection
Like the diffuse reflection, light bounces of a specular surface hit. The direction of the reflected light ray is determined by its incident angle. This is easy to model, as no randomness needs to be applied to the ray sampling. In our implementation, the specular reflection is further enhanced by applying a small noise relative to the Ns-value of the material hit (corresponds to the “roughness” in our implementation).

## Refraction
Refraction works like reflection. In reality, Fresnel’s law determines to relation between reflection and refraction. If the incident angle is big (close to 90°), the probability of a ray reflecting is much higher. This is implemented in our ray tracer as well. If the ray is refracted, the strength of the refraction is determined by the refractive indices of the media. Our ray tracer models Fresnel’s law accurately by processing the surface evaluation (BRDF) hierarchically. 


## Light importance sampling and virtual light size
As we majorly use “simple lights” like point lights and directional lights for the stronger illumination, problems arise when reflection occurs. To offset this, the lights receive a virtual radius that is used to create artificial reflections. This also comes in handy in refraction, as otherwise, refraction effects wouldn’t be visible.
Furthermore, the lights are heavily importance samples. The light ray is targeted directly at the light source. To make sampling of lights independent of the number of lights in the scene, in each bounce, only one light is sampled. The probability of a light being sampled is determined by its distance to the sampling point – the intensity of the light is weighted accordingly.

# General performance considerations
## Sub-pixel-sampling anti-aliasing
In path tracing, several rays are usually shot per pixel. This can be used to anti-alias the image by randomizing the ray direction inside the pixels area.
Doing so will decrease the quality of low sample images but increase the quality of high sample images.

## Russian roulette sampling
Tracing rays with a high number of bounces can be very expensive, especially in fully enclosed objects. To improve performance, Russian roulette sampling can be used. The basic idea is to “kill” rays that contribute little to the output image early. In our implementation, this is evaluated by the light contribution of a ray. Rays with lower accumulated emission will be sorted out.

## Post Processing
Super sampling anti-aliasing
This simple but effective technique allows to render a picture at a higher resolution than intended and then scaling it down resulting in a much smoother image. While this creates good results for every sampling amount, it is computationally very expensive.

## Intel Open Image Denoiser
The Monte Carlo estimation in a path tracer creates significant noise. Only very high sample counts can offset that noise, requiring very long render times. As this is unhandy, denoising is used to remove the noisy pixels from the image. State-of-the-art approaches use AI, several filters and additional image information to remove the present noise from the image.
We use the Intel Open Image Denoiser as it runs on any hardware available in most computers. It is optimized for intel CPUs but runs sufficiently well on other hardware.
The best alternative would be the NVidia OPTIX denoiser as it also supports continuous image data flow. This makes it possible to link several frames from for example an animation or real time render reducing noise significantly.

# Future: Spectral rendering
After experimenting with spectral rendering in an older version of the kernel, it became apparent that a complete change of the color system would be necessary to support full spectral rendering. Also, the noise introduced with spectral rendering is even worse then the noise created by the monte carlo method, resulting in high color noise in the picture. The denoiser is not able to reduce this type of noise.
To solve this problem, we plan to add hero wavelength sampling, a technique to significantly reduce color noise in the image. Its core idea is to use a single ray to sample multiple wavelengths. Doing so will result in a much clearer image.

# Future: Aggressive Ray Reusage using Hash maps
The following procedure is completely self-developed. We are not aware of any similar technique.
This is a newly proposed procedure to use VRAM to reuse rays. As our path racer is very light on the RAM usage, a lot of its capacity is wasted. This unused RAM can be used to accelerate the path tracing even further. We propose a special hashing structure that is based on the triangle ID already present.
## HashMap structure
The core problem of saving and accessing the data for every single hit point is to do it efficiently without any kind of sorting. Luckily, the triangle IDs can be used to create hash intervals that hold the data for the ray reusage. This way, we can also scale the hash map based on the triangle size: the bigger the triangle surface area, the more different “hit areas” we must handle based on a given constant factor that determines the resolution of the hashed points.
On the one hand, this eliminates the need for saving the exact hit point. This reduces the memory space necessary to save the data structure. On the other hand, it enables lightning-fast access on the data with no iteration over any data which results in a very fast access time decoupled from the triangle or ray count (O (1) complexity). After we have finished filling in data into local hash maps, we can then compress the data stored inside. As some entries might be empty, we can ignore them. 

## Merging the data
There are three computation stages present to reuse the rays. The procedure in general can be understood as an extended type of post-processing, as the load on the main raytracing algorithm during tracing is minimal.
First stage: Saving the ray data.
-	Initialize a special data container containing hit position and masked emission to save the ray intersection date at each intersection. This must persist throughout the whole tracing process. 
-	At each ray intersection (bounce), temporarily save the current mask value, emission gathered, hit position and ID of the triangle hit. This will be used to “reverse engineer” the path later.
-	Before exiting the kernel unit, regenerate the following path properties for each point hit on the path. This way, we now have as many sub rays as we have bounces. These can be reused.

Second stage: hashing.
-	For every entry in the hit point information data container, we now access the corresponding triangle ID and use it to calculate the hash address based on the local position coordinates. Every triangle ID corresponds to a precomputed memory section based on its surface area. More on this later.
-	The hash map stores accumulated data. This implies that we use the local hit points partial ray estimation to modulate this value. The procedure is simple: weigh the quality of the partial ray based on its remaining bounce count. Add this value to the already stored evaluated value. Then add the calculated weight to a weigh counter. The weighting is optional. Importantly, this is not thread safe. Some kind of atomic operator must be implemented.

Third stage: post computing.
-	Finally, we need to apply the collected data to every ray intersection. For every hit point, we use the hash function to fetch the accumulated ray information in a constant radius around the hit point. As the accumulated information will already contain very precise information, a radius of 1 should be sufficient, equally important when looking at performance.
-	Calculate the complete contribution as sum of the accumulation contribution of every point.
 





## Textures and Maps
![Ablaufdiagramm maps and textures](/docs/Documentation/media/UML/mapsAblauf.svg)

The textures and the other maps are applied in the KernelMapTools.h file. There is a method for each operation shown in the diagram, with an additional debug method.

Calculate barycentric coordinates:
- In this method, each hitpoint is assigned a pTex vector2 that contains the relative texture coordinate. This is important for applying the textures.

Apply normals from normal map:
- This method returns immediately if no normal map is assigned to the input triangle.
- If a normal map is available, the corresponding value is read from the map at the texture point in pTex.
- The difference of the normal from the map to the standardized vector3 (0, 0, 1) is applied to the normal of the triangle at the HitPoint.

Apply bump map:
- This method returns immediately if no bump map is assigned to the input triangle.
- If a bump map is present, the method calculates the derivative between the current value (based on the barycentric coordinate in pTex), the value to the right and the value above of it.
- The derivatives of the height field is used to change the normal of the input point.

Apply texture:
- This method returns immediately if no texture is assigned to the input triangle.
- Based on the barycentric coordinate in pTex, the diffuse vector3 is read from the texture and returned.

