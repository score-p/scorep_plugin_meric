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

namespace MericPlugin
{
struct Metric
{
    Metric( unsigned int domain_idx,
            unsigned int domain_id,
            std::string  domain_name,
            unsigned int counter_idx,
            std::string  counter_name );

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

    double
    read( const ExtlibEnergyTimeStamp* ts ) const;


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
