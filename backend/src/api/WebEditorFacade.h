//
// Created by nikla on 24.10.2023.
//

#ifndef ROYAL_TRACER_WEBEDITORFACADE_H
#define ROYAL_TRACER_WEBEDITORFACADE_H

#include "../architecture/EditorFacade.h"
#include "../architecture/RaytracingFacade.h"
#include "../architecture/RoyalMediator.h"
#include "../utils/base64pp.h"
#include "crow.h"
#include "crow/middlewares/cors.h"
class WebEditorFacade : public EditorFacade {
private:
    crow::App<crow::CORSHandler> app;

    crow::response exceptionSafeStore(const std::function<bool(std::string, const std::string&)>& callback, std::string id, std::string body);
    crow::response exceptionSafeDelete(const std::function<bool(std::string)>& callback, std::string id);
public:
  explicit WebEditorFacade(RoyalMediator* rm);
  ~WebEditorFacade() override = default;
  bool update3DObject(std::string id, const std::string& content) override;
  bool delete3DObject(std::string id) override;
  bool updateMaterial(std::string id, const std::string& content) override;
  bool deleteMaterial(std::string id) override;
  bool updateTexture(std::string id, const std::string& content) override;
  bool deleteTexture(std::string id) override;
  void startServer(int port);

  Image initiateRaytracing(std::string config) override;
};


#endif //ROYAL_TRACER_WEBEDITORFACADE_H
