# Setup Instructions
1. Download the latest release from the [GitHub Releases Page](https://github.com/superflo22/royal-tracer/releases).
2. Unzip the folder.
3. Double click on the `royal-tracer.bat` file to start the application.

# Editor Usage
## Menu Bar
- Import
    - Import a scene from a ´.zip´ file (including objects, config, materials and textures).
    - Import a scene from a `.yaml` config file.
    - Import a scene from a directory (including objects, config, materials and textures).
    - Import a single object (`.obj`) into the scene.
    
    For all the Import options the objects are added to the workspace, and the scene is built up according to the config (if present).
- Add Light
    - Point-Light
    - Directional-Light

    Adds the selected light to the scene.
- Render
    - Starts the raytracing process.
    - Running process is indicated with a spinner icon.
- Show Result
    - Button is only visible if there already is a raytraced result.
    - Shows the last raytraced image.
- Download Scene
    - Downloads the current scene as a `.zip` file (including objects, config, materials and textures).
## Overview
- Shows all the elements which are currently in the scene.
- Selecting an element opens the according inspector.
## Workspace
- Shows all the objects that are already loaded into the editor via one of the import options. These elements don't necessarily have to be already in the scene.
- Clicking on an object adds it to the scene.
## Inspector
- Shows the settings of the current selected element from the scene.
- The settings can be changed in the according fields and applied by pressing the "Apply Changes" button.
### Object Inspector
Materials are automatically assigned to objects when the according material is already present in the editor when the object is added to the scene. Textures and materials can be preloaded either by loading a directory or a zip. Nevertheless, materials can also be loaded and assigned manually.
- Materials can be added via the material button in the object inspector.
- For the materials to work, make sure to first load all necessary textures via the texture button.

### Animation System
The objects contained in the scene as well as the camera can be animated linearely in rotation and translation. To animate a part of the scene, the important steps are:

1. Select the number of keyframes
2. Select an object or camera to animate. The initial Keyframe is generated automatically
3. Add one or more additional keyframes containing updates to the rotation and translation. These will be interpolated linearly.

Important: the last keyframe has the index number-1.

<br>

---

# Setup Instructions (Development)
## Web Editor
1. Go into `editor/src/`
2. Run `npm install`
3. Run `npx webpack --config webpack.config.cjs`
4. Run `npx webpack serve --open`

## Backend
1. Have a working CLION IDE.
2. In your IDE open the `CMakelists.txt` file in the `/backend` directory
3. The IDE suggest to select and build the CMake project. Do so.
4. This will throw an error, because you need to link the openImageDenoiser from Intel.
5. Download the openImageDenoiser from [Intel](https://openimagedenoise.github.io/downloads.html).
6. Unzip the file.
7. Go back to `CMakelist.txt` and modify the following line to the path where you unpacked the openImageDenoiser: 
        
            set(OIDN_ROOT C:/oidn-2.1.0.x64.windows)

    - The path needs to point to the directory that contains the `bin` folder.
7. After rerunning the CMake build, another error will be thrown, because you need to install the vcpkg dependencies.
8. In the console a `fix` Button will appear. Click it and then install the dependencies.
9. After that, the CMake build should work and you can run the project.
