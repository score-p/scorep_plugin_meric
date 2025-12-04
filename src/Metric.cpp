/*
 * SPDX-FileCopyrightText: (c) 2018 Technische Universität Dresden <tu-dresden.de>
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "Metric.h"

#include <scorep/plugin/plugin.hpp>

#include <chrono>
#include <string>

namespace MericPlugin
{
Metric::Metric( unsigned int domain_idx, unsigned int domain_id, std::string domain_name, unsigned int counter_idx, std::string counter_name ) :
    domain_idx( domain_idx ),
    domain_id( domain_id ),
    domain_name( domain_name ),
    counter_idx( counter_idx ),
    counter_name( counter_name )
{
}


size_t
Metric::id() const
{
    return this->counter_idx * EXTLIB_NUM_DOMAINS + this->domain_idx;
}


std::string
Metric::name() const
{
    return this->domain_name + ":" + this->counter_name;
}


bool
Metric::operator==( const Metric& other ) const
{
    return this->id() == other.id();
}


double
Metric::read( const ExtlibEnergyTimeStamp* ts ) const
{
    return ts->domain_data[ this->domain_idx ].energy_per_counter[ this->counter_idx ];
}
}
