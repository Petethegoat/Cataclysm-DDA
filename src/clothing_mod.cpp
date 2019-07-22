#include "clothing_mod.h"

#include <string>
#include <set>

#include "generic_factory.h"
#include "item.h"
#include "debug.h"

namespace
{

generic_factory<clothing_mod> all_clothing_mods( "clothing mods" );

} // namespace

static std::map<clothing_mod_type, std::vector<clothing_mod>> clothing_mods_by_type;

/** @relates string_id */
template<>
bool string_id<clothing_mod>::is_valid() const
{
    return all_clothing_mods.is_valid( *this );
}

/** @relates string_id */
template<>
const clothing_mod &string_id<clothing_mod>::obj() const
{
    return all_clothing_mods.obj( *this );
}

namespace io
{

static const std::map<std::string, clothing_mod_type> clothing_mod_type_map = {{
        { "acid", clothing_mod_type_acid },
        { "fire", clothing_mod_type_fire },
        { "bash", clothing_mod_type_bash },
        { "cut", clothing_mod_type_cut },
        { "encumbrance", clothing_mod_type_encumbrance },
        { "warmth", clothing_mod_type_warmth },
        { "storage", clothing_mod_type_storage },
        { "invalid", clothing_mod_type_invalid }
    }
};

template<>
clothing_mod_type string_to_enum<clothing_mod_type>( const std::string &data )
{
    auto iter = clothing_mod_type_map.find( data );

    if( iter == clothing_mod_type_map.end() ) {
        debugmsg( "Invalid mod type '%s'.", data.c_str() );
        return clothing_mod_type_invalid;
    }

    return string_to_enum_look_up( clothing_mod_type_map, data );
}

template<>
const std::string enum_to_string<clothing_mod_type>( clothing_mod_type data )
{
    const auto iter = std::find_if( clothing_mod_type_map.begin(), clothing_mod_type_map.end(),
    [data]( const std::pair<std::string, clothing_mod_type> &pr ) {
        return pr.second == data;
    } );

    if( iter == clothing_mod_type_map.end() ) {
        if( iter == clothing_mod_type_map.end() ) {
            debugmsg( "Invalid mod type value '%d'.", data );
            return "invalid";
        }
    }

    return iter->first;
}

} // namespace io

void clothing_mod::load( JsonObject &jo, const std::string & )
{
    mandatory( jo, was_loaded, "flag", flag );
    mandatory( jo, was_loaded, "item", item_string );
    mandatory( jo, was_loaded, "implement_prompt", implement_prompt );
    mandatory( jo, was_loaded, "destroy_prompt", destroy_prompt );

    JsonArray jarr = jo.get_array( "mod_value" );
    while( jarr.has_more() ) {
        JsonObject mv_jo = jarr.next_object();
        mod_value mv;
        std::string temp_str;
        mandatory( mv_jo, was_loaded, "type", temp_str );
        mv.type = io::string_to_enum<clothing_mod_type>( temp_str );
        mandatory( mv_jo, was_loaded, "value", mv.value );
        JsonArray jarr_prop = mv_jo.get_array( "proportion" );
        while( jarr_prop.has_more() ) {
            std::string str = jarr_prop.next_string();
            if( str == "thickness" ) {
                mv.thickness_propotion = true;
            } else if( str == "coverage" ) {
                mv.coverage_propotion = true;
            } else {
                jarr_prop.throw_error( "Invalid value, valid are: \"coverage\" and \"thickness\"" );
            }
        }
        mod_values.push_back( mv );
    }
}

float clothing_mod::get_mod_val( const clothing_mod_type &type, const item &it ) const
{
    const int thickness = it.get_thickness();
    const int coverage = it.get_coverage();
    float result = 0.0f;
    for( const mod_value &mv : mod_values ) {
        if( mv.type == type ) {
            float tmp = mv.value;
            if( mv.thickness_propotion ) {
                tmp *= thickness;
            }
            if( mv.coverage_propotion ) {
                tmp *= coverage / 100.0f;
            }
            result += tmp;
        }
    }
    return result;
}

bool clothing_mod::has_mod_type( const clothing_mod_type &type ) const
{
    for( auto mv : mod_values ) {
        if( mv.type == type ) {
            return true;
        }
    }
    return false;
}

size_t clothing_mod::count()
{
    return all_clothing_mods.size();
}

void clothing_mods::load( JsonObject &jo, const std::string &src )
{
    all_clothing_mods.load( jo, src );
}

void clothing_mods::reset()
{
    all_clothing_mods.reset();
    clothing_mods_by_type.clear();
}

const std::vector<clothing_mod> &clothing_mods::get_all()
{
    return all_clothing_mods.get_all();
}

const std::vector<clothing_mod> &clothing_mods::get_all_with( clothing_mod_type type )
{
    const auto iter = clothing_mods_by_type.find( type );
    if( iter != clothing_mods_by_type.end() ) {
        return iter->second;
    } else {
        // Build cache
        std::vector<clothing_mod> list = std::vector<clothing_mod> {};
        for( auto cm : get_all() ) {
            if( cm.has_mod_type( type ) ) {
                list.push_back( cm );
            }
        }
        clothing_mods_by_type.emplace( type, std::move( list ) );
        return clothing_mods_by_type[type];
    }
}

const std::string clothing_mods::string_from_clothing_mod_type( clothing_mod_type type )
{
    return io::enum_to_string<clothing_mod_type>( type );
}
