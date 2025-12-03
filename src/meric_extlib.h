/*
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#pragma once

#include <meric_ext.h>

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

#include "utils.h"


struct domain_info
{
    unsigned int                                  id;  // The value in the Domains enum
    unsigned int                                  idx; // The index in an ExtlibEnergyTimeStamp.domain_data

    std::unordered_map<std::string, unsigned int> counter_id_by_name;
};


std::ostream&
operator<<( std::ostream&      os,
            const domain_info& domain );




class ExtlibWrapper
{
    struct extlib_deleter
    {
        void
        operator()( ExtlibEnergy* energy_domains ) const
        {
            extlib_close( energy_domains );
        }
    };

    using ExtlibEnergyPtr = std::unique_ptr<ExtlibEnergy, extlib_deleter>;

public:

    static const std::unordered_map<std::string, unsigned int> domain_id_by_name;
    static const std::unordered_map<unsigned int, std::string> domain_name_by_id;

public:

    ExtlibWrapper();
    ExtlibWrapper( const std::vector<unsigned int>& requested_domains );
    std::unordered_map<std::string, domain_info>
    query_available_counters();

    ExtlibEnergyTimeStamp*
    read();

    static ExtlibEnergyTimeStamp*
    calc_energy_consumption( ExtlibEnergyTimeStamp* begin,
                             ExtlibEnergyTimeStamp* end );


private:
    ExtlibEnergyPtr energy_domains;
};
