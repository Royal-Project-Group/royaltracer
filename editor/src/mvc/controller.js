import * as THREE from 'three';
import {OBJLoader} from 'three/addons/loaders/OBJLoader.js';
import {MTLLoader} from "three/addons/loaders/MTLLoader.js";
import JSZip from 'jszip';
import UTIF from 'utif';


class Controller {

    #model;
    #view;
    #loader;
    #materialLoader;
    #textureLoader;
    #url;

    #workspace;
    #animator;

    #frame = 0;

    #imageMap = new Map();

    constructor(model){
        this.#model = model;
        this.#loader = new OBJLoader();
        this.#materialLoader = new MTLLoader();
        this.#textureLoader = new THREE.TextureLoader();
        fetch('../config.json')
            .then(response => response.json())
            .then(data => {
                this.#url = data.url;
            })
    }

    // Load a scene from a .zip file
    readScene(file, callback){
        console.log("Controller: got scene file with name:" + file.name);

        if(file.name.endsWith('.zip')){
            this.readZip(file, callback);
        }else{
            const reader = new FileReader();
            reader.onload = (event) => {
                const contents = event.target.result;
                this.#model.fromYaml(contents);
                this.#view.showInspector("camera")

                // Call the callback function here
                if (callback && typeof callback === 'function') {
                callback();
                }
            };
            reader.readAsText(file);
        }
    }

    async readZip(data, callback) {
        try {
            const imageExtensions = ['jpg', 'jpeg', 'png', 'gif', 'bmp', 'webp', 'svg', 'tif', 'tiff'];
            const mime_types = {
                'jpg': 'image/jpeg',
                'jpeg': 'image/jpeg',
                'png': 'image/png',
                'gif': 'image/gif',
                'bmp': 'image/bmp',
                'webp': 'image/webp',
                'svg': 'image/svg+xml',
                'tif': 'image/tiff',
                'tiff': 'image/tiff',
            }
        
            const zip = await JSZip.loadAsync(data);
            let configFile;
            let objectFiles = [];
            let materialFiles = [];
            let textureFiles = [];

            // Loop through each file in the zip
            zip.forEach((relativePath, file) => {
                if (!file.dir) {
                    file.name = file.name.replace(/^.*[\\/]/, '')
                }
                console.log("File:", relativePath);
                if(relativePath.endsWith(".yaml") || relativePath.endsWith(".yml")){
                    configFile = file;
                }else if (relativePath.endsWith(".obj")) {
                    objectFiles.push(file);
                }else if (relativePath.endsWith(".mtl")) {
                    materialFiles.push(file);
                }else if (imageExtensions.includes(relativePath.split('.').pop().toLowerCase())) {
                    textureFiles.push(file);
                }
            });

            for(let txr in textureFiles){
                let contents = await textureFiles[txr].async("base64");
                const extension = textureFiles[txr].name.split('.').pop().toLowerCase();
                const dataURLprefix = "data:"+ mime_types[extension] + ";base64,";
                contents = dataURLprefix + contents;
                if( extension == "tif" || extension == "tiff"){
                    this.convertTiffToPng(contents)
                    .then(pngDataUrl => {
                        console.log('PNG Data URL:', pngDataUrl);
                        this.parseTexture(pngDataUrl, textureFiles[txr].name.replace(/\.[^/.]+$/, "") + ".png");
                    })
                    .catch(error => {
                        console.error(error);
                    });
                }else{
                    this.parseTexture(contents, textureFiles[txr].name);
                }
            }

            for(let mtl in materialFiles){
                let mtlContent = await materialFiles[mtl].async("string");
                mtlContent = this.cutTexturePaths(mtlContent);
                // Load the material from the content of the file
                const material = this.#materialLoader.parse(mtlContent);
                // Preload the material
                material.preload();
                // Save the material to the model
                this.#model.addMaterial(materialFiles[mtl].name, material, mtlContent);
            }

            for(let obj in objectFiles){
                this.#workspace.loadObjectFromString(await objectFiles[obj].async("string"), objectFiles[obj].name);
            }

            this.#model.fromYaml(await configFile.async("string"));
            this.#view.showInspector("camera");

        } catch (err) {
            console.error('Error reading ZIP file:', err);
        } finally{
            // Call the callback function here
            if (callback && typeof callback === 'function') {
                callback();
            }
        }
    }

