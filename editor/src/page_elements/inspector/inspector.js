import {EnumTypes} from "../../enumTypes.js";

class Inspector{

    _view;
    _controller;

    _objectHeaderName;

    _positionX;
    _positionY;
    _positionZ;

    _objectInspector;
    _materialInspector;
    _textureInspector;
    _scaleInspector;
    _lightInspector;
    _cameraInspector;

    _currentInspector;
    _cloneAndTrashButton;
    _standardInspector;
    _positionLabel;

    _applyChangesButton;


    constructor(view, controller) {
        this._view = view;
        this._controller = controller;

        this._objectHeaderName = document.getElementById('objectNameHeader');

        // get the textfields for the position
        this._positionX = document.getElementById('positionX');
        this._positionY = document.getElementById('positionY');
        this._positionZ = document.getElementById('positionZ');

        // get apply changes button
        this._applyChangesButton = document.getElementById('applyChanges');

        // get all the different inspectors
        this._objectInspector = document.getElementById('objectInspector');
        this._materialInspector = document.getElementById('materialInspector');
        this._textureInspector = document.getElementById('textureInspector');
        this._scaleInspector = document.getElementById('scaleInspector');
        this._lightInspector = document.getElementById('lightInspector');
        this._cameraInspector = document.getElementById('cameraInspector');

        this._currentInspector = document.getElementById('currentInspector');
        this._cloneAndTrashButton = document.getElementById('cloneAndTrashButton');
        this._standardInspector = document.getElementById('standardInspector');
        this._positionLabel = document.getElementById('positionLabel');
    }

    // Prepares the inspector area so the concrete implementations can show their fields correctly.
    show(name){
        // Remove all Helper objects from the scene, as they should only be displayed when the according object/light is selected
        this._view.removeAllHelper();

        this._objectHeaderName.innerHTML = name;

        // Show the inspector (surrounding div and standard inspector)
        this._currentInspector.style.display = 'block';
        this._standardInspector.style.display = 'block';
        this._cloneAndTrashButton.style.display = 'block';
        this._positionLabel.innerHTML = "Position:";

        // hide all inspector parts so the actual implementations just have to show the ones they need
        this._objectInspector.style.display = 'none';
        this._materialInspector.style.display = 'none';
        this._lightInspector.style.display = 'none';
        this._scaleInspector.style.display = 'none';
        this._cameraInspector.style.display = 'none';
        this._textureInspector.style.display = 'none';

        // Remove all current EventHandlers of the "Apply Changes" Button as each inspector will add it's own functionality
        var old_element = document.getElementById('applyChanges');
        var new_element = old_element.cloneNode(true);
        old_element.parentNode.replaceChild(new_element, old_element);
        this._applyChangesButton = document.getElementById('applyChanges');
    }

    // Hides the complete inspector.
    hide(){
        this._currentInspector.style.display = 'none';
    }

    
    // His method has to be implemented by the concrete inspector implementations.
    onApplyChanges(name){
        console.log(name + " is being updated.")
    }

}

export {Inspector};