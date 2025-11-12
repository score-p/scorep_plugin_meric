/*
 * SPDX-FileCopyrightText: (c) 2018 Technische Universität Dresden <tu-dresden.de>
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "meric_measurement.h"


meric_measurement::meric_measurement( std::chrono::microseconds interval ) : interval( interval )
{
}

void
meric_measurement::start( const std::vector<energy_metric>& handles )
{
    data.clear();
    for ( auto& handle : handles )
    {
        data.insert( std::make_pair( std::ref( const_cast<energy_metric&>( handle ) ),
                                     std::vector<TVPair>() ) );
    }
    active             = true;
    measurement_thread = std::thread([ this ](){
        this->collect_readings();
    } );
}

void
meric_measurement::stop()
{
    active = false;
    if ( measurement_thread.joinable() )
    {
        measurement_thread.join();
    }
}

std::vector<TVPair>&
meric_measurement::readings( energy_metric& handle )
{
    return data[ handle ];
}

void
meric_measurement::collect_readings()
{
    while ( active )
    {
        for ( auto& item : data )
        {
            const auto& metric   = item.first.get();
            auto&       sequence = item.second;
            sequence.emplace_back( metric.read() );
        }
        std::this_thread::sleep_for( interval );
    }
}
