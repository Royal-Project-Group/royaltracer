import {Tab} from "./tab.js";


import * as THREE from 'three';
import {OBJLoader} from 'three/addons/loaders/OBJLoader.js';
import {MTLLoader} from "three/addons/loaders/MTLLoader.js";
import {Controller} from "../mvc/controller.js";

class WorkspaceTab extends Tab {

    #objectMap;
    #objectStringMap;
    #loader;
    #materialLoader;
    #textureLoader;

    #itemList;

    #controller;

    constructor(workspaceDivId, controller) {
        super(workspaceDivId);

        document.getElementById("loadDirectoryBtn").addEventListener("click", () => {
            this.loadDirectory();
        });

        this.#itemList = document.getElementById("itemList");

        this.#objectMap = new Map();
        this.#objectStringMap = new Map();

        this.#loader = new OBJLoader();
        this.#materialLoader = new MTLLoader();
        this.#textureLoader = new THREE.TextureLoader();

        this.#controller = controller;
    }

    addObject(label, object, objectString) {
        console.log("Adding object: " + label);
        this.#objectMap.set(label, object);
        this.#objectStringMap.set(label, objectString);

        const li = document.createElement("li");
        const btn = document.createElement("button");
        const img = document.createElement("img");
        img.className = "item-image";
        // make ./obj.png the default image
        img.src = "./media/obj.png";

        img.className = "item-image";
        const p = document.createElement("p");
        p.innerText = label;

        btn.appendChild(img);
        btn.appendChild(p);
        li.appendChild(btn);

        btn.addEventListener("click", () => {
            this.#controller.addObjectToSceneAndModel(label, object, objectString)
        });

        this.#itemList.appendChild(li);

        console.log("Added object: " + label);
    }

    contains(label) {
        return this.#objectMap.has(label);
    }

    loadDirectory() {
        const input = document.createElement("input");
        input.type = "file";
        input.multiple = true;
        input.directory = true;
        input.webkitdirectory = true;
        input.mozdirectory = true;
        input.msdirectory = true;
        input.odirectory = true;
        input.directory = true;
        input.addEventListener("change", (event) => {
            const files = event.target.files;
            this.loadFiles(files);
        });
        input.click();
    }

    async loadFiles(files) {
        document.getElementById('loadingOverlay').style.display = "flex";

        let objectCount = 0;
        let textureCount = 0;

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

        // first of all load all textures
        for (const file of files) {
            textureCount += 1;
            // if file is a texture -> in the mime_types
            if (mime_types[file.name.split('.').pop()] !== undefined) {
               await this.#controller.loadTexture(file);

            }
        }

        for (const file of files) {
            if (file.name.endsWith(".mtl")) {
                await this.#controller.loadMaterial(file);
            }
        }

        for (const file of files) {
            if (file.name.endsWith(".obj")) {
                objectCount += 1;
            }
        }

        // then load all objects
        for (const file of files) {
            if (file.name.endsWith(".obj")) {
                this.loadObject(file,
                    async () => {
                        objectCount -= 1;
                        if (objectCount === 0) {
                            for (const file of files) {
                                if (file.name.endsWith(".yaml")) {
                                    await this.#controller.delegateYamlImport(file);
                                }
                            }
                            document.getElementById('loadingOverlay').style.display = "none";
                        }
                    }
                );
            }
        }


    }

    loadTextureFiles(files) {

    }

    loadMaterialFiles(files) {

    }
    loadObjectFiles(files) {

    }

    loadObject(file, callback) {
        const reader = new FileReader();
        console.log("Loading object: " + file.name);
        reader.onload = (event) => {
            const contents = event.target.result;
            const objName = file.name;
            const obj = this.#loader.parse(contents);
            console.log("Parsed object: " + obj);
            this.addObject(objName, obj, contents);

            if (callback && typeof callback === 'function') {
                callback();
            }
        };
        reader.readAsText(file);
    }

    loadObjectFromString(fileString, objName) {
        const obj = this.#loader.parse(fileString);
        console.log("Parsed object: " + obj);
        this.addObject(objName, obj, fileString);
    }

    addObjectFromWorkspaceToSceneAndModel(label) {
        const object = this.#objectMap.get(label);
        const objectString = this.#objectStringMap.get(label);
        return this.#controller.addObjectToSceneAndModel(label, object, objectString);
    }


}

export {WorkspaceTab};