//
// Created by nikla on 17.11.2023.
//

#ifndef ROYAL_TRACER_PARSER_H
#define ROYAL_TRACER_PARSER_H

#include "../utils/lexpp.h"
#include <string>
#include <memory>

/// This abstract class is a blueprint for deriving parsers.
/// Only parsers of textual content should derive from this class.
template<class Resource>
class Parser {
public:
    /// This method defines the setup for parsers, calling determineElement for every line.
    virtual std::shared_ptr<Resource> parse(const std::string& content) {
        std::shared_ptr<Resource> res = std::make_shared<Resource>();
        std::vector<std::string> lines = lexpp::lex(content, "\n", false);

        for(std::string& line : lines) {
            if(line.ends_with('\r')) {
                line.pop_back();
            }
            determineElement(line, res);
        }

        return res;
    };

    /// This abstract method should be overwritten by deriving parsers to implement their custom parsing functionality.
    virtual void determineElement(std::string line, std::shared_ptr<Resource> res)=0;
};

#endif //ROYAL_TRACER_PARSER_H