    // Read a .obj file and parse it
    readObj(file, callback) {
        console.log("Controller: file with name:" + file.name);

        const reader = new FileReader();
        reader.onload = (event) => {
            const contents = event.target.result;
            const objName = this.parseObj(contents, file.name);

            // Call the callback function here
            if (callback && typeof callback === 'function') {
                callback(objName);
            }
        };
        reader.readAsText(file);
    }

    parseObj(contents, name){
        console.log("now parsing contents");
        const object = this.#loader.parse(contents);
        console.log(object);

        // should be view code
        return this.#model.addObject(name, object, contents);
    }

    // Load the material and instantly parse it
    async loadMaterial(file){
        console.log("Controller: got material file with name:" + file.name);
        const reader = new FileReader();

        const promise = new Promise(resolve => {
            reader.onload = (event) => {
                let mtlContent = event.target.result;
                mtlContent = this.cutTexturePaths(mtlContent);

                // Load the material from the content of the file
                const mtl = this.#materialLoader.parse(mtlContent);
                // Preload the material
                mtl.preload();

                // Save the material to the model
                this.#model.addMaterial(file.name,mtl,mtlContent);
                resolve();
            };
        });
        reader.readAsText(file);

        return promise;
    }


    cutTexturePaths(mtlContent) {
        // Regular expression to find texture file paths (assumes paths do not contain spaces)
        // Supports map_Kd, map_Ks, map_Ka, map_Bump, bump, and map_d
        const texturePathRegex = /(map_[Kd|Ks|Ka|Bump|bump|d]+\s+|bump\s+)([\w\/\\]+\/)?([\w\.]+)/g;
        // Replace the full path with just the filename
        return mtlContent.replace(texturePathRegex, (match, prefix, path, filename) => {
            if (filename.toLowerCase().endsWith('.tiff') || filename.toLowerCase().endsWith('.tif')) {
                // Replace the .tiff or .tif extension with .png
                filename =  filename.replace(/\.(tiff|tif)$/i, '.png', '.png');
            }
            return prefix + filename;
        });
    }

    setMaterialOfObj(materialName, objectName){
        this.#model.setMaterial(materialName, objectName);
    }

    // delete the material from the model
    deleteMaterial(name, objectName){
        this.#model.deleteMaterial(name);
        this.#model.resetMaterialOfObject(objectName);
    }

    // Load in one or multiple textures and save it to the model
    async loadTexture(file){
        const name = file.name;
        const reader = new FileReader();
        const promise = new Promise(resolve => {
            reader.onload = (event) => {
                const contents = event.target.result;
                const extension = name.split('.').pop().toLowerCase();
                if (extension == "tif" || extension == "tiff") {
                    this.convertTiffToPng(contents)
                        .then(pngDataUrl => {
                            console.log('PNG Data URL:', pngDataUrl);
                            this.parseTexture(pngDataUrl, name.replace(/\.[^/.]+$/, "") + ".png");
                            resolve();
                        })
                        .catch(error => {
                            console.error(error);
                        });
                } else {
                    this.parseTexture(contents, name);
                    resolve();
                }

            };
        });
        reader.readAsDataURL(file);
        return promise;
    }

    convertTiffToPng(tiffDataUrl) {
        return new Promise((resolve, reject) => {
            // Convert data URL to ArrayBuffer
            const data = atob(tiffDataUrl.split(',')[1]);
            const buffer = new ArrayBuffer(data.length);
            const view = new Uint8Array(buffer);
            for (let i = 0; i < data.length; i++) {
                view[i] = data.charCodeAt(i);
            }
    
            // Use UTIF to parse the TIFF data
            try {
                const ifds = UTIF.decode(buffer);  // Decode TIFF data
                UTIF.decodeImage(buffer, ifds[0]); // Decode the first image in the TIFF file
                const rgba = UTIF.toRGBA8(ifds[0]); // Get RGBA array
    
                // Create a canvas and draw the image
                const canvas = document.createElement('canvas');
                canvas.width = ifds[0].width;
                canvas.height = ifds[0].height;
                const ctx = canvas.getContext('2d');
                const imgData = new ImageData(new Uint8ClampedArray(rgba), ifds[0].width, ifds[0].height);
                ctx.putImageData(imgData, 0, 0);
    
                // Convert the canvas to a PNG data URL
                const pngDataUrl = canvas.toDataURL('image/png');
                resolve(pngDataUrl);
            } catch (e) {
                reject(e);
            }
        });
    }
    

    // Parse the texture and save it to the model
    parseTexture(contents, name){
        console.log("now parsing texture");
        const texture = this.#textureLoader.load(contents);
        // get the name of the texture
        const textureName = name;

        // save the texture to the model
        this.#model.addTexture(textureName,texture,contents);
    }


