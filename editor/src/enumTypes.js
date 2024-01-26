const EnumTypes = {
    OBJECT: "object",
    CAMERA: "camera",
    DIRECTIONAL_LIGHT: "directionalLight",
    POINT_LIGHT: "pointLight",
}

const TabEditorTypes = {
    OVERVIEW: "overviewDiv",
    WORKSPACE: "workspace",
    ANIMATOR: "animator",
}


// method that returns bool if object is a light
function isLight(renderType) {
    return renderType.includes("Light");
}

export {EnumTypes, TabEditorTypes, isLight};