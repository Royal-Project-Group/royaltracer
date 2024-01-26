// Sources:
// https://github.com/lambdaxymox/cgwavefront_obj/blob/master/GRAMMAR.md
// https://en.wikipedia.org/wiki/Wavefront_.obj_file
// https://en.cppreference.com/w/c/io/fopen
// https://de.wikipedia.org/wiki/Bottom-up-Parser
// https://de.wikipedia.org/wiki/LR-Parser
// https://en.wikipedia.org/wiki/Types_of_mesh
// https://en.wikipedia.org/wiki/Polygon_mesh
// https://serokell.io/blog/how-to-implement-lr1-parser

#include <fstream>
#include "OBJParser.hpp"

using float3 = std::array<float, 3>;

void OBJParser::determineElement(std::string line, std::shared_ptr<ThreeDObject> obj) {
    if(line.empty()) {
        return;
    }
    std::vector<std::string> lntokens = lexpp::lex(line, " \t", false);
    const std::string &token = lntokens[0];

    if (token == "o") {
        // object name
    } else if (token == "g") {
        // group
    } else if (token == "s") {
        // smoothing group
    } else if (token == "#") {
        return;
        // comment
        return;
    } else if (token == "v") {
        // vertex
        // remove element
        vertices.push_back(parseVertexData<3>(lntokens));
    } else if (token == "vt") {
        // texture coordinate
        txt_coords.push_back(parseVertexData<3>(lntokens));
    } else if (token == "vn") {
        // vertex normal
        normals.push_back(parseVertexData<3>(lntokens));
    } else if (token == "vp") {
        // parameterized vertex
    } else if (token == "p") {
        // point
    } else if (token == "l") {
        // line
    } else if (token == "f") {
        // face
        try {
            ParsedTriangle parsed_t = parseFace(lntokens);
            obj->triangles.push_back(simplifyTriangle(parsed_t));
        } catch (std::exception exception) {
            // check if exception was caused by stoi and return corresponding error message
            std::string what = exception.what();
            if (what.contains("stoi")) {
                throw std::invalid_argument("OBJParser: obj file passed non-numeric value to face element!");
            }
            throw exception;
        }
    } else if (token == "mtllib") {
        // parse MTL file
        std::string content = MtlStorageSystem::get_instance().fetch(lntokens[1]);
        MTLParser mtlp = MTLParser{};
        std::vector<Material> new_materials = *mtlp.parse(content);
        // add new materials to existing ones
        for(Material new_m : new_materials) {
            materials.push_back(new_m);
        }
        obj->mat = materials;
    } else if (token == "usemtl") {
        //CHANGES: Instead of assigning a material to an object, we need to assign each triangle a material id.
        //For now, each triangle will receive a material name parameter that is then used to determine the materialID
        std::string name = lntokens[1];
        for (int i = 0; i<materials.size(); i++) {
            if(name == materials[i].name) {
                this->temp_materialID = i;
            }
        }
    }
}

ParsedTriangle OBJParser::parseFace(const std::vector<std::string> &tokens) {
    // for now, only triangled faces are supported
    std::shared_ptr<ParsedTriangle> t = std::make_shared<ParsedTriangle>();
    std::vector<std::string> pars;
    std::array<float3, 3> v_array{};
    std::array<float3, 3> vt_array{};
    std::array<float3, 3> vn_array{};

    // assign material ID
    if(this->temp_materialID !=-1)
        t->materialID = this->temp_materialID + this->offset;
    else
        t->materialID = 0;

    // extract the params from the tokens
    pars.insert(pars.begin(), std::next(tokens.begin()), tokens.end());
    size_t numEls = std::min(pars.size(), t->MAX_NUM_VERTICES);
    for (int i = 0; i < numEls; i++) {
        std::vector<std::string> parts = split(pars[i], '/');//lexpp::lex(pars[i], "/");
        // If an index lower than 1 is given, this throws
        unsigned int index = std::stoi(parts[0]) - 1;

        // extract the vertices
        std::copy(vertices[index].begin(),
                  vertices[index].end(),
                  v_array[i].begin());
        if (parts.size() > 1 && !parts[1].empty()) {
            // extract the texture coordinates
            index = std::stoi(parts[1]) - 1;
            std::copy(txt_coords[index].begin(),
                      txt_coords[index].end(),
                      vt_array[i].begin());
        }
        if (parts.size() > 2) {
            // extract the normals
            index = std::stoi(parts[2]) - 1;
            std::copy(normals[index].begin(),
                      normals[index].end(),
                      vn_array[i].begin());
        }
    }
    // copy the extracted data into the triangle
    std::copy(v_array[0].begin(), v_array[0].end(), t->v1.begin());
    std::copy(v_array[1].begin(), v_array[1].end(), t->v2.begin());
    std::copy(v_array[2].begin(), v_array[2].end(), t->v3.begin());

    std::copy(vt_array[0].begin(), vt_array[0].end(), t->vt1.begin());
    std::copy(vt_array[1].begin(), vt_array[1].end(), t->vt2.begin());
    std::copy(vt_array[2].begin(), vt_array[2].end(), t->vt3.begin());

    std::copy(vn_array[0].begin(), vn_array[0].end(), t->vn1.begin());
    std::copy(vn_array[1].begin(), vn_array[1].end(), t->vn2.begin());
    std::copy(vn_array[2].begin(), vn_array[2].end(), t->vn3.begin());


    //checkNormalsOnTriangle(t);
    return *t;
}