    // delete the texture from the model
    delete(name, type){
        if(type === "object") {
            this.#model.removeObject(name);
        }else if(type.includes("Light")) {
            this.#model.removeLight(name);
        }
    }

    addObjectToSceneAndModel(label, object, objectString){
       return this.#model.addObject(label, object, objectString);
    }

    setWorkspace(workspace){
        this.#workspace = workspace;
    }

    getWorkspace(){
        return this.#workspace;
    }

    // Load a scene from a .yaml file
    async delegateYamlImport(yaml){
        // load the contents of the yaml file with the file reader
        const reader = new FileReader();
        const promise = new Promise(resolve => {
            reader.onload = (event) => {
                const contents = event.target.result;
                this.#model.fromYaml(contents);
                resolve();
            };
        });
        reader.readAsText(yaml);
        return promise;
    }

    // clone the object or light with the given name
    clone(name, type){
        if(type === "object"){
            let object = this.#model.getObject(name);
            let newObject = object.clone();
            this.#model.addObject(name, newObject);
        }else if(type.includes("Light")){
            let light = this.#model.getLight(name);
            let newLight = light.clone();
            this.#model.addLight(name, newLight)
        }
    }

    addPointLight(){
        const light = new THREE.PointLight();
        light.position.set( 0,0,0 );
        this.#model.addLight("point_light", light);
    }

    addDirectionalLight(){
        const light = new THREE.DirectionalLight();
        this.#model.addLight("directional_light", light);
    }

    // Function to export data to a backend service
    export(callback){
        let yaml = this.#model.asYaml();

        console.log(yaml);

        let body = {
            "yaml": yaml
        }

        fetch(`${this.#url}/export`, {
            method: 'POST',
            body: JSON.stringify(body)
        })
        .then(response => {
            if(response.ok){
                console.log("export successful");
                return response.json()
            }else{
                response.text().then(text => {
                    this.showBackendMessage(response.ok, text);
                });
                // throw non-sense just to skip the next thenable
                throw 0;
            }
        })
        .then(data =>{
            let image = data.image;
            const { width, height, pixels } = this.parsePPM(image);

            const imageData = new ImageData(new Uint8ClampedArray(pixels), width, height);

            this.#model.setImage(imageData);
            this.#view.showImage(imageData);
            callback();
        })
        .catch(() => {
            console.log("export failed");
            callback();
        });
    }

    

    // this will render the next frame of the animation
    renderShit(){
        let framesToRender = this.#animator.getNumberOfFrames();
        console.log(framesToRender)
        for(let i = 0; i < framesToRender; i++){
           this.#animator.animate(i);

            this.exportAnimation(i);
        }

    }

    async downloadRawFrames() {
        const zip = new JSZip();

        let imageCount = this.#imageMap.size;
        // calculate the number of digits needed for the frame index
        let digits = Math.floor(Math.log10(imageCount))+1;


        for (let [frameIndex, imageData] of this.#imageMap.entries()) {
            // Format the frame index with leading zeros based on the total digits
            let formattedIndex = String(frameIndex).padStart(digits, '0');

            // Create a canvas to draw the image
            const canvas = document.createElement('canvas');
            canvas.width = imageData.width;
            canvas.height = imageData.height;
            const ctx = canvas.getContext('2d');
            ctx.putImageData(imageData, 0, 0);

            // Convert canvas to blob and add to zip
            const blob = await new Promise(resolve => canvas.toBlob(resolve, 'image/png'));
            zip.file(`frame-${formattedIndex}.png`, blob);;
        }

        // Generate the zip file and trigger the download
        zip.generateAsync({ type: "blob" })
            .then(function (blob) {
                const url = URL.createObjectURL(blob);

                // Create a temporary link to trigger the download
                const a = document.createElement('a');
                a.href = url;
                a.download = "frames.zip";
                document.body.appendChild(a);
                a.click();
                document.body.removeChild(a);

                // Release the Object URL
                URL.revokeObjectURL(url);
            });
    }

