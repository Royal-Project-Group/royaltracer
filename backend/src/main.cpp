#define LEXPP_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "./architecture/RoyalMediator.h"
#include "./api/WebEditorFacade.h"
#include "./raytracer/OpenCLRaytracingFacade.h"


int main(int argc, char *argv[]) {
    RoyalMediator medi = RoyalMediator("/");
    OpenCLRaytracingFacade openClRaytracingFacade(&medi);
    WebEditorFacade server(&medi);
    medi.setEf(&server);
    medi.setRf(&openClRaytracingFacade);

    server.startServer(3000);
}

