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


struct energy_metric
{
    energy_metric( unsigned int domain_idx,
                   unsigned int domain_id,
                   std::string  domain_name,
                   unsigned int counter_idx,
                   std::string  counter_name );

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


namespace std
{
inline ostream&
operator<<( ostream& s, const energy_metric& metric )
{
    s << "(" << metric.domain_name << ":" << metric.counter_name << ")";
    return s;
}


template <>
struct hash<energy_metric>
{
    size_t inline
    operator()( const energy_metric& metric ) const
    {
        return metric.id();
    }
};
};
