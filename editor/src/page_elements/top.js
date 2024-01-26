import {System} from "three/addons/libs/ecsy.module.js";
import JSZip from "jszip";

class Top{

    #view;
    #controller

    #importBtnDropdown
    #importDropdown
    sceneInputYaml;
    #loadSceneButtonYaml;
    #loadSceneButtonZip;
    #objInput;
    #loadObjButton;

    #lightDropdownButton;
    #lightDropdown;
    #pointLightButton;
    #directionalLightButton;

    #exportBtn;
    #showResultButton;

    constructor(view, controller) {
        this.#view = view;
        this.#controller = controller;

        this.#importBtnDropdown = document.getElementById('importBtnDropdown');
        this.#importDropdown = document.getElementById('importDropdown');

        this.sceneInputYaml = document.getElementById('sceneInputYaml');
        this.#loadSceneButtonYaml = document.getElementById('ImportSceneBtnYaml');

        this.sceneInputZip = document.getElementById('sceneInputZip');
        this.#loadSceneButtonZip = document.getElementById('ImportSceneBtnZip');

        this.#objInput = document.getElementById('objInput');
        this.#loadObjButton = document.getElementById('ImportObjBtn');

        this.#lightDropdownButton = document.getElementById('lightDropdownBtn');
        this.#lightDropdown = document.getElementById('lightDropdown');
        this.#pointLightButton = document.getElementById('pointLightBtn');
        this.#directionalLightButton = document.getElementById('directionalLightBtn');

        this.#exportBtn = document.getElementById("exportBtn");
        this.#showResultButton = document.getElementById('showResultBtn');

        this.intializeEventListeners();
    }

