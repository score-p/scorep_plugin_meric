#pragma once

#include <scorep/plugin/plugin.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

using TVPair = std::pair<scorep::chrono::ticks, double>;

struct energy_metric
{
    energy_metric( const std::string& name );

    energy_metric( const energy_metric& ) = delete;
    /* copy-assign */
    energy_metric&
    operator=( const energy_metric& ) = delete;

    /* move constructor */
    energy_metric( energy_metric&& ) = default;
    /* move assignment */
    energy_metric&
    operator=( energy_metric&& ) = default;

    bool
    operator==( const energy_metric& other ) const;

    TVPair
                read() const;

    std::string name;
    TVPair      ref;
};


namespace std
{
inline ostream&
operator<<( ostream& s, const energy_metric& metric )
{
    s << "(" << metric.name << ")";
    return s;
}

template <>
struct hash<energy_metric>
{
    size_t inline
    operator()( const energy_metric& metric ) const
    {
        return std::hash<std::string>{} ( metric.name );
    }
};
};
