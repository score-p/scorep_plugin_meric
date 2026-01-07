/*
 * SPDX-FileCopyrightText: (c) 2018 Technische Universität Dresden <tu-dresden.de>
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "MeasurementThread.h"


namespace MericPlugin
{
MeasurementThread::MeasurementThread( std::chrono::microseconds interval ) : _interval( interval )
{
}


void
MeasurementThread::start( ExtlibWrapper extlib, const std::vector<Metric>& handles )
{
    data.clear();
    for ( auto& handle : handles )
    {
        data.insert( std::make_pair( std::ref( const_cast<Metric&>( handle ) ),
                                     std::vector<TVPair>() ) );
    }
    active             = true;
    measurement_thread = std::thread([ this ](){
            this->collect_readings();
        } );
    this->extlib = std::move( extlib );
}


ExtlibWrapper
MeasurementThread::stop()
{
    active = false;
    if ( measurement_thread.joinable() )
    {
        measurement_thread.join();
    }
    return std::move( this->extlib );
}


std::vector<MeasurementThread::TVPair>&
MeasurementThread::readings( Metric& handle )
{
    return data[ handle ];
}


void
MeasurementThread::collect_readings()
{
    ExtlibWrapper::TimeStamp prev, cur, res;
    prev = this->extlib.read();
    while ( active )
    {
        const auto timestamp = scorep::chrono::measurement_clock::now();
        cur = this->extlib.read();
        res = ExtlibWrapper::calc_energy_consumption( prev, cur );
        for ( auto& item : data )
        {
            const auto& metric   = item.first.get();
            auto&       sequence = item.second;
            sequence.emplace_back( timestamp, metric.read( res.get() ) );
        }
        prev = cur;
        std::this_thread::sleep_for( _interval );
    }
}
}
