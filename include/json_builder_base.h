#ifndef JSON_BUILDER_BASE_H
#define JSON_BUILDER_BASE_H

#include "struct.h"

#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"

/******************************************************************************
* Template base for simple classes that build JSON out of structs
******************************************************************************/
template <typename TStruct, int TDocSize>
class JsonBuilderBase
{
public:
    JsonBuilderBase();

    virtual RetResult add(const TStruct *entry) = 0;

    RetResult build(char *buff_out, int buff_size, bool beautify);

    bool is_empty();

    RetResult reset();

    void print();

    StaticJsonDocument<TDocSize>* get_json_doc();
protected:
    StaticJsonDocument<TDocSize> _json_doc;
    JsonArray _root_array;
};

#endif