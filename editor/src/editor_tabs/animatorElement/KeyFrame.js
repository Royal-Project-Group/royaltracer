class KeyFrame{

    // The index of the frame in the animation
    #frameIndex;

    // three js properties like position, rotation
    #position;
    #rotation;
    #keyFrameListId;
    constructor(position, rotation, frameIndex, keyFrameListId) {
        //important to clone the position and rotation, otherwise they will be the same object
        this.#position = position.clone();
        this.#rotation = rotation.clone();
        this.#frameIndex = frameIndex;
        this.#keyFrameListId = keyFrameListId;

        this.createKeyFrameElement();
    }

    getPosition(){
        return this.#position;
    }

    getRotation(){
        return this.#rotation;
    }

    getFrameIndex(){
        return this.#frameIndex;
    }

    createKeyFrameElement(){
        const keyFrameList = document.getElementById(this.#keyFrameListId);
        const keyFrameElement = document.createElement('li');
        keyFrameElement.innerHTML = this.#frameIndex;
        keyFrameList.appendChild(keyFrameElement);

    }
}
export default KeyFrame;