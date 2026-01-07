/*
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "ExtlibWrapper.h"

#include <scorep/plugin/log.hpp>

#include <iostream>


using namespace MericPlugin;

int
main()
{
    // Silence warnings and info emitted by the plugin
    scorep::plugin::log::set_min_severity_level( nitro::log::severity_level::error );

    const auto   domains     = ExtlibWrapper( ExtlibWrapper::all_domain_ids() ).query_enabled_domains();
    unsigned int num_domains = domains.size();

    if ( num_domains == 0 )
    {
        std::cout << "No domains could be enabled" << std::endl;
        return 1;
    }

    unsigned int num_counters = 0;
    std::cout << "Available domains : ";
    for ( const auto& it : domains )
    {
        std::cout << it.first << ", ";
    }
    std::cout << std::endl;

    for ( const auto& it : domains )
    {
        const size_t n = it.second.counter_idx_by_name.size();
        num_counters += n;
        std::cout << it.first << " counters available: " << it.second.counter_names() << std::endl;
    }

    if ( num_counters == 0 )
    {
        std::cout << "No counters are available" << std::endl;
        return 1;
    }

    return 0;
}
