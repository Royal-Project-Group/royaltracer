import * as THREE from 'three';
import {Inspector} from "./inspector.js";

class CameraInspector extends Inspector {

    #rotationX;
    #rotationY;
    #rotationZ;

    #fov;
    #width;
    #height;
    #bounces;
    #samples;

    constructor(view, controller) {
        super(view, controller);

        this.#rotationX = document.getElementById('rotationX');
        this.#rotationY = document.getElementById('rotationY');
        this.#rotationZ = document.getElementById('rotationZ');

        this.#fov = document.getElementById('fov');
        this.#width = document.getElementById('width');
        this.#height = document.getElementById('height');
        this.#bounces = document.getElementById('bounces');
        this.#samples = document.getElementById('samples');
    }

    // Contains the logic which should be executed to show all necessary inspector parts
    show(name) {
        super.show(name);
        const camera = this._view.getCamera();

        // Populate the inspector with the current values
        this._positionX.value = camera.position.x;
        this._positionY.value = camera.position.y;
        this._positionZ.value = camera.position.z;

        this.#rotationX.value = THREE.MathUtils.radToDeg(camera.rotation.x);
        this.#rotationY.value = THREE.MathUtils.radToDeg(camera.rotation.y);
        this.#rotationZ.value = THREE.MathUtils.radToDeg(camera.rotation.z);

        this.#fov.value = camera.fov;
        this.#width.value = this._controller.getWidth();
        this.#height.value = this._controller.getHeight();
        this.#bounces.value = this._controller.getBounces();
        this.#samples.value = this._controller.getSamples();

        // Show the necessary inspector parts
        this._objectInspector.style.display = 'block';
        this._cameraInspector.style.display = 'block';
        // Don't show clone and trash button as the camera cannot be deleted
        this._cloneAndTrashButton.style.display = 'none';

        // Add the new EventHandler
        this._applyChangesButton.addEventListener('click', () => {
            this.onApplyChanges(name);
        });
    }

    // Parses all the values set in the inspector fields and applies them to the camera.
    onApplyChanges(name) {
        super.onApplyChanges(name);
        let cam = this._view.getCamera();

        cam.position.x = parseFloat(this._positionX.value);
        cam.position.y = parseFloat(this._positionY.value);
        cam.position.z = parseFloat(this._positionZ.value);

        cam.position.x = parseFloat(this._positionX.value);
        cam.position.y = parseFloat(this._positionY.value);
        cam.position.z = parseFloat(this._positionZ.value);

        cam.rotation.x = THREE.MathUtils.degToRad(parseFloat(this.#rotationX.value));
        cam.rotation.y = THREE.MathUtils.degToRad(parseFloat(this.#rotationY.value));
        cam.rotation.z = THREE.MathUtils.degToRad(parseFloat(this.#rotationZ.value));

        cam.fov = parseFloat(this.#fov.value);
        this._controller.setWidth(parseFloat(this.#width.value));
        this._controller.setHeight(parseFloat(this.#height.value));
        this._controller.setBounces(parseFloat(this.#bounces.value));
        this._controller.setSamples(parseFloat(this.#samples.value));

        this._controller.setCameraPosition(cam.position.x, cam.position.y, cam.position.z);
        this._controller.setCameraRotation(cam.rotation.x, cam.rotation.y, cam.rotation.z);

        cam.updateProjectionMatrix();
    }

}

export {CameraInspector};