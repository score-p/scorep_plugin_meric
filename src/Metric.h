/*
 * SPDX-FileCopyrightText: (c) 2018 Technische Universität Dresden <tu-dresden.de>
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#pragma once

#include <meric_ext.h>
#include <scorep/plugin/plugin.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>

namespace MericPlugin
{
struct Metric
{
    using Total       = std::integral_constant<unsigned, 0>;
    using DomainTotal = std::integral_constant<unsigned, 1>;
    using Single      = std::integral_constant<unsigned, 2>;

    Metric( Single,
            unsigned int domain_idx,
            unsigned int domain_id,
            std::string  domain_name,
            unsigned int counter_idx,
            std::string  counter_name );

    Metric ( DomainTotal,
             unsigned int domain_idx,
             unsigned int domain_id,
             std::string  domain_name );

    Metric ( Total );

    Metric( const Metric& ) = delete;

    /* copy-assign */
    Metric&
    operator=( const Metric& ) = delete;

    /* move constructor */
    Metric( Metric&& ) = default;

    /* move assignment */
    Metric&
    operator=( Metric&& ) = default;

    bool
    operator==( const Metric& other ) const;

    size_t
    id() const;

    std::string
    name() const;

    std::string
    description() const;

    double
    read( const ExtlibEnergyTimeStamp* ts ) const;

    bool
    isSingle() const
    {
        return type == Single::value;
    };
    bool
    isDomainTotal() const
    {
        return type == DomainTotal::value;
    };
    bool
    isTotal() const
    {
        return type == Total::value;
    };

    unsigned int type;
    unsigned int domain_idx;  // Index in ExtlibEnergyTimeStamp.domain_data array
    unsigned int domain_id;   // Domain Id, i.e. value in the ExtlibEnergy::Domains enum
    std::string  domain_name;
    unsigned int counter_idx; // Index in ExtlibEnergyTimeStamp.domain_data[domain_idx].energy_per_counter array
    std::string  counter_name;
};
}


namespace std
{
inline ostream&
operator<<( ostream& s, const MericPlugin::Metric& metric )
{
    s << "(" << metric.domain_name << ":" << metric.counter_name << ")";
    return s;
}


template <>
struct hash<MericPlugin::Metric>
{
    size_t inline
    operator()( const MericPlugin::Metric& metric ) const
    {
        return metric.id();
    }
};
}
