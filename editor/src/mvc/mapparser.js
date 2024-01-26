
class MapParser {
    /*
     function for parsing the map_kd string
     the mapKdString is a string of the form:
     map_kd -s 1 1 1 -o 0 0 0 -mm 0 1 -bm 0 1 texture.png
     there is also that the texture path is relative to the .mtl file
     map_kd -s 1 1 1 -o 0 0 0 -mm 0 1 -bm 0 1 home/documents/mtl/texture.png
     TODO so for further use we need to cut texture.png from the string
     the function returns a map with the following keys:
     filename, scale, offset, range, bump
     */
    parseMap(mapKdString) {
        const result = new Map();
        const mapKdArray = mapKdString.split(" ");
        const filename = mapKdArray.pop();

        function parseSection(startToken) {
            const section = [];
            let index = mapKdArray.indexOf(startToken) + 1;

            while (index < mapKdArray.length && !["-s", "-o", "-mm","-bm"].includes(mapKdArray[index])) {
                section.push(mapKdArray[index]);
                index++;
            }

            return section;
        }

        const scale = parseSection("-s");
        const offset = parseSection("-o");
        const range = parseSection("-mm");
        const bump = parseSection("-bm");

        result.set("filename", filename);
        result.set("scale", scale);
        result.set("offset", offset);
        result.set("range", range);

        return result;
    }
}

export default MapParser;