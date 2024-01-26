import * as THREE from 'three';
import {Inspector} from "./inspector.js";


class ObjectInspector extends Inspector{

    #rotationX;
    #rotationY;
    #rotationZ;

    #scaleX;
    #scaleY;
    #scaleZ;

    // Materials
    #materialDropdownButton;
    #materialInput;
    #materialDropdown;
    #selectMaterialButton;
    #currentMtl;

    // Textures
    #textureDropdownButton;
    #textureInput;
    #textureDropdown;
    #selectTextureButton;
    #currentTex;

    constructor(view, controller) {
        super(view, controller);
        this.#rotationX = document.getElementById('rotationX');
        this.#rotationY = document.getElementById('rotationY');
        this.#rotationZ = document.getElementById('rotationZ');

        this.#scaleX = document.getElementById('scaleX');
        this.#scaleY = document.getElementById('scaleY');
        this.#scaleZ = document.getElementById('scaleZ');

        // Materials
        this.#materialDropdownButton = document.getElementById('materialDropdownBtn');
        this.#materialInput = document.getElementById('mtlInput');
        this.#materialDropdown = document.getElementById('materialDropdown');
        this.#selectMaterialButton = document.getElementById('selectMaterialBtn');

        // Textures
        this.#textureDropdownButton = document.getElementById('textureDropdownBtn');
        this.#textureInput = document.getElementById('texInput');
        this.#textureDropdown = document.getElementById('textureDropdown');
        this.#selectTextureButton = document.getElementById('selectTextureBtn');

        this.initializeMaterialSelection();
        this.initializedTextureSelection();
    }

    show(name) {
        super.show(name);
        const obj = this._view.getObject(name);

        // Populate the inspector with the current values
        this._positionX.value = obj.position.x;
        this._positionY.value = obj.position.y;
        this._positionZ.value = obj.position.z;

        this.#rotationX.value = THREE.MathUtils.radToDeg(obj.rotation.x);
        this.#rotationY.value = THREE.MathUtils.radToDeg(obj.rotation.y);
        this.#rotationZ.value = THREE.MathUtils.radToDeg(obj.rotation.z);

        this.#scaleX.value = obj.scale.x;
        this.#scaleY.value = obj.scale.y;
        this.#scaleZ.value = obj.scale.z;

        this.#currentMtl = this._view.getMaterialFromObject(name);
        this.#materialDropdownButton.innerHTML = this.#currentMtl != null ? "selected Material: " + this.#currentMtl : "Select Material";

        // Show the necessary inspector parts
        this._objectInspector.style.display = 'block';
        this._materialInspector.style.display = 'block';
        this._textureInspector.style.display = 'block';
        this._scaleInspector.style.display = 'block';


        // Add the new EventHandler
        this._applyChangesButton.addEventListener('click', () => {
            this.onApplyChanges(name);
        });
    }

