# Royal Tracer - Conceptional Description

## Our Mission
Our journey began with a clear goal: to go beyond the realm of basic prototypes. We aspired to create a tool that integrates seamlessly into everyday workflows, akin to the reliable and functional products already present in the market. Our mission was to bridge the gap between advanced technology and practical usability, ensuring that our tool is not only innovative but also user-friendly and applicable in various settings.

## Choosing the Programming Language: Prioritizing Speed and Efficiency
One of the cornerstones of a successful software product is its performance. Recognizing this, we selected C++ as our programming language of choice. Renowned for its efficiency and close-to-the-metal nature, C++ allows direct interaction with the underlying hardware. This characteristic enabled us to optimize our tool for speed and power, ensuring that it can handle intensive tasks without compromising on performance.

## Balancing GPU and CPU Rendering: Ensuring Broad Accessibility
At the heart of our project lies ray tracing, a technique that requires substantial computational resources. Typically, this is managed by GPUs, known for their ability to accelerate such processes. However, we acknowledge that not every user has access to top-tier GPUs. Thus, our design caters to both GPU and CPU capabilities. By utilizing OpenCL, we ensure dual compatibility, making our tool accessible and efficient for a diverse range of users with varying hardware configurations.

## Utilizing the GPU
At the heart of our project lies ray tracing, a technique that requires substantial computational resources. Since *graphical processing units* (GPUs) are known for their ability to accelerate such processes due to their unparalleled multithreading capabilities, leveraging this potential was an important to-do for our project from the start. On the way we faced several hurdles like GPU-vendor specific configuration necessities or CPU utilization, which was discarded at a later point. But, using OpenCL, a framework for device-independent software, we achieved utilizing CPU-integrated graphics chips as well as top-tier graphics cards in our software.

## Developing a Modular Architecture: Emphasizing Flexibility and Customization
Flexibility and adaptability were paramount in our architectural development process. We designed our system with modularity in mind, allowing for various components such as the editor, ray tracer, and storage management to be interchangeable. This modular design not only facilitates maintenance and upgrades but also provides users with the freedom to customize or enhance the tool with their unique modules or features.

## Transitioning to a Web-Based Editor: Enhancing Accessibility and Convenience
Our initial approach involved a C++ and QT toolkit-based editor. However, with a commitment to enhancing user accessibility, we transitioned to a web-based editor. This strategic shift enables users to access our editor from any location, eliminating the need for complex installations. Although this move presented challenges, particularly in terms of file access and integration, the resultant increase in convenience and accessibility was a significant step forward in aligning with our mission.

## Implementing Path Tracing: Elevating Realism in Rendering
To achieve visuals that closely mimic reality, we integrated path tracing into our ray tracer. This sophisticated technique significantly improves the realism of rendered images by accurately simulating complex lighting and shading scenarios. The incorporation of path tracing marks a significant milestone in our quest to deliver high-quality lifelike renderings.

## Conclusion: Realizing Our Vision with a Dynamic Final Product
Each strategic decision and development phase has culminated in a tool that embodies our initial vision. What we present now is a fully-functional, real-world application, poised for continuous improvement and adaptation. Our dedication to delivering a user-centric, adaptable tool remains the driving force behind our ongoing development and future enhancements.