class Popup{

    #popupContainer;
    #popupImage;
    #closePopupBtn;

    constructor() {
        this.#popupContainer = document.getElementById('resultPopup');
        this.#popupImage = document.getElementById('popupImage');

        this.#closePopupBtn = document.getElementById('closePopup');

        // Set the event listeners
        document.getElementById("saveRenderBtn").addEventListener("click", () => {
            this.saveImage("render.png");
        })
        document.getElementById('closePopup').addEventListener('click', () => {
            this.#popupContainer.style.display = "none";
        })
    }

    // Open the popup with the given image
    showImage(imageData) {
        this.#popupImage.src = this.imagedataToImage(imageData).src;
        this.show();
    }

    // Open the popup
    show(){
        this.#popupContainer.style.display = "flex";
    }

    // Converts imageData into an image object
    imagedataToImage(imageData) {
        var canvas = document.createElement('canvas');
        var ctx = canvas.getContext('2d');
        canvas.width = imageData.width;
        canvas.height = imageData.height;
        ctx.putImageData(imageData, 0, 0);

        var image = new Image();
        image.src = canvas.toDataURL();
        return image;
    }

    // Downloads the image from the popup as an png.
    saveImage(suggestedName) {
        let img = document.getElementById("popupImage");

        let c = document.createElement("canvas");
        let ctx = c.getContext("2d");
        c.width = img.naturalWidth;
        c.height = img.naturalHeight;
        ctx.drawImage(img,0,0);
        c.toBlob((blob) => {
            if(blob) {
                this.saveFile(blob, suggestedName)
                    .then(r => console.log("Image saved successfully"))
                    .catch(e => {
                        // NOT an abort error
                        if(e.code !== 20) {
                            console.error(e)
                        }
                    });
            }
        }, "image/png");
    }

    // Source: https://web.dev/patterns/files/save-a-file/
    // Uses the new File System API if supported, otherwise uses the traditional approach.
    async saveFile(blob, suggestedName) {
        // Feature detection. The API needs to be supported
        // and the app not run in an iframe.
        const supportsFileSystemAccess =
            'showSaveFilePicker' in window &&
            (() => {
                try {
                    return window.self === window.top;
                } catch {
                    return false;
                }
            })();
        // If the File System Access API is supported…
        if (supportsFileSystemAccess) {
            // Show the file save dialog.
            const handle = await window.showSaveFilePicker({
                startIn: "downloads",
                suggestedName: suggestedName,
                types: [
                    {
                        description: "PNG file",
                        accept: {"image/png": [".png"] }
                    }
                ]
            });
            // Write the blob to the file.
            const writable = await handle.createWritable();
            await writable.write(blob);
            await writable.close();
            return;
        }
        // Fallback if the File System Access API is not supported…
        // Create the blob URL.
        const blobURL = URL.createObjectURL(blob);
        // Create the `<a download>` element and append it invisibly.
        const a = document.createElement('a');
        a.href = blobURL;
        a.download = suggestedName;
        a.style.display = 'none';
        document.body.append(a);
        // Programmatically click the element.
        a.click();
        // Revoke the blob URL and remove the element.
        setTimeout(() => {
            URL.revokeObjectURL(blobURL);
            a.remove();
        }, 1000);
    };
}

export {Popup};