    finishAnimation(){

        // popup dialog if user wants to download the raw frames
        let download = confirm("Do you want to download the raw frames?");
        if(download){
            this.downloadRawFrames();
        }

        console.log("All imanuels are rendered");

        const frameRate = 30;
        const frameDuration = 1000 / frameRate; // Duration of each frame in milliseconds

        const canvas = document.createElement('canvas');
        const ctx = canvas.getContext('2d');
        canvas.width = this.#imageMap.get(0).width;
        canvas.height = this.#imageMap.get(0).height;


        const stream = canvas.captureStream(frameRate);
        const mediaRecorder = new MediaRecorder(stream, { mimeType: 'video/webm' });
        const chunks = [];

        mediaRecorder.ondataavailable = (event) => {
            if (event.data.size > 0) {
                chunks.push(event.data);
            }
        };

        mediaRecorder.onstop = () => {
            const blob = new Blob(chunks, { type: 'video/webm' });
            const videoUrl = URL.createObjectURL(blob);

            // Create a video element and append it to the body
            const video = document.createElement('video');
            video.controls = true;
            video.src = videoUrl;
            document.body.appendChild(video);
        };

        mediaRecorder.start();

        let frameIndex = 0;
        let lastFrameTime = performance.now();

        const drawFrame = () => {
            let now = performance.now();
            let elapsed = now - lastFrameTime;

            if (elapsed >= frameDuration) {
                lastFrameTime = now - (elapsed % frameDuration);

                if (frameIndex < this.#imageMap.size) {
                    ctx.putImageData(this.#imageMap.get(frameIndex), 0, 0);
                    frameIndex++;
                }
            }

            if (frameIndex < this.#imageMap.size) {
                requestAnimationFrame(drawFrame);
            } else {
                setTimeout(() => mediaRecorder.stop(), frameDuration);
            }
        };

        requestAnimationFrame(drawFrame);
        
    }


    exportAnimation(i){
        let yaml = this.#model.asYaml();

        console.log(yaml);

        let body = {
            "yaml": yaml,
        }

        fetch(`${this.#url}/export`, {
            method: 'POST',
            body: JSON.stringify(body)
        })
            .then(response => {
                if(response.ok){
                    console.log("export successful");
                    return response.json()
                }else{
                    response.text().then(text => {
                        this.showBackendMessage(response.ok, text);
                    });
                    // throw non-sense just to skip the next thenable
                    throw 0;
                }
            })
            .then(data =>{
                let image = data.image;
                const { width, height, pixels } = this.parsePPM(image);

                const imageData = new ImageData(new Uint8ClampedArray(pixels), width, height);

                this.#imageMap.set(i, imageData);
                if(this.#imageMap.size === this.#animator.getNumberOfFrames()){
                    this.finishAnimation();
                }


            })
            .catch(() => {
                console.log("export failed");

            });
    }

    setAnimator(animator){
        this.#animator = animator;
    }

  // Function to parse a PPM image
    parsePPM(ppmContent) {
        const lines = ppmContent.trim().split('\n');
        const [width, height] = lines[1].split(' ').map(Number);

        // Extract pixel data from the PPM content
        const pixels = [];
        for (let i = 3; i < lines.length; i++) {
            const row = lines[i].split(' ').map(Number);

            // Ensure that pixel values are in the [0, 255] range
            const sanitizedRow = row.map(value => Math.max(0, Math.min(255, value)));
            // As the alpha channel must be set but is not contained in the ppm format it is added here with a default value
            sanitizedRow.push(255)

            pixels.push(...sanitizedRow);
        }

        return { width, height, pixels };
    }


    // Set the camera position
    setCameraPosition(x, y, z){
        this.#model.setCameraPosition(x, y, z);
    }

    // Set the camera rotation
    setCameraRotation(x, y, z){
        this.#model.setCameraRotation(x, y, z);
    }

    setView(v){
        this.#view = v;
        this.#view.update(this.#model);
    }

    getWidth() {
        return this.#model.getWidth();
    }

    setWidth(width) {
        this.#model.setWidth(width);
    }

    getHeight() {
        return this.#model.getHeight();
    }

    setHeight(height) {
        this.#model.setHeight(height);
    }

    getBounces() {
        return this.#model.getBounces();
    }

    setBounces(bounces) {
        this.#model.setBounces(bounces);
    }

    getSamples(){
        return this.#model.getSamples();
    }

    setSamples(samples) {
        this.#model.setSamples(samples);
    }

    showBackendMessage(success, msg) {
        console.log(msg);
        let theme = success ? "success":"danger";
        this.#view.displayMessage(theme, msg);
    }

    downloadYaml() {
        return this.#model.asYaml();

    }

    getAllTextures() {
        return this.#model.getTexturesString();

    }

    getAllMaterials() {
        return this.#model.getMaterialsString();

    }

    getAllObjects() {
        return this.#model.getObjsString();

    }



}
export default Controller;