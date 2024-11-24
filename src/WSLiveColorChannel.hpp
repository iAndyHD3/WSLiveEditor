#pragma  once
#include <optional>
#include <string>
#include <fmt/format.h>
#include <matjson.hpp>

struct WSLiveColorChannel
{
    std::optional<std::string> str;

    int id = 0;

    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;

    bool p1 = false;
    bool p2 = false;
    bool blending = false;

    std::optional<float> opacity;

    std::string getStringAsGDLevel() const
    {
        auto str = fmt::format("1_{}_2_{}_3_{}", r, g, b);

        if(blending) str += "_5_1";
        if(opacity) str += fmt::format("_7_{}_8_1", *opacity);

        if(p1) str += "_4_1";
        else if(p2) str += "_4_2";


        return str;
    }

    //throws
    static WSLiveColorChannel getFromObject(const matjson::Value& obj)
    {
        auto id = obj.find("id");
        if(id == obj.end()) throw std::exception("id key not found");
        int id_val = id->second.as_int();
        if(id_val < 0 || id_val > 999) throw std::exception("id key must be an int");

        auto colstring = obj.find("string");
        if(colstring != obj.end() && colstring->second.is_string())
        {
            return WSLiveColorChannel{.str = colstring->second.as_string(), .id = id_val};
        }

        auto rgb = obj.find("rgb");
        if(rgb == obj.end()) throw std::exception("rgb nor string key found");
        if(!rgb->second.is_array()) throw std::exception("rgb key must be an array");



        const auto& rgbarr = rgb->second.as_array();
        return WSLiveColorChannel {
            .str = {},
            .id = id_val,
            .r = static_cast<unsigned char>(rgbarr[0].as_int()),
            .g = static_cast<unsigned char>(rgbarr[1].as_int()),
            .b = static_cast<unsigned char>(rgbarr[2].as_int()),
            .p1 = obj.contains("p1") ? obj.find("p1")->second.as_bool() : false,
            .p2 = obj.contains("p2") ? obj.find("p2")->second.as_bool() : false,
            .blending = obj.contains("blending") ? obj.find("blending")->second.as_bool() : false,
            .opacity = obj.contains("opacity") ? static_cast<float>(obj.find("opacity")->second.as_double()) : std::optional<float>{},
        };
    }
    static std::vector<WSLiveColorChannel> getFromArray(const matjson::Array& arr)
    {
        std::vector<WSLiveColorChannel> ret;
        ret.reserve(arr.size());

        for(const auto& v : arr)
        {
            if(!v.is_object()) continue;
            
            ret.push_back(WSLiveColorChannel::getFromObject(v.as_object()));
        }
        return ret;
    }
};
