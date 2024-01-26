//
// Created by nikla on 24.10.2023.
//

// # Anmerkung: Mit dieser Request soll bei Laden oder Ändern eines 3D-Objekts diese Änderung außerhalb des Webeditors gespeichert werden.
// PUT [ID der obj.Datei]
// body: [Inhalt der obj.Datei]
//
// # Anmerkung: Mit dieser Request soll bei Löschen eines 3D-Objekts dieses Objekt auch außerhalb des Editors gelöscht
// DELETE [ID der obj.Datei]
//
// # Anmerkung: Starte Raytracing-Vorgang. IN DER RESPONSE MUSS SICH DAS BILD BEFINDEN.
// POST [Config-Datei]

#include "crow.h"
#include "crow/middlewares/cors.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>

#include <filesystem>

#include "WebEditorFacade.h"

bool readConfig(std::string& configFile, std::string& domain) {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open configuration file " << configFile << std::endl;
        return false;
    }

    std::getline(file, domain);

    return true;
}

// Start the server and add the routes.
WebEditorFacade::WebEditorFacade(RoyalMediator* m): EditorFacade(m) {
    std::string configFile = "cors.txt"; // Provide the path to your configuration file
    std::string domain;

    if (!readConfig(configFile, domain)) {
        std::cerr<<"Debug value is used for cors"<<std::endl;
    }

    // Customize CORS headers so all origins will be accepted.
    auto &cors = app.get_middleware<crow::CORSHandler>();

    if(!domain.empty()) {
        cors.global().headers("Access-Control-Allow-Origin", domain).methods("GET"_method,"POST"_method,"PUT"_method,"DELETE"_method).headers("Content-Type");
        cors.global().origin(domain);
    } else {
        cors.global().headers("Access-Control-Allow-Origin", "*").methods("GET"_method,"POST"_method,"PUT"_method,"DELETE"_method).headers("Content-Type");
    }

    //Define routes for the three api endpoints.
    CROW_ROUTE(app, "/api/obj/<string>").methods("PUT"_method)([this](const crow::request &req, std::string id) {
        // weird C++ syntax for passing the function as a callback
        using std::placeholders::_1, std::placeholders::_2;
        return exceptionSafeStore(std::bind(&WebEditorFacade::update3DObject, this, _1, _2), id, req.body);
    });

    CROW_ROUTE(app, "/api/obj/<string>").methods("DELETE"_method)([this](const crow::request &req, std::string id) {
        // weird C++ syntax for passing the function as a callback
        using std::placeholders::_1;
        return exceptionSafeDelete(std::bind(&WebEditorFacade::delete3DObject, this, _1), id);
    });

    CROW_ROUTE(app, "/api/mtl/<string>").methods("PUT"_method)([this](const crow::request &req, std::string id) {
        // weird C++ syntax for passing the function as a callback
        using std::placeholders::_1, std::placeholders::_2;
        return exceptionSafeStore(std::bind(&WebEditorFacade::updateMaterial, this, _1, _2), id, req.body);
    });

    CROW_ROUTE(app, "/api/mtl/<string>").methods("DELETE"_method)([this](const crow::request &req, std::string id) {
        // weird C++ syntax for passing the function as a callback
        using std::placeholders::_1;
        return exceptionSafeDelete(std::bind(&WebEditorFacade::deleteMaterial, this, _1), id);
    });

    CROW_ROUTE(app, "/api/txr/<string>").methods("PUT"_method)([this](const crow::request &req, std::string id) {
        // weird C++ syntax for passing the function as a callback
        using std::placeholders::_1, std::placeholders::_2;
        return exceptionSafeStore(std::bind(&WebEditorFacade::updateTexture, this, _1, _2), id, req.body);
    });

    CROW_ROUTE(app, "/api/txr/<string>").methods("DELETE"_method)([this](const crow::request &req, std::string id) {
        // weird C++ syntax for passing the function as a callback
        using std::placeholders::_1;
        return exceptionSafeDelete(std::bind(&WebEditorFacade::deleteMaterial, this, _1), id);
    });

    CROW_ROUTE(app, "/api/export").methods("POST"_method)([this](const crow::request &req) {
        try {
            Image img = initiateRaytracing(req.body);

            nlohmann::json jsonObject;
            jsonObject["image"] = img.getAsPPM();
            std::string jsonString = jsonObject.dump();


            return crow::response(jsonString);
        } catch (std::exception& e) {
            CROW_LOG_ERROR << "Render Exception: " << e.what();
            std::string usrMsg =    "An error occurred while rendering the image."
                                    "Please check your input or take a look into the web console,"
                                    "where the scene configuration can be found";

            return crow::response(500,usrMsg);
        }
    });
    CROW_ROUTE(app, "/api/health").methods("GET"_method)([this] {
        return crow::response(200);
    });
}

