/*
 * SPDX-FileCopyrightText: (c) 2018 Technische Universität Dresden <tu-dresden.de>
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "Metric.h"

#include <chrono>
#include <string>
#include <sstream>


namespace MericPlugin
{
Metric::Metric( Metric::Single, unsigned int domain_idx, unsigned int domain_id, std::string domain_name, unsigned int counter_idx, std::string counter_name ) :
    type( Metric::Single::value ),
    domain_idx( domain_idx ),
    domain_id( domain_id ),
    domain_name( domain_name ),
    counter_idx( counter_idx ),
    counter_name( counter_name )
{
}


Metric::Metric( Metric::DomainTotal, unsigned int domain_idx, unsigned int domain_id, std::string domain_name ) :
    type( Metric::DomainTotal::value ),
    domain_idx( domain_idx ),
    domain_id( domain_id ),
    domain_name( domain_name ),
    counter_idx( 0 ),
    counter_name( "TOTAL" )
{
}


Metric::Metric( Metric::Total ) :
    type( Metric::Total::value ),
    domain_idx( 0 ),
    domain_id( ExtlibEnergy::Domains::EXTLIB_ENERGY_DOMAIN_END ),
    domain_name( "TOTAL" ),
    counter_idx( 0 ),
    counter_name( "TOTAL" )
{
}


size_t
Metric::id() const
{
    // Multi-index for (counter, domain, type) tuples
    return ( this->counter_idx * EXTLIB_NUM_DOMAINS + this->domain_idx ) * ( 3 ) + this->type;
}


std::string
Metric::name() const
{
    return this->domain_name + ":" + this->counter_name;
}


std::string
Metric::description() const
{
    std::stringstream ss;
    switch ( this->type )
    {
        case Single::value:
            ss << "Counter '" << this->counter_name << "' in Meric energy domain '" << this->domain_name << "'";
            break;
        case DomainTotal::value:
            ss << "Total energy for meric domain '" << this->domain_name << "'";
            break;
        case Total::value:
            ss << "Total energy consumption for all enabled meric domains";
            break;
    }
    return ss.str();
}


bool
Metric::operator==( const Metric& other ) const
{
    return this->id() == other.id();
}


double
Metric::read( const ExtlibEnergyTimeStamp* ts ) const
{
    switch ( this->type )
    {
        case Single::value:
            return ts->domain_data[ this->domain_idx ].energy_per_counter[ this->counter_idx ];
        case DomainTotal::value:
            return ts->domain_data[ this->domain_idx ].energy_total;
        case Total::value:
            return ts->domain_data->energy_total;
        default:
            return 0.;
    }
}
}
