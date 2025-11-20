/*
 * SPDX-FileCopyrightText: (c) 2018 Technische Universität Dresden <tu-dresden.de>
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "meric_measurement.h"

#include <meric_ext.h>


meric_measurement::meric_measurement( std::chrono::microseconds interval ) : _interval( interval ), energy_domains( nullptr )
{
}

void
meric_measurement::start( ExtlibEnergy* energy_domains, const std::vector<energy_metric>& handles )
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
    this->energy_domains = energy_domains;
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

std::vector<meric_measurement::TVPair>&
meric_measurement::readings( energy_metric& handle )
{
    return data[ handle ];
}

void
meric_measurement::collect_readings()
{
    ExtlibEnergyTimeStamp* prev = nullptr;
    ExtlibEnergyTimeStamp* cur  = nullptr;
    ExtlibEnergyTimeStamp* res  = nullptr;
    prev = extlib_read_energy_measurements( energy_domains );
    while ( active )
    {
        const auto timestamp = scorep::chrono::measurement_clock::now();
        cur = extlib_read_energy_measurements( energy_domains );
        res = extlib_calc_energy_consumption( prev, cur );
        for ( auto& item : data )
        {
            const auto& metric   = item.first.get();
            auto&       sequence = item.second;
            sequence.emplace_back( timestamp, metric.read( res ) );
        }
        extlib_free_energy_timestamp( prev );
        prev = cur;
        cur  = nullptr;
        extlib_free_energy_timestamp( res );
        res = nullptr;
        std::this_thread::sleep_for( _interval );
    }
}
