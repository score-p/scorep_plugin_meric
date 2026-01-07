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
class ExtlibWrapper
{
public:

    static const std::unordered_map<std::string, unsigned int> domain_id_by_name;
    static const std::unordered_map<unsigned int, std::string> domain_name_by_id;

    static std::vector<unsigned int>
    all_domain_ids();

    static std::vector<std::string>
    all_domain_names();

    ExtlibWrapper() = default;
    ExtlibWrapper( const std::vector<unsigned int>& requested_domains );

    struct Domain
    {
        unsigned int                                  id;  // The value in the Domains enum
        unsigned int                                  idx; // The index in an ExtlibEnergyTimeStamp.domain_data

        std::unordered_map<std::string, unsigned int> counter_idx_by_name;

        std::string
        name() const;

        std::string
        counter_names() const;
    };

    std::unordered_map<std::string, Domain>
    query_enabled_domains();

    using TimeStamp = std::shared_ptr<ExtlibEnergyTimeStamp>;

    TimeStamp
    read();

    static TimeStamp
    calc_energy_consumption( TimeStamp begin,
                             TimeStamp end );


private:
    struct ExtlibDeleter
    {
        void
        operator()( ExtlibEnergy* energy_domains ) const
        {
            extlib_close( energy_domains );
        }
    };

    using ExtlibEnergyPtr = std::unique_ptr<ExtlibEnergy, ExtlibDeleter>;

    ExtlibEnergyPtr energy_domains;
};
}
