/*
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#pragma once

#include "utils.h"

#include <meric_ext.h>

#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace MericPlugin
{
struct domain_info
{
    unsigned int                                  id;  // The value in the Domains enum
    unsigned int                                  idx; // The index in an ExtlibEnergyTimeStamp.domain_data

    std::unordered_map<std::string, unsigned int> counter_id_by_name;
};


class ExtlibWrapper
{
public:

    static const std::unordered_map<std::string, unsigned int> domain_id_by_name;
    static const std::unordered_map<unsigned int, std::string> domain_name_by_id;

    static std::vector<unsigned int>
    all_domain_ids();

    static std::vector<std::string>
    all_domain_names();

public:

    ExtlibWrapper() = default;
    ExtlibWrapper( const std::vector<unsigned int>& requested_domains );

    std::unordered_map<std::string, domain_info>
    query_available_counters();

    using TimeStamp = std::shared_ptr<ExtlibEnergyTimeStamp>;

    TimeStamp
    read();

    static TimeStamp
    calc_energy_consumption( TimeStamp begin,
                             TimeStamp end );


private:
    struct extlib_deleter
    {
        void
        operator()( ExtlibEnergy* energy_domains ) const
        {
            extlib_close( energy_domains );
        }
    };

    using ExtlibEnergyPtr = std::unique_ptr<ExtlibEnergy, extlib_deleter>;

    ExtlibEnergyPtr energy_domains;
};
}


namespace std
{
inline ostream&
operator<<( ostream& os, const MericPlugin::domain_info& domain )
{
    os << " Counters for domain " << MericPlugin::ExtlibWrapper::domain_name_by_id.at( domain.id ) << ": ";
    for ( auto counter_it : domain.counter_id_by_name )
    {
        os << counter_it.first << ", ";
    }
    return os;
}
}