// calculates normals for every triangle that has not defined all three triangles
void OBJParser::checkNormalsOnTriangle(std::shared_ptr<ParsedTriangle> t) {
    std::array<float3, 3> t_normals = {t->vn1, t->vn2, t->vn3};
    std::array<float3, 3> t_vertices = {t->v1, t->v2, t->v3};
    for (int i = 0; i < t_normals.size(); i++) {
        if (isEqual(std::accumulate(t_normals[i].begin(), t_normals[i].end(), 0.0f), 0.0f)) {
            // normal not specified
            int idx1 = (i + 1) % t_normals.size();
            int idx2 = (i + 2) % t_normals.size();
            float3 a = t_vertices[i];
            float3 b = t_vertices[idx1];
            float3 c = t_vertices[idx2];
            float3 edge1 = subtractVectors<float, 3>(b, a);
            float3 edge2 = subtractVectors<float, 3>(c, a);
            //printFloat3(edge1);
            //printFloat3(edge2);
            t_normals[i] = normalize(crossProduct(edge1, edge2));
        }
    }
    std::copy(t_normals[0].begin(), t_normals[0].end(), t->vn1.begin());
    std::copy(t_normals[1].begin(), t_normals[1].end(), t->vn2.begin());
    std::copy(t_normals[2].begin(), t_normals[2].end(), t->vn3.begin());
}

// calculates three normals into one
SimpleTriangle OBJParser::simplifyTriangle(const ParsedTriangle &parsed_t) {
    std::shared_ptr<SimpleTriangle> simple_t = std::make_shared<SimpleTriangle>();
    std::array<float3, 3> t_vertices = {parsed_t.v1, parsed_t.v2, parsed_t.v3};
    float3 sum = std::accumulate(t_vertices.begin(), t_vertices.end(), t_vertices[0], addVectors<float, 3>);
    float3 t_normal = normalize(sum);

    // deep copy of parsed triangle
    simple_t->obj_id = parsed_t.obj_id;
    simple_t->materialID = parsed_t.materialID;
    std::copy(parsed_t.v1.begin(),
              parsed_t.v1.end(),
              simple_t->v1.begin());
    std::copy(parsed_t.v2.begin(),
              parsed_t.v2.end(),
              simple_t->v2.begin());
    std::copy(parsed_t.v3.begin(),
              parsed_t.v3.end(),
              simple_t->v3.begin());

    std::copy(parsed_t.vt1.begin(),
              parsed_t.vt1.end(),
              simple_t->vt1.begin());
    std::copy(parsed_t.vt2.begin(),
              parsed_t.vt2.end(),
              simple_t->vt2.begin());
    std::copy(parsed_t.vt3.begin(),
              parsed_t.vt3.end(),
              simple_t->vt3.begin());
    std::copy(t_normal.begin(),
              t_normal.end(),
              simple_t->vn.begin());

    return *simple_t;
}

bool OBJParser::isEqual(float x, float y) {
    const float epsilon = 1e-6;
    return std::abs(x - y) <= epsilon * std::abs(x);
    // see Knuth section 4.2.2 pages 217-218
}

// split string on given delimiter
std::vector<std::string> OBJParser::split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        //elems.push_back(item);
        elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
    }
    return elems;
}

// overwrote this method to make it private
std::shared_ptr<ThreeDObject> OBJParser::parse(const std::string &content) {
    return Parser<ThreeDObject>::parse(content);
}

std::shared_ptr<ThreeDObject> OBJParser::parse(const std::string& content, const int &id, const int &offset) {
    this->id = id;
    this->temp_materialID = -1;
    this->offset = offset;
    std::shared_ptr<ThreeDObject> obj = parse(content);

    obj->id = id;
    /*if(obj->mat.size() == 0) {
        obj->mat.push_back(MTLParser::getDefaultMaterial());
    }*/
    // assign object ID
    for (SimpleTriangle& tri : obj->triangles) {
        tri.obj_id = id;
    }

    return obj;
}