    // Add event listeners to delegate the button presses to the according logic.
    intializeEventListeners(){

        // event fires when the import dropdown button is pressed
        this.#importBtnDropdown.addEventListener('click', () => {
            this.#importDropdown.classList.toggle("show");
        });

        // event fires when the obj import button is pressed
        this.#loadObjButton.addEventListener('click', () => {
            this.#objInput.click();
        });

        // event fires when the scene import button is pressed
        this.#loadSceneButtonYaml.addEventListener('click', () => {
            this.sceneInputYaml.click();
        });

        this.#loadSceneButtonZip.addEventListener('click', () => {
            this.sceneInputZip.click();
        });

        // event fires when an obj file is selected
        this.sceneInputYaml.addEventListener('input', (event) => {
            const file = event.target.files[0];
            if (file) {
                document.getElementById('loadingOverlay').style.display = "flex";

                // Pass a callback function to the controller
                this.#controller.readScene(file, () => {
                    document.getElementById('loadingOverlay').style.display = "none";
                });

                this.sceneInputYaml.value = null;
            }
        });

        this.sceneInputZip.addEventListener('input', (event) => {
            const file = event.target.files[0];
            if (file) {
                document.getElementById('loadingOverlay').style.display = "flex";

                // Pass a callback function to the controller
                this.#controller.readScene(file, () => {
                    document.getElementById('loadingOverlay').style.display = "none";
                });

                this.sceneInputZip.value = null;
            }
        });

        // event fires when an obj file is selected
        this.#objInput.addEventListener('input', (event) => {
            const file = event.target.files[0];
            if (file) {
                console.log("file selected calling controller");
                document.getElementById('loadingOverlay').style.display = "flex";

                this.#controller.getWorkspace().loadObject(file);

                // Pass a callback function to the controller
                this.#controller.readObj(file, () => {
                    document.getElementById('loadingOverlay').style.display = "none";
                    console.log("deactivating");
                });

                this.#objInput.value = null;
            }
        });

        // event fires when the light dropdown button is pressed
        this.#lightDropdownButton.addEventListener('click', () => {
            this.#lightDropdown.classList.toggle("show");
        });
        // event fires when the point light button is pressed
        this.#pointLightButton.addEventListener('click', () => {
            this.#controller.addPointLight();

        });
        // event fires when the directional light button is pressed
        this.#directionalLightButton.addEventListener('click', () => {
            this.#controller.addDirectionalLight();
        });

        // event fires when the renderening process should be started
        this.#exportBtn.addEventListener('click', () => {
            this.#exportBtn.innerHTML = `<img width="16" height="16" src="media/loader.svg" alt="Loading...">`;
            this.#controller.export(() => {
                this.#exportBtn.innerHTML = `<i class="bi bi-skip-start-fill"></i>`;
            });
        })
        // event fires when the result popup should be openend.
        document.getElementById("showResultBtn").addEventListener('click', () => {
            this.#view.showPopup();
        })

        document.addEventListener('click', (event) => {
            // Check if the click is outside the dropdown
            if (!this.#importBtnDropdown.contains(event.target) && !this.#importDropdown.contains(event.target)) {
                // If the dropdown is shown, hide it
                if (this.#importDropdown.classList.contains("show")) {
                    this.#importDropdown.classList.remove("show");
                }
            }
            if (!this.#lightDropdownButton.contains(event.target) && !this.#lightDropdown.contains(event.target)) {
                // If the dropdown is shown, hide it
                if (this.#lightDropdown.classList.contains("show")) {
                    this.#lightDropdown.classList.remove("show");
                }
            }
        });

        document.getElementById('downloadSceneBtn').addEventListener('click', () => {
            this.downLoadScene();
        });

        document.getElementById("renderAnimationBtn").addEventListener('click', () => {
            console.log("rendering animation")
            this.#controller.renderShit();
        })
    }

    // Shows the button which can be pressed to show the result.
    showShowResultButton(){
        this.#showResultButton.style.display = "initial";
    }

    // Hides the button which can be pressed to show the result.
    hideShowResultButton(){
        this.#showResultButton.style.display = "none";
    }

    // Downloads the complete scene including all objects, materials, textures and the yaml config.
    async downLoadScene() {

        document.getElementById('loadingOverlay').style.display = "flex";

        const zip = new JSZip();

        // Retrieve the YAML string from your model or controller
        const yamlString = this.#controller.downloadYaml();
        zip.file("config.yaml", yamlString);

        const objectMap = this.#controller.getAllObjects();
        objectMap.forEach((value, key) => {       
            zip.file(key, value);
        });

        const textureMap = this.#controller.getAllTextures();
        textureMap.forEach((value, key) => {
            // Convert the data URL to a blob
            const blob = this.dataUrlToBlob(value);     
            zip.file(key, blob);
        });

        const materialMap = this.#controller.getAllMaterials();
        materialMap.forEach((value, key) => {
            zip.file(key, value);
        });


        try {
            const zipBlob = await zip.generateAsync({ type: "blob" });

            // Create a URL for the blob
            const url = URL.createObjectURL(zipBlob);

            // Create a temporary link to trigger the download
            const anchor = document.createElement('a');
            anchor.href = url;
            anchor.download = 'scene.zip';
            document.body.appendChild(anchor);
            anchor.click();

            // Clean up the URL and remove the anchor from the document
            document.body.removeChild(anchor);
            URL.revokeObjectURL(url);
        } catch (error) {
            console.error('Error creating zip file', error);

        }
        finally {
            document.getElementById('loadingOverlay').style.display = "none";
        }

    }

    // converts a data URL into a blob, this is necessary for encoding images into the right way so they can be saved as a file
    dataUrlToBlob(dataUrl) {
        // Split the data URL at the comma to separate the base64 encoding from the header
        const parts = dataUrl.split(',');
        const base64 = parts[1];
        const mime = parts[0].split(':')[1].split(';')[0];
    
        // Convert Base64 to raw binary data held in a string
        const binary = atob(base64);
    
        // Create a Uint8Array from the binary string
        const array = [];
        for (let i = 0; i < binary.length; i++) {
            array.push(binary.charCodeAt(i));
        }
    
        // Return our Blob object
        return new Blob([new Uint8Array(array)], { type: mime });
    }
    

}

export {Top};