    // Parse the inspector values and apply them to the object.
    onApplyChanges(name) {
        super.onApplyChanges(name);

        let obj = this._view.getObject(name);
        // Set new rotation values (converted from degrees to radians)
        obj.rotation.x = THREE.MathUtils.degToRad(parseFloat(this.#rotationX.value));
        obj.rotation.y = THREE.MathUtils.degToRad(parseFloat(this.#rotationY.value));
        obj.rotation.z = THREE.MathUtils.degToRad(parseFloat(this.#rotationZ.value));

        // Set position values
        obj.position.x = parseFloat(this._positionX.value);
        obj.position.y = parseFloat(this._positionY.value);
        obj.position.z = parseFloat(this._positionZ.value);

        // Set scale values
        obj.scale.x = parseFloat(this.#scaleX.value);
        obj.scale.y = parseFloat(this.#scaleY.value);
        obj.scale.z = parseFloat(this.#scaleZ.value);

    }

    // Prepare the Material Dropdown so it can be used correctly.
    initializeMaterialSelection(){
        // Add event for material input
        this.#materialInput.addEventListener('input', (event) => {
            const files = event.target.files;
            if (files.length > 0) {
                for (let i = 0; i < files.length; i++) {
                    this._controller.loadMaterial(files[i]);
                }
                this.#materialInput.value = null;
            }
        });
        // Add Event for selecting a Material
        this.#selectMaterialButton.addEventListener('click', () => {
            this.#materialInput.click();
        });
        // Add Event for Material Dropdown
        this.#materialDropdownButton.addEventListener('click', () => {
            this.#materialDropdown.classList.toggle("show");

        });
    }

    // Append a material button to the material dropdown
    appendMaterialButton(label) {
        // Get the button list div
        const buttonListDiv = document.getElementById('materialDropdown');

        // Create a new button element
        const button = document.createElement('button');
        button.classList.add('materialBtn');
        button.setAttribute('id', label)
        button.textContent = label;
        // Instantly set the material of the current object to the selected material
        this._view.setCurrentMtl(label);

        button.addEventListener('click', (ev) => {
            // Set the material of the current object to the selected material
            this._view.setCurrentMtl(label);
        });

        // Create the trash button
        const trashButton = document.createElement('button');
        trashButton.classList.add('inspectorBtnTop', 'inspectorBtnTopTrash');
        trashButton.setAttribute('id', 'trashBtn' + label); // Unique ID for each trash button
        trashButton.setAttribute('title', 'Trash Button');
        trashButton.innerHTML = '<i class="bi bi-trash"></i>'; // Using Font Awesome trash icon
        trashButton.style.width = '30px';
        trashButton.style.height = '30px';
        trashButton.style.marginLeft = '5px';
        trashButton.addEventListener('click', () => {
            console.log("deleted material: " + label);
            if (this.#currentMtl === label) {
                this.#currentMtl = null;
                this.setDropdownText(null);
            }
            this._controller.deleteMaterial(label, this._view.getCurrentObject());
        });

        // Create the button div
        const buttonDiv = document.createElement('div');
        buttonDiv.setAttribute('id', 'buttonDiv' + label); // Unique ID for each button div
        buttonDiv.classList.add('materialButtonDiv', 'buttonRow');
        buttonDiv.appendChild(button);
        buttonDiv.appendChild(trashButton);
        // add the button to the div at the beginning
        buttonListDiv.insertBefore(buttonDiv, buttonListDiv.childNodes[0]);
    }

    // remove a material button from the material dropdown
    removeMaterialButton(label) {
        // Get the button list div
        const buttonListDiv = document.getElementById('materialDropdown');
        // Get the button div
        const buttonDiv = document.getElementById('buttonDiv' + label);
        // Remove the button div from the DOM
        buttonListDiv.removeChild(buttonDiv);
    }

    // Prepare the Texture Dropdown so it can be used correctly.
    initializedTextureSelection(){
        // Add event for texture input
        this.#textureInput.addEventListener('input', (event) => {
            const files = event.target.files;
            if (files.length > 0) {
                for (let i = 0; i < files.length; i++) {
                    this._controller.loadTexture(files[i]);
                }
                this.#textureInput.value = null;
            }
        });
        // Add Event for selecting a Texture
        this.#selectTextureButton.addEventListener('click', () => {
            this.#textureInput.click();
        });
        // Add Event for Texture Dropdown
        this.#textureDropdownButton.addEventListener('click', () => {
            this.#textureDropdown.classList.toggle("show");
        });
    }

    // Append a texture button to the texture dropdown
    appendTextureButton(label) {
        // Get the button list div
        const buttonListDiv = document.getElementById('textureDropdown');
        // Create a new button element
        const button = document.createElement('button');
        button.classList.add('textureBtn');
        button.setAttribute('id', label)
        button.textContent = label;
        // add the button to the div at the beginning
        buttonListDiv.insertBefore(button, buttonListDiv.childNodes[0]);
    }
    
    // function for setting the current material
    setDropdownText(label) {
        this.#currentMtl = label;
        this.updateDropdowns(label);
    }

    // Update the dropdowns
    updateDropdowns(label){
        this.#materialDropdownButton.innerHTML = this.#currentMtl != null ? "selected Material: " + this.#currentMtl : "Select Material";
    }
}

export {ObjectInspector};