/// Saves the given obj file. Returns true if successful.
bool WebEditorFacade::update3DObject(std::string id, const std::string& content) {
    // OBJ data is sent in plain text, therefore it can be stored directly.
    return mediator->mediateResourceStorage(Obj, id, content);
}

/// Deletes the obj file for the corresponding id. Returns true if successful.
bool WebEditorFacade::delete3DObject(std::string id) {
    return mediator->mediateResourceRemoval(Obj, id);
}

/// Start the rendering process of the given config
Image WebEditorFacade::initiateRaytracing(std::string config) {
    return mediator->mediateImageRendering(config);
}

void WebEditorFacade::startServer(int port) {
    //set the port, set the app to run on multiple threads, and run the app
    app.multithreaded().port(port).run();
}

/// This method invokes the storage of a material in the backend
bool WebEditorFacade::updateMaterial(std::string id, const std::string& content) {
    // MTL data is sent in plain text, therefore it can just be stored.
    return mediator->mediateResourceStorage(Mat, id, content);
}

/// This method invokes the deletion of a material in the backend
bool WebEditorFacade::deleteMaterial(std::string id) {
    return mediator->mediateResourceRemoval(Mat, id);
}

/// This method invokes the storage of a texture in the backend
bool WebEditorFacade::updateTexture(std::string id, const std::string &content) {
    // convert the content into bytes using base64 decode
    std::string base64 = content.substr(content.find_first_of(',')+1);
    std::optional<std::vector<std::uint8_t>> bytes = base64pp::decode(base64);
    if(bytes) {
        return mediator->mediateResourceStorage(Txr, id, bytes.value());
    }
    return false;
}

/// This method invokes the deletion of a texture in the backend.
bool WebEditorFacade::deleteTexture(std::string id) {
    return mediator->mediateResourceRemoval(Txr, id);
}

/// This method ensures that the given callback is handled exception-safely.
/// It is used to respond to a PUT request by the editor.
/// @param callback function to be called by this method
/// @param id ID of the resource to be stored
/// @param body Content of the resource
crow::response WebEditorFacade::exceptionSafeStore(const std::function<bool(std::string, const std::string&)>& callback, std::string id, std::string body) {
    try {
        return callback(id, body) ?
               crow::response(200, "Successfully stored '"+id+"'.") :
               crow::response(500, "An error occurred while storing '"+ id +"', please try again.");
    } catch (std::exception& e) {
        // Log the error
        CROW_LOG_ERROR << "Storage exception: " << e.what();
        // Return an appropriate response
        std::string usrMsg = "Faulty storage request. ("+id+")";
        return {500, usrMsg};
    }
}

/// This method ensures that the given callback is handled exception-safely.
/// It is used to respond to a DELETE request by the editor.
/// @param callback function to be called by this method
/// @param id ID of the resource to be deleted
crow::response
WebEditorFacade::exceptionSafeDelete(const std::function<bool(std::string)> &callback, std::string id) {
    try {
        return callback(id) ?
               crow::response(200, "Successfully deleted '"+id+"'.") :
               crow::response(500, "Deletion of '"+id+"' did not succeed, file probably does not exist.");
    } catch (std::exception& e) {
        // Log the error
        CROW_LOG_ERROR << "Deletion exception: " << e.what();
        // Return an appropriate response
        std::string usrMsg = "Faulty deletion request. ("+id+")";
        return crow::response(500, usrMsg);
    }